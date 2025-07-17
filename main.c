#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h> 
#include <sys/stat.h> 
#include <sys/types.h>
#include <sys/wait.h> 
#include <ctype.h> 
#include <signal.h>
#include <fcntl.h>
#include <stdbool.h> // for bool
#define MAX_INPUT_LEN 4096
#define MAX_PATH_LEN 4096
#define MAX_MSG_LEN 1024 // USED FOR ERROR message AND DEBUG message
#define MAX_FILENAME_LEN 526
#define MAX_FUNCNAME_LEN 256
#define MAX_JOBS 50
#define MAX_STATE_LEN 11
#define DEBUG 1

volatile sig_atomic_t foreground_pid = -1;

void throw_error(const char func_name[MAX_FUNCNAME_LEN], const char msg[MAX_MSG_LEN]){
    printf("Error in: %s:\n\t\t%s", func_name, msg);
}

void debug_msg(const char msg[MAX_MSG_LEN]){
    #ifdef DEBUG
        printf("Debug: %s\n", msg);
    #endif
}

struct Job {
    int id;
    int pid;
    int gpid;
    char state[MAX_STATE_LEN];
};

struct Shell {
    struct Job* jobs[MAX_JOBS];
    int job_count;
};

struct Job new_job(int id, int pid, int gpid, const char* state) {
    struct Job job;
    job.id = id;
    job.pid = pid;
    job.gpid = gpid;
    strncpy(job.state, state, MAX_STATE_LEN);
    return job;
}

void add_job(struct Shell* shell, struct Job job) {
    if (shell->job_count >= MAX_JOBS) {
        throw_error("add_job", "Too many jobs\n");
        return;
    }
    shell->jobs[shell->job_count] = malloc(sizeof(struct Job)); // reserve and adds
    *(shell->jobs[shell->job_count]) = job;
    shell->job_count++;
}

void sigint_handler(int signo) {
    if (foreground_pid > 0) {
        kill(foreground_pid, SIGINT); // send SIGINT to the foreground process
    }
}

void get_input(char* input, size_t size){
    printf("prompt > ");
    if (fgets(input, size, stdin)) {
        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') {
            input[len - 1] = '\0';  // strip newline
        }
    }
    else{
        throw_error("get_input", "fgets returned false!!!");
    }
}

bool is_same(char* input, const char* other){
    return strcmp(input, other) == 0;
}

void pwd(){
    char cwd[MAX_PATH_LEN];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        throw_error("pwd", "getcwd() error");
    }
}

void cd(char* path){
    if (path == NULL) {
        throw_error("cd", "No path Provided");
        return;
    }

    if (chdir(path) != 0) {
        throw_error("cd", "error in chdir");
    }
}

bool check_builtin(char* input, struct Shell* shell){
    if (is_same(input, "quit")){ 
        /* No need to do anything main loop will handle 
        but it is a builtin so return true */
        for (int i = 0; i < shell->job_count; ++i) {
            free(shell->jobs[i]);
        }
        return true;
    }else if (is_same(input, "pwd")){
        pwd();
        return true;
    }

    char* command = strtok(input, " \t");
    if (is_same(command, "cd")){
        char* path = strtok(NULL, " \t");
        cd(path);
        return true;
    }
    return false; // Not QUIT, PWD or CD
}

bool check_exe(char* input, struct Shell* shell){
    char exe_path[MAX_FILENAME_LEN];
    snprintf(exe_path, sizeof(exe_path), "./%s", input); // adding ./ to path

    if (access(exe_path, X_OK) != 0){
        debug_msg("Not executable!!!"); // Not an error can be another command
        return false;
    }
    
    pid_t pid = fork();
    
    if (pid < 0) {
        throw_error("check_exe", "Fork didn't work");
    }
    else if (pid == 0) { // child
        // Child process: run executable
        pid_t my_pid = getpid();
        setpgid(0, 0);  // make new group
        execl(exe_path, input, (char*)NULL);
        throw_error("check_exe", "Error in execl. It returned");
        exit(127);
    }else { // parent
        setpgid(pid, pid);
        struct Job job = new_job(shell->job_count + 1, pid, pid, "Foreground");
        add_job(shell, job);

        foreground_pid = pid;
        int status;
        waitpid(pid, &status, 0);
        foreground_pid = -1;
    }
    return true;
}

bool check_bg(char* input, struct Shell* shell){
    // Check if the last char is % if so then make a bg job
    // Fork and not wait
    // have the status as background
    // create handler for SIGCHILD which then calls wait() or waitpid(). 
    return true;
}

void take_action(char* input, struct Shell* shell){
    if (check_builtin(input, shell)){
        debug_msg("Is a builtin func");
        return;
    }else {
        if (check_exe(input, shell)) {
            debug_msg("Is a executable file");
            return;
        }else if (check_bg(input, shell)){
            debug("Is a bg file");
            return;
        }
    }
}

int main(){
    signal(SIGINT, sigint_handler);

    char user_input[MAX_INPUT_LEN];
    struct Shell shell;
    shell.job_count = 0;
    do{
        get_input(user_input, MAX_INPUT_LEN);
        take_action(user_input, &shell);
    }while (!is_same(user_input, "quit"));
    return 0;
}
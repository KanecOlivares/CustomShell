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
#define MAX_FUNCNAME_LEN 256
#define MAX_STATE_LEN 11
#define DEBUG 1

struct Job {
    int id;
    int pid;
    char* state[MAX_STATE_LEN];
};

void throw_error(const char func_name[MAX_FUNCNAME_LEN], const char msg[MAX_MSG_LEN]){
    printf("Error in: %s:\n\t\t%s", func_name, msg);
}

void debug_msg(const char msg[MAX_MSG_LEN]){
    #ifdef DEBUG
        printf("Debug: %s\n", msg);
    #endif
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

bool check_builtin(char* input){
    if (is_same(input, "quit")){ 
        /* No need to do anything main loop will handle 
        but it is a builtin so return true */
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
    return false;
}

void take_action(char* input){
    if (check_builtin(input)){
        // debug_msg("Is a builtin func");
        return;
    }else {
        debug_msg("Command not recognized.");
    }
}

int main(){
    char user_input[MAX_INPUT_LEN];
    do{
        get_input(user_input, MAX_INPUT_LEN);
        take_action(user_input);
    }while (!is_same(user_input, "quit"));
    return 0;
}
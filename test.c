#include <stdio.h>
#include <unistd.h>
#define MAX_ITER 10
int main () {
    for (int i = 1; i <= MAX_ITER; ++i){
        printf("Testing %d\n", i);
        sleep(1);
    }
    
}
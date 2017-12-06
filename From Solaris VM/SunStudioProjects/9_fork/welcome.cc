#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <errno.h>
#define CHILD_ID 0
#define FORK_ERROR -1

int main (){
    pid_t process_id = fork();

    if (process_id == FORK_ERROR){
        perror("Error fork");
        return EXIT_FAILURE;
    }

    if (process_id == CHILD_ID){
        execl("/bin/cat", "cat", "long_text", NULL);
        perror("Error exec");
        return EXIT_FAILURE;
    }

    process_id = wait(NULL);
    while(process_id == FORK_ERROR && errno != ECHILD){
        perror("Error wait. Wait again");
        process_id = wait(NULL);
    }

    printf ("I am parent\n");

    return EXIT_SUCCESS;
}

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#define CHILD_ID 0
#define FORK_ERROR -1


int main (int argc, char **argv){
    if(argc < 2) {
        perror("Enter programm's name, which you want to execute\n");
        return EXIT_FAILURE;
    }

    pid_t process_id = fork();

    if (process_id == FORK_ERROR){
        perror("Error fork");
        return EXIT_FAILURE;
    }

    if (process_id == CHILD_ID){
        execvp(argv[1], &argv[1]);
        perror("Error exec");
        return EXIT_FAILURE;
    }

    printf("Wait child: %d\n", process_id);

    int status = 0;
    process_id = wait(&status);
    if (process_id == FORK_ERROR){
        perror("Error wait");
        return EXIT_FAILURE;
    }

    if(WIFSIGNALED(status)) {
        printf("Child process exit by signal, signal: %d\n", WTERMSIG(status));
        return EXIT_SUCCESS;
    }

    //Need to add check like in signals. There is macro for it. Maybe WIFEXITED?
    printf("Child process stop by exit, returns code: %d\n", WEXITSTATUS(status));
    return EXIT_SUCCESS;
}

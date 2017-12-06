#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <wait.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "header.h"

int writeFork(FILE *handler) {
    pid_t pid = fork();
    if(pid == ERROR) {
        perror("Fork error");
        return EXIT_FAILURE;
    }

    if(pid != CHILD) {
        return EXIT_SUCCESS;
    }

    printf("Enter string: \n");

    ssize_t code;
    char buffer;

    while((code = read(STDIN_FILENO, &buffer, sizeof(char))) > 0 && buffer != '\n') {
        if(fputc(buffer, handler) == EOF) {
            perror("Write str in pipe error");
            pclose(handler);
            return EXIT_FAILURE;
        }
    }

    if(code == ERROR) {
        perror("Read error");
        exit(EXIT_FAILURE);
    }

    pclose(handler);
    exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
    FILE *handler;
    if((handler = popen("tr [a-z] [A-Z]", "w")) == NULL) {
        perror("Error making pipe");
        return EXIT_FAILURE;
    }

    if(writeFork(handler) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    pclose(handler);

    int status;
    while(wait(&status) != ERROR) {
        if(WIFEXITED(status) && WEXITSTATUS(status) != EXIT_SUCCESS || WIFSIGNALED(status)) {
            return EXIT_FAILURE;
        }
    }
	
	printf("\n");	

    return EXIT_SUCCESS;
}


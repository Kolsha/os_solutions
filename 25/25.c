#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <wait.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "header.h"

int writeFork(int pipefd[]) {
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
    close(pipefd[READ]);

    while((code = read(STDIN_FILENO, &buffer, sizeof(char))) > 0 && buffer != '\n') {
        if(write(pipefd[WRITE], &buffer, sizeof(char)) == ERROR) {
            perror("Write error");
            exit(EXIT_FAILURE);
        }
    }

    if(code == ERROR) {
        perror("Read error");
        exit(EXIT_FAILURE);
    }

    close(pipefd[WRITE]);
    exit(EXIT_SUCCESS);
}

int readFork(int pipefd[]) {
    pid_t pid = fork();
    if(pid == ERROR) {
        perror("Fork error");
        return EXIT_FAILURE;
    }

    if(pid != CHILD) {
        return EXIT_SUCCESS;
    }

    ssize_t code;
    char buffer;
    close(pipefd[WRITE]);

    while((code = read(pipefd[READ], &buffer, sizeof(char))) > 0) {
        buffer = (char)toupper(buffer);
        if(write(STDOUT_FILENO, &buffer, sizeof(char)) == ERROR) {
            perror("Write error");
            exit(EXIT_FAILURE);
        }
    }

    if(code == ERROR) {
        perror("Read error");
        exit(EXIT_FAILURE);
    }

    if(write(STDOUT_FILENO, "\n", sizeof(char)) == ERROR) {
        perror("Write error");
        exit(EXIT_FAILURE);
    }

    close(pipefd[WRITE]);
    exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
    int pipefd[2];

    if (pipe(pipefd) != EXIT_SUCCESS) {
        perror("Pipe error");
        return EXIT_FAILURE;
    }

    if(writeFork(pipefd) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    if(readFork(pipefd) != EXIT_SUCCESS) {
        return  EXIT_FAILURE;
    }

    close(pipefd[READ]);
    close(pipefd[WRITE]);

    int status;
    while(wait(&status) != ERROR) {
        if(WIFEXITED(status) && WEXITSTATUS(status) != EXIT_SUCCESS || WIFSIGNALED(status)) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}


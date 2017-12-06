#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <wait.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#define READ 0
#define WRITE 1
#define ERROR -1
#define CHILD 0

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

    ssize_t len = 0;
    char    buf[BUFSIZ] = {'\0'};
    close(pipefd[READ]);
    
    /*while(fgets(&buf[len], BUFSIZ, stdin) != NULL) {
       len += strlen(&buf[len]);
       buf[len] = '\n';
    }
     **/

    int n;
    while((n = read(0, buf, BUFSIZ)) > 0){
        write(pipefd[WRITE], buf, n);
        
    }
    //len += strlen(&buf[len]) + 1;
    /*buf[len] = 0;
     if(write(pipefd[WRITE], buf, len) == ERROR) {
            perror("Write error");
            exit(EXIT_FAILURE);
     }
  */

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
    char    buf[BUFSIZ] = {'\0'};
    close(pipefd[WRITE]);

    while((code = read(pipefd[READ], buf, BUFSIZ)) > 0) {

	for(size_t i = 0; i < code; i++){
		buf[i] = toupper(buf[i]);
	}
        if(write(1, buf, code) == ERROR) {
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


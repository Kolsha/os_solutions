#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define ERROR -1
#define RESULT_STRING "Count empty lines: "

int main(int argc, char *argv[]) {
    if(argc < 2) {
        fprintf(stderr, "Need filename for analise\n");
        return EXIT_FAILURE;
    }

    int file = open(argv[1], O_RDONLY);
    if(file == ERROR) {
        perror("Can't open file");
        return EXIT_FAILURE;
    }

    FILE *handler;
    if((handler = popen("wc -l", "w")) == NULL) {
        perror("Error making pipe");
        close(file);
        return EXIT_FAILURE;
    }

    if(write(STDOUT_FILENO, RESULT_STRING, strlen(RESULT_STRING)) == ERROR) {
        perror("Can't write to stdout");
        close(file);
        pclose(handler);
        return EXIT_FAILURE;
    }

    ssize_t code;
    char buffer;
    char prevChar = '\n';
    while((code = read(file, &buffer, sizeof(char))) > 0) {
        if(buffer == '\n' && prevChar == '\n') {
            fprintf(handler, "%c", buffer);
        }

        prevChar = buffer;
    }

    if(code == ERROR) {
        perror("Error read file");
        close(file);
        pclose(handler);
        return EXIT_FAILURE;
    }

    pclose(handler);
    close(file);

    return EXIT_SUCCESS;
}

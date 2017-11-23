#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>

#define ERROR -1

int main(int argc, char **argv) {
    ssize_t code;
    char buffer;
    while((code = read(STDIN_FILENO, &buffer, sizeof(char))) > 0) {
        buffer = (char)toupper(buffer);

        if(write(STDOUT_FILENO, &buffer, sizeof(char)) == ERROR) {
            perror("Can't write to stdout");
            return EXIT_FAILURE;
        }
    }

    if(code == ERROR) {
        perror("Can't read from stdin");
        return EXIT_FAILURE;
    }

    return EXIT_FAILURE;
}

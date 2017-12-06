#include <stdio.h>
#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>

#define COUNT_SYMBS 1
#define ERROR -1

int setNewAttr(int terminal, struct termios *old_option) {
    struct termios new_option;

    if (tcgetattr(terminal, old_option) == ERROR) {
        perror("Error tcgetattr");
        return EXIT_FAILURE;
    }

    new_option = *old_option;
    new_option.c_lflag &= ~(ICANON);
    new_option.c_cc[VMIN] = COUNT_SYMBS;
    new_option.c_cc[VTIME] = 0;

    if (tcsetattr(terminal, TCSANOW, &new_option) == ERROR) {
        perror("Error tcsetattr");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int setOldAttr(int terminal, struct termios *old_option) {
    if (tcsetattr(terminal, TCSANOW, old_option) == ERROR) {
        perror("Error tcsetattr");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int getInput(int terminal) {
    char buffer[COUNT_SYMBS] = {};
    char question[] = "Enter something: ";
    char answer[] = "\nYour input: ";
    char newLine[] = "\n";

    if (write(terminal, question, strlen(question)) == ERROR) {
        perror("Error write");
        return EXIT_FAILURE;
    }

    if (read(terminal, buffer, COUNT_SYMBS) == ERROR) {
        perror("Error read");
        return EXIT_FAILURE;
    }

    if (write(terminal, answer, strlen(answer)) == ERROR) {
        perror("Error write");
        return EXIT_FAILURE;
    }

    if (write(terminal, buffer, COUNT_SYMBS) == ERROR) {
        perror("Error write");
        return EXIT_FAILURE;
    }

    if (write(terminal, newLine, strlen(newLine)) == ERROR) {
        perror("Error write");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int main() {
    int terminal = open("/dev/tty", O_RDWR);
    if (terminal == ERROR) {
        perror("Error open terminal");
        return EXIT_FAILURE;
    }

    if(!isatty(terminal)) {
        perror("Open not terminal");
        close(terminal);
        return EXIT_FAILURE;
    }

    struct termios old_option;
    if(setNewAttr(terminal, &old_option) != EXIT_SUCCESS) {
        close(terminal);
        return EXIT_FAILURE;
    }

    if(getInput(terminal) != EXIT_SUCCESS) {
        setOldAttr(terminal, &old_option);
        close(terminal);
        return EXIT_FAILURE;
    }

    if(setOldAttr(terminal, &old_option) != EXIT_SUCCESS) {
        close(terminal);
        return EXIT_FAILURE;
    }

    close(terminal);
    return EXIT_SUCCESS;
}


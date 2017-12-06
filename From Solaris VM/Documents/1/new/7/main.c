#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <termios.h>
#include <sys/mman.h>

#include "header.h"

struct FileLine {
    off_t offset;
    int length;
};



struct termios tty, savtty;
int fd;

int signaled = 0;

int read_line_num(){
    char inp_buff[BUFSIZ] = {0};
    char tmp = '\0';
    size_t pos = 0;
    read(fd, &tmp, 1);
    while(tmp != '\n'){
        putchar(tmp);
        if(signaled){
            return 0;
        }
        inp_buff[pos++] = tmp;
        read(fd, &tmp, 1);
        alarm(TIME_WAIT);
    }
    putchar('\n');
    int res = atoi(inp_buff);;
    return res;
}


char *mappingFile =  NULL;
size_t fileLength = 0;

int parseFile(struct FileLine *lines, int *countLine) {
    *countLine = 0;

    for(size_t i = 0; i < fileLength; ++i) {
        if(mappingFile[i] != '\n') {
            ++lines[*countLine].length;
            continue;
        }

        ++*countLine;
        if(*countLine >= COUNT_LINES) {
            fprintf(stderr, "File too big\n");
            return ERROR_FILE_SIZE;
        }

        lines[*countLine].offset = lines[*countLine - 1].offset + lines[*countLine - 1].length + 1;
    }

    //++*countLine;
    return EXIT_SUCCESS;
}

void printAllString(int sig) {
    printf("%s", mappingFile);
    tcsetattr(fd, TCSAFLUSH, &savtty);
    sig = 0;
    signaled = 1;
}

int printLines(int countLines, struct FileLine *lines) {
    printf("Enter number of string: (0 for exit)\n");

    int needLine = 0;

    if(signal(SIGALRM, printAllString) == SIG_ERR) {
        perror("Signal set error");
        return ERROR_SIGNAL;
    }
    alarm(TIME_WAIT);

    for(;;) {
        needLine = read_line_num();
        alarm(0);

        if(needLine == 0) {
            return EXIT_SUCCESS;
        }

        if(needLine < 1 || needLine > countLines) {
            printf("Bad number!\n");
            alarm(TIME_WAIT);
            continue;
        }

        for(off_t i = lines[needLine - 1].offset, count = i + lines[needLine - 1].length; i < count; ++i) {
            printf("%c", mappingFile[i]);
        }
        printf("\n");

        alarm(TIME_WAIT);
    }

    if(errno == EINTR) { //если выход из цикла произошел из-за сигнала SIGALRM
        return EXIT_SUCCESS;
    }

    perror("Scanf error");
    return EXIT_FAILURE;
}

void * mapping(char *file) {
    int input = open(file, O_RDONLY);
    if(input == ERROR_OPEN_FILE) {
        perror("Open file error");
        return NULL;
    }

    off_t offset = lseek(input, 0, SEEK_END);
    if(offset == ERROR_READ_FILE) {
        perror("Can't read file");
        return NULL;
    }

    fileLength = (size_t)offset;
    char *ptr = mmap(NULL, fileLength, PROT_READ, MAP_PRIVATE, input, 0);
    if(ptr == MAP_FAILED) {
        perror("Error mapping");
        return NULL;
    }

    close(input);

    return ptr;
}

int main(int argc, char **argv) {
    if(argc < 2) {
        fprintf(stderr, "Need filename\n");
        return BAD_ARGS;
    }

    fd = open("/dev/tty", O_RDONLY);
    tcgetattr(fd, &tty);
    if (isatty(fileno(stdout)) == 0) {
        fprintf(stderr,"stdout not terminal\n");
        exit(1);
    }
    savtty = tty;
    tty.c_lflag &= ~(ISIG | ICANON | ECHO);
    tty.c_cc[VMIN] = 1; /* MIN */
    tcsetattr(fd, TCSAFLUSH, &tty);
    setbuf(stdout, (char *) NULL);


    mappingFile = mapping(argv[1]);
    if(mappingFile == NULL) {
        tcsetattr(fd, TCSAFLUSH, &savtty);
        return ERROR_OPEN_FILE;
    }

    struct FileLine lines[COUNT_LINES] = {};
    int countLines = 0;
    if(parseFile(lines, &countLines) != EXIT_SUCCESS) {
        tcsetattr(fd, TCSAFLUSH, &savtty);
        return ERROR_READ_FILE;
    }

    if(countLines == 0) {
        fprintf(stderr, "Empty file\n");
        tcsetattr(fd, TCSAFLUSH, &savtty);
        return FILE_IS_EMPTY;
    }

    if(printLines(countLines, lines) != EXIT_SUCCESS) {
        tcsetattr(fd, TCSAFLUSH, &savtty);
        return ERROR_READ_FILE;
    }
    tcsetattr(fd, TCSAFLUSH, &savtty);

    return EXIT_SUCCESS;
}

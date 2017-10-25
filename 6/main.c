#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <termios.h>

#define COUNT_LINES 160
#define ERROR_READ_FILE -1
#define BAD_ARGS -2
#define ERROR_OPEN_FILE -1
#define ERROR_SIGNAL -1
#define FILE_IS_EMPTY -4
#define ERROR_FILE_SIZE -5
#define BAD_INPUT -6
#define TIME_WAIT 5


struct termios tty, savtty;
int fd;

int signaled = 0;


struct FileLine {
    off_t offset;
    int length;
};

int input = 0;

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

int parseFile(struct FileLine *lines, int *countLine) {
    *countLine = 0;
    char c = 0;
    ssize_t code = 0;

    while((code = read(input, &c, sizeof(char))) > 0) {
        if(c != '\n') {
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

    if(code == ERROR_READ_FILE) {
        perror("Can't read file");
        return ERROR_READ_FILE;
    }

    //++*countLine;
    return EXIT_SUCCESS;
}

void printAllString(int sig) {
    tcsetattr(fd, TCSAFLUSH, &savtty);
    if(lseek(input, 0, SEEK_SET) == ERROR_READ_FILE) {
        perror("Can't read file");
    }

    char c = 0;
    ssize_t code = 0;
    while((code = read(input, &c, sizeof(char))) > 0) {
        printf("%c", c);
    }

    if(code == ERROR_READ_FILE) {
        perror("Can't read file");
    }
    sig = 0;
    signaled = 1;

}

int printLines(int countLines, struct FileLine *lines) {
    printf("Enter number of string: (0 for exit)\n");

    int needLine = 0;
    char c = 0;
    int code = 0;

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

        if(lseek(input, lines[needLine - 1].offset, SEEK_SET) == ERROR_READ_FILE) {
            perror("Can't read file");
            return ERROR_READ_FILE;
        }

        for(int i = 0; i < lines[needLine - 1].length; ++i) {
            if(read(input, &c, sizeof(char)) <= 0) {
                perror("Can't read file");
                return ERROR_READ_FILE;
            }

            printf("%c", c);
        }
        printf("\n");

        alarm(TIME_WAIT);
    }

    if(errno == EINTR) { //если выход из цикла произошел из-за сигнала SIGALRM
        return EXIT_SUCCESS;
    }

    if(code == 0) {
        fprintf(stderr, "Bad input\n");
        return BAD_INPUT;
    }

    perror("Scanf error");
    return EXIT_FAILURE;
}

int main(int argc, char **argv) {

    if(argc < 2) {
        fprintf(stderr, "Need filename\n");
        return BAD_ARGS;
    }


    struct termios tty, savtty;

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


    input = open(argv[1], O_RDONLY);
    if(input == ERROR_OPEN_FILE) {
        perror("Open file error");
        tcsetattr(fd, TCSAFLUSH, &savtty);
        return ERROR_OPEN_FILE;
    }

    struct FileLine lines[COUNT_LINES] = {};
    int countLines = 0;
    if(parseFile(lines, &countLines) != EXIT_SUCCESS) {
        close(input);
        tcsetattr(fd, TCSAFLUSH, &savtty);
        return ERROR_READ_FILE;
    }

    if(countLines == 0) {
        fprintf(stderr, "Empty file\n");
        close(input);
        tcsetattr(fd, TCSAFLUSH, &savtty);
        return FILE_IS_EMPTY;
    }

    if(printLines(countLines, lines) != EXIT_SUCCESS) {
        close(input);
        tcsetattr(fd, TCSAFLUSH, &savtty);
        return ERROR_READ_FILE;
    }

    tcsetattr(fd, TCSAFLUSH, &savtty);
    close(input);
    return EXIT_SUCCESS;
}

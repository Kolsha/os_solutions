#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <stdlib.h>

int main()
{
    char ch, *text =
            "The quick brown fox jumped over the lazy dog\'s back";
    int fd, i, errors = 0, len;
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
    printf("Type beneath the following line\n\n%s\n", text);
    len = strlen(text);
    for (i = 0; i < len; i++) {
        read(fd, &ch, 1);
        if (ch == text[i])
            putchar(ch);
        else {
            putchar('\07');
            putchar('*');
            errors++;
        }
    }
    tcsetattr(fd, TCSAFLUSH, &savtty);
    printf("\n\nnumber of errors: %d\n", errors);
}

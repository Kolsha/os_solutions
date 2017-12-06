#include <stdio.h>
#include <memory.h>
#include <termios.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <poll.h>
#include <errno.h>
#include <unistd.h>
#define TIME_OUT 1000

int main(int argc, char *argv[])
{

    if(argc <= 1) {
        //return printInfo(".");
        printf("Input's pls\n");
        return EXIT_FAILURE;
    }

    char    buf[BUFSIZ] = {'\0'};
    int *is_alive = (int*)malloc(sizeof(int) * argc);
    if(is_alive == NULL){
        perror("Not enough memory\n");
        return EXIT_FAILURE;
    }
    is_alive[0] = 1;
    for (int i = 1; i < argc; i++) {
        is_alive[i] = open(argv[i], O_RDONLY);
        if(is_alive[i] < 0){
            fprintf(stderr, "Cannot open %s\n", argv[i]);
        }
    }


    while(is_alive[0]){
        int count_not_alive = 0;
        for (int i = 1; i < argc; i++) {
            if(is_alive[i] < 0){
                count_not_alive++;
                continue;
            }
            struct pollfd fds;
            fds.fd = is_alive[i];
            fds.events = POLLIN;
            fds.revents = 0;
            int ret = poll(&fds, 1, TIME_OUT);
            if(ret < 0){
                perror("Poll error");
            }

            if(ret > 0 && (fds.revents & POLLIN)){
                if(read(fds.fd, buf, BUFSIZ) == 0){
                    close(is_alive[i]);
                    is_alive[i] = -1;
                    fprintf(stderr, "Close %d:", fds.fd);
                }
                else{
                    fprintf(stderr, "Readed from %d: %s", fds.fd, buf);
                }
            }
            if((ret < 0 && errno == EINVAL)  ||
                    (fds.revents & POLLNVAL) ||
                    (fds.revents & POLLHUP)){
                close(is_alive[i]);
                is_alive[i] = -1;
            }
        }

        if(count_not_alive >= (argc - 1)){
            break;
        }
    }

    for (int i = 1; i < (argc - 1); i++) {
        if(is_alive[i] >= 0){
            close(is_alive[i]);
        }
    }

    free(is_alive);

    return 0;
}

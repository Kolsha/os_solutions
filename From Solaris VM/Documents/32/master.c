#include <sys/types.h>
#include <signal.h>
#include <termios.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "header.h"

int closeQueue(int queue) {
    if(msgctl(queue,IPC_RMID,NULL) == ERROR) {
        perror("Can't close queue");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int recMessages(int queue) {
    struct message buf;
    int count = 0;
    do {
        ssize_t length;
        if((length = msgrcv(queue, &buf, SIZE_STRUCT_MESSAGE, MSG_ALL_TYPES, NO_FLAGS)) == ERROR) {
            perror("Can't get message");
            return EXIT_FAILURE;
        }

        switch(buf.mtype) {
            case MSG_TYPE_REG:
                ++count;
                printf("New sender %d\n", buf.pid);
                break;

            case MSG_TYPE_DEL:
                --count;
                printf("Del sender %d\n", buf.pid);
                break;

            default:
                printf("Process %d: %s\n", buf.pid, buf.mtext);
        }
    } while (count > 0);

    return EXIT_SUCCESS;
}

int main() {
    key_t queueKey = ftok("master.c", PROJECT_PREFIX);
    if(queueKey == ERROR) {
        perror("Can't generate key for queue");
        return EXIT_FAILURE;
    }

    int queue;
    queue = msgget(queueKey, IPC_CREAT | QUEUE_RIGHTS);
    if(queue == ERROR) {
        perror("Can't make queue");
        return EXIT_FAILURE;
    }

    printf("Queue was created\n");

    if(recMessages(queue) != EXIT_SUCCESS) {
        closeQueue(queue);
        return EXIT_FAILURE;
    }

    if(closeQueue(queue) != EXIT_SUCCESS) {
        perror("Can't close queue");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


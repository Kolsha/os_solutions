#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "header.h"

int closeQueue(int queue) {
    if(msgctl(queue,IPC_RMID,NULL) == ERROR) {
        perror("Can't close queue");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

size_t msgLength(char *msg, ssize_t code) {
    if(code <= 0) {
        return 0;
    }

    if(msg[code - 1] == '\n') {
        msg[code - 1] = '\0';
    } else {
        msg[code] = '\0';
        ++code;
    }

    return code;
}

int sendMessages(int queue) {
    struct message buf;
    buf.mtype = MSG_TYPE_DATA;
    int isWork = TRUE;
    do {
        ssize_t code;
        if((code = read(STDIN_FILENO, buf.mtext, SIZE_MSG - 1)) == ERROR) {
            perror("Can't read message");
            return EXIT_FAILURE;
        }

        size_t length = msgLength(buf.mtext, code);

        if(code == 0) {
            buf.mtype = MSG_TYPE_END;
            isWork = FALSE;

        }

        if(msgsnd(queue, &buf, length, NO_FLAGS) == ERROR) {
            perror("Can't send message");
            return EXIT_FAILURE;
        }

    } while(isWork);
    printf("Send ended\n");

    return EXIT_SUCCESS;
}

int waitAnswer(int queue) {
    struct message buf;
    if(msgrcv(queue, &buf, SIZE_MSG, MSG_TYPE_ANSWER, NO_FLAGS) == ERROR) {
        perror("Can't get answer");
        return EXIT_FAILURE;
    }

    printf("Get answer from recv\n");

    return EXIT_SUCCESS;
}

int main() {
    int queue;

    queue = msgget(getuid(), IPC_CREAT | QUEUE_RIGHTS);
    if(queue == ERROR)     {
        perror("Can't make queue");
        return EXIT_FAILURE;
    }
    printf("Queue was created\nEnter messages (max length - %d symbols, Ctrl+D - for exit):\n", SIZE_MSG - 1);
    //because we need to have '\0' in the end of message

    if(sendMessages(queue) != EXIT_SUCCESS) {
        closeQueue(queue);
        return EXIT_FAILURE;
    }

    if(waitAnswer(queue) != EXIT_SUCCESS) {
        closeQueue(queue);
        return EXIT_FAILURE;
    }

    if(closeQueue(queue) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    printf("Queue was closed\n");
    return EXIT_SUCCESS;
}

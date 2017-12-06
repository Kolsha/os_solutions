#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>

#include "header.h"

int regInQueue(int queue) {
    pid_t pid = getpid();
    struct message buf = {MSG_TYPE_REG, pid, ""};

    if(msgsnd(queue, &buf, sizeof(pid_t), NO_FLAGS) == ERROR) {
        perror("Can't register in queue");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int delFromQueue(int queue) {
    pid_t pid = getpid();
    struct message buf = {MSG_TYPE_DEL, pid, ""};

    if(msgsnd(queue, &buf, sizeof(pid_t), NO_FLAGS) == ERROR) {
        perror("Can't leave queue");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int generateMessage(char *buf) {
    int length = rand() % (SIZE_MSG - 2) + 1; //length from 1 to SIZE_MSG-1
    int i;
    for(i = 0; i < length; ++i) {
        buf[i] = (char)(rand() % (END_CHAR - START_CHAR) + START_CHAR);
    }
    buf[length] = '\0';
    ++length;

    return length;
}

int sendMessages(int queue) {
    pid_t pid = getpid();
    struct message buf = {MSG_TYPE_DATA, pid, ""};
    int i;

    printf("I'm %d\n", pid);
    
    for(i = 0; i < COUNT_MESSAGES; ++i) {
        int length = generateMessage(buf.mtext);

        printf("Send message: %s\n", buf.mtext);

        if(msgsnd(queue, &buf, length + sizeof(pid_t), NO_FLAGS) == ERROR) {
            perror("Can't send message");
            return EXIT_FAILURE;
        }

        sleep(SLEEP_TIME);
    }

    return EXIT_SUCCESS;
}

int main() {
    srand(time(NULL));

    key_t queueKey = ftok("master.c", PROJECT_PREFIX);
    if(queueKey == ERROR) {
        perror("Can't generate key for queue");
        return EXIT_FAILURE;
    }

    int queue;
    queue = msgget(queueKey, NO_FLAGS);
    if(queue == ERROR)     {
        perror("Can't make queue");
        return EXIT_FAILURE;
    }

    printf("Connected to queue\n");

    if(regInQueue(queue) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    if(sendMessages(queue) != EXIT_SUCCESS) {
        delFromQueue(queue);
        return EXIT_FAILURE;
    }

    if(delFromQueue(queue) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


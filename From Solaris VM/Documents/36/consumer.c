#include <stdio.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/msg.h>

#include "header.h"

int detachSharedMem(void *attachedMem) {
    if(shmdt(attachedMem) == ERROR) {
        perror("Can't detach memory");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int regInProducer(int queue) {
    struct message buf = {MSG_REG, ""};
    if(msgsnd(queue, &buf, EMPTY_MSG, NO_FLAGS) == ERROR) {
        perror("Cant't register in producer");
        return EXIT_FAILURE;
    }

    if(msgrcv(queue, &buf, MSG_SIZE, MSG_CONFIRM_REG, NO_FLAGS) == ERROR) {
        perror("Can't get confirm reg");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int consume(int sem, int queue, char *attachedMem) {
    struct sembuf waitWriting = {SEM_CONSUMER, LOCK, SEM_UNDO};
    struct sembuf consumed = {SEM_PRODUCER, UNLOCK, SEM_UNDO};
    struct sembuf consumerReset = {SEM_CONSUMER, 0, SEM_UNDO};

    if(regInProducer(queue) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    while(TRUE) {
        if(semop(sem, &waitWriting, 1) == ERROR) {
            perror("Can't lock consumer");
            return EXIT_FAILURE;
        }

        if(*attachedMem == END_PRODUCE) {
            printf("End work\n");
            break;
        }
        printf("Producer sent: %s\n", attachedMem);

        if(semop(sem, &consumerReset, 1) == ERROR) {
            perror("Can't reset consumer");
            return EXIT_FAILURE;
        }

        if(semop(sem, &consumed, 1) == ERROR) {
            perror("Can't unlock producer");
            return EXIT_FAILURE;
        }
    }

    if(semop(sem, &consumed, 1) == ERROR) {
        perror("Can't unlock producer");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int main() {
    key_t idKey = ftok("producer.c", PROJECT_PREFIX);
    if(idKey == ERROR) {
        perror("Can't generate key for queue");
        return EXIT_FAILURE;
    }

    int sem = semget(idKey, COUNT_SEMAPHORE, IPC_CREAT | ACCESS_RIGHTS);
    if(sem == ERROR) {
        perror("Can't get semaphore");
        return EXIT_FAILURE;
    }

    int queue;
    queue = msgget(idKey, NO_FLAGS);
    if(queue == ERROR)     {
        perror("Can't take queue");
        return EXIT_FAILURE;
    }

    int sharedMem = shmget(idKey, MSG_SIZE, IPC_CREAT | ACCESS_RIGHTS);
    if(sharedMem == ERROR) {
        perror("Can't get shared memory");
        return EXIT_FAILURE;
    }

    char *attachedMem = shmat(sharedMem, ANY_PLACE, NO_FLAGS);
    if(attachedMem == NULL) {
        perror("Can't attach memory");
        return EXIT_FAILURE;
    }

    if(consume(sem, queue, attachedMem) != EXIT_SUCCESS) {
        detachSharedMem(attachedMem);
        return EXIT_FAILURE;
    }

    if(detachSharedMem(attachedMem) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


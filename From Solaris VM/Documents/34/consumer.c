#include <stdio.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <stdlib.h>

#include "header.h"

int detachSharedMem(void *attachedMem) {
    if(shmdt(attachedMem) == ERROR) {
        perror("Can't detach memory");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int consume(int sem, char *attachedMem) {
    struct sembuf waitWriting = {SEM_CONSUMER, LOCK, SEM_UNDO};
    struct sembuf consumed = {SEM_PRODUCER, UNLOCK, SEM_UNDO};

    int i;
    for(i = 0; i < COUNT_MESSAGES; ++i) {
        if(semop(sem, &waitWriting, 1) == ERROR) {
            perror("Can't unlock producer");
            return EXIT_FAILURE;
        }

	sleep(1);

        printf("Producer sent: %s\n", attachedMem);

        if(semop(sem, &consumed, 1) == ERROR) {
            perror("Can't lock consumer");
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

int main() {
    key_t idKey = ftok("producer.c", PROJECT_PREFIX);
    if(idKey == ERROR) {
        perror("Can't generate key for queue");
        return EXIT_FAILURE;
    }

    int sem = semget(idKey, COUNT_SEMAPHORE, NO_FLAGS);
    if(sem == ERROR) {
        perror("Can't get semaphore");
        return EXIT_FAILURE;
    }

    int sharedMem = shmget(idKey, MSG_SIZE, NO_FLAGS);
    if(sharedMem == ERROR) {
        perror("Can't get shared memory");
        return EXIT_FAILURE;
    }

    char *attachedMem = shmat(sharedMem, ANY_PLACE, NO_FLAGS);
    if(attachedMem == NULL) {
        perror("Can't attach memory");
        return EXIT_FAILURE;
    }

    if(consume(sem, attachedMem) != EXIT_SUCCESS) {
        detachSharedMem(attachedMem);
        return EXIT_FAILURE;
    }

    if(detachSharedMem(attachedMem) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


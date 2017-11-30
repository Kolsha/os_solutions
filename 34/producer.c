#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "header.h"

int generateMessage(char *buf) {
    int length = rand() % (MSG_SIZE - 2) + 1; //length from 1 to SIZE_MSG-1
    int i;
    for(i = 0; i < length; ++i) {
        buf[i] = (char)(rand() % (END_CHAR - START_CHAR) + START_CHAR);
    }
    buf[length] = '\0';
    ++length;

    return length;
}

int detachSharedMem(void *attachedMem) {
    if(shmdt(attachedMem) == ERROR) {
        perror("Can't detach memory");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int delSharedMem(int sharedMem) {
    if(shmctl(sharedMem, IPC_RMID, NULL) == ERROR) {
        perror("Can't del shared memory");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int delSem(int sem) {
    if(semctl(sem, DUMMY, IPC_RMID, DUMMY) == ERROR) {
        perror("Can't del semaphore");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int produce(int sem, char *attachedMem) {
    struct sembuf waitReading = {SEM_PRODUCER, LOCK, SEM_UNDO};
    struct sembuf produced = {SEM_CONSUMER, UNLOCK, SEM_UNDO};

    int i;
    for(i = 0; i < COUNT_MESSAGES; ++i) {
        generateMessage(attachedMem);

        printf("Send: %s\n", attachedMem);

        if(semop(sem, &produced, 1) == ERROR) {
            perror("Can't unlock consumer");
            return EXIT_FAILURE;
        }

        if(semop(sem, &waitReading, 1) == ERROR) {
            perror("Can't lock producer");
            return EXIT_FAILURE;
        }

//	sleep(1);
    }

    return EXIT_SUCCESS;
}

int main() {
    srand(time(NULL));

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

    int sharedMem = shmget(idKey, MSG_SIZE, IPC_CREAT | ACCESS_RIGHTS);
    if(sharedMem == ERROR) {
        perror("Can't get shared memory");
        delSem(sem);
        return EXIT_FAILURE;
    }

    char *attachedMem = shmat(sharedMem, ANY_PLACE, NO_FLAGS);
    if(attachedMem == NULL) {
        delSem(sem);
        delSharedMem(sharedMem);
        perror("Can't attach memory");
        return EXIT_FAILURE;
    }

    if(produce(sem, attachedMem) != EXIT_SUCCESS) {
        delSem(sem);
        detachSharedMem(attachedMem);
        delSharedMem(sharedMem);
        return EXIT_FAILURE;
    }

    if(detachSharedMem(attachedMem) != EXIT_SUCCESS) {
        delSharedMem(sharedMem);
        delSem(sem);
        return EXIT_FAILURE;
    }

    if(delSharedMem(sharedMem) != EXIT_SUCCESS) {
        delSem(sem);
        return EXIT_FAILURE;
    }

    if(delSem(sem) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


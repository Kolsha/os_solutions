#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/msg.h>
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

int closeQueue(int queue) {
    if(msgctl(queue,IPC_RMID,NULL) == ERROR) {
        perror("Can't close queue");
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

int updConsumers(int queue, short *count) {
    struct message buf;
    while(msgrcv(queue, &buf, MSG_SIZE, MSG_REG, IPC_NOWAIT) >= 0) {
        buf.mtype = MSG_CONFIRM_REG;
        if(msgsnd(queue, &buf, EMPTY_MSG, NO_FLAGS) == ERROR) {
            perror("Can't send confirm message");
            return EXIT_FAILURE;
        }

        ++*count;
    }

    return EXIT_SUCCESS;
}

int getMessageCount() {
    printf("Enter message count: ");
    int res;

    while((scanf("%d", &res) != 1 && !scanf("%*s")) || res < 1) {
        printf("Enter correct value: ");
    }

    return res;
}

int produce(int sem, int queue, char *attachedMem) {
    struct sembuf waitReading = {SEM_PRODUCER, 0, SEM_UNDO};
    struct sembuf produced = {SEM_CONSUMER, 0, SEM_UNDO};
    struct sembuf consumerReset = {SEM_CONSUMER, 0, SEM_UNDO};
    int countMessages = getMessageCount();
    short countConsumers = 0;
    int i;
    for(i = 0; i < countMessages;) {
        if(updConsumers(queue, &countConsumers) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }

        if(countConsumers <= 0) {
            continue;
        }

        if(semop(sem, &consumerReset, 1) == ERROR) {
            perror("Can't reset consumer");
            return EXIT_FAILURE;
        }

        generateMessage(attachedMem);
        printf("Send to %hd consumers: %s\n", countConsumers, attachedMem);

        produced.sem_op = countConsumers;
        if(semop(sem, &produced, 1) == ERROR) {
            perror("Can't unlock consumer");
            return EXIT_FAILURE;
        }

        waitReading.sem_op = -countConsumers;
        if(semop(sem, &waitReading, 1) == ERROR) {
            perror("Can't lock producer");
            return EXIT_FAILURE;
        }

        sleep(1);
        ++i;
    }

    *attachedMem = END_PRODUCE;
    produced.sem_op = countConsumers;
    if(semop(sem, &produced, 1) == ERROR) {
        perror("Can't unlock consumer");
        return EXIT_FAILURE;
    }
    waitReading.sem_op = -countConsumers;
    if(semop(sem, &waitReading, 1) == ERROR) {
        perror("Can't lock producer");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int initIpc(int *sem, int *queue, int *sharedMem, char **attachedMem) {
    key_t idKey = ftok("producer.c", PROJECT_PREFIX);
    if(idKey == ERROR) {
        perror("Can't generate key for queue");
        return EXIT_FAILURE;
    }

    *sem = semget(idKey, COUNT_SEMAPHORE, IPC_CREAT | ACCESS_RIGHTS);
    if(*sem == ERROR) {
        perror("Can't get semaphore");
        return EXIT_FAILURE;
    }

    *queue = msgget(idKey, IPC_CREAT | ACCESS_RIGHTS);
    if(*queue == ERROR) {
        perror("Can't make queue");
        delSem(*sem);
        return EXIT_FAILURE;
    }

    *sharedMem = shmget(idKey, MSG_SIZE, IPC_CREAT | ACCESS_RIGHTS);
    if(*sharedMem == ERROR) {
        perror("Can't get shared memory");
        delSem(*sem);
        closeQueue(*queue);
        return EXIT_FAILURE;
    }

    *attachedMem = shmat(*sharedMem, ANY_PLACE, NO_FLAGS);
    if(*attachedMem == NULL) {
        perror("Can't attach memory");
        delSem(*sem);
        delSharedMem(*sharedMem);
        closeQueue(*queue);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int destroyIpc(int sem, int queue, int sharedMem, char *attachedMem) {
    if(detachSharedMem(attachedMem) != EXIT_SUCCESS) {
        delSharedMem(sharedMem);
        delSem(sem);
        closeQueue(queue);
        return EXIT_FAILURE;
    }

    if(delSharedMem(sharedMem) != EXIT_SUCCESS) {
        delSem(sem);
        closeQueue(queue);
        return EXIT_FAILURE;
    }

    if(closeQueue(queue) != EXIT_SUCCESS) {
        delSem(sem);
        return EXIT_FAILURE;
    }

    if(delSem(sem) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int main() {
    srand(time(NULL));

    int sem;
    int queue;
    int sharedMem;
    char *attachedMem;
    if(initIpc(&sem, &queue, &sharedMem, &attachedMem) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    if(produce(sem, queue, attachedMem) != EXIT_SUCCESS) {
        delSem(sem);
        closeQueue(queue);
        detachSharedMem(attachedMem);
        delSharedMem(sharedMem);
        return EXIT_FAILURE;
    }

   if(destroyIpc(sem, queue, sharedMem, attachedMem) != EXIT_SUCCESS) {
       return EXIT_FAILURE;
   }

    return EXIT_SUCCESS;
}


#ifndef _INC_34_HEADER_H_
#define _INC_34_HEADER_H_

#define ERROR -1
#define PROJECT_PREFIX 45
#define ACCESS_RIGHTS 0600
#define COUNT_SEMAPHORE 2
#define NO_FLAGS 0
#define ANY_PLACE 0
#define DUMMY 0
#define SEM_PRODUCER 0
#define SEM_CONSUMER 1
#define MSG_SIZE 50
#define LOCK -1
#define UNLOCK 1
#define COUNT_MESSAGES 15
#define START_CHAR 32
#define END_CHAR 127
#define MSG_REG 1
#define MSG_CONFIRM_REG 2
#define MSG_DEL 3
#define EMPTY_MSG 0
#define END_PRODUCE '\0'
#define TRUE 1

struct message {
    long mtype;
    char mtext[MSG_SIZE];
};

#endif //_INC_34_HEADER_H_


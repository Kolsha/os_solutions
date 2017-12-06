#ifndef _INC_32_HEADER_H_
#define _INC_32_HEADER_H_

#define SIZE_MSG 100
#define SIZE_STRUCT_MESSAGE (SIZE_MSG + sizeof(pid_t))
#define ERROR -1
#define NO_FLAGS 0
#define MSG_TYPE_REG 1
#define MSG_TYPE_DEL 2
#define MSG_TYPE_DATA 3
#define MSG_ALL_TYPES 0
#define QUEUE_RIGHTS 0600
#define PROJECT_PREFIX 97
#define COUNT_MESSAGES 10
#define START_CHAR 32
#define END_CHAR 127
#define SLEEP_TIME 1

struct message {
    long mtype;
    pid_t pid;
    char mtext[SIZE_MSG];
};

#endif //_INC_32_HEADER_H_


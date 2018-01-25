#pragma once

#include <pthread.h>

#define SOCKET int

#define BUFFER_SIZE 8192

#define WRITE_BY 64

#define CACHE_SIZE 1024


pthread_mutex_t cacheLock;



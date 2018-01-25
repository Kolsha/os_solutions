#pragma once

#include <stdlib.h>
#include <pthread.h>
#include "common.h"

#include "hash_table.h"

typedef enum entryStatus
{
    ES_DOWNLOADING,
    ES_VALID,
    ES_INVALID
} entryStatus_t;

typedef struct cacheEntry
{
    char   *url;
    char   *data;
    size_t dataCount;
    size_t waitersCount;
    entryStatus_t entryStatus;
} cacheEntry_t;

HashTable cache;


//struct cacheEntry cache[CACHE_SIZE];


int getFreeCacheIndex();


int findInCache(char *url);

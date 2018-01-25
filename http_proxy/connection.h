#pragma once

#include <stdlib.h>
#include "cache.h"
#include "common.h"

typedef enum connectionStatus
{
    CS_GETTING_REQUEST = 0,

    CS_CONNECTING_TO_SERVER = 10,
    CS_WRITING_REQUEST = 11,
    CS_FORWARDING_REQUEST = 12,
    CS_FORWARDING_RESPONSE = 13,
    CS_RESPONDING_FROM_PROGESS_CACHE = 19,
    CS_RESPONDING_FROM_CACHE = 20
} connectionStatus_t;

typedef struct connection
{
    SOCKET clientSocket;
    SOCKET serverSocket;
    connectionStatus_t connectionStatus;

    char   *buffer;
    size_t buffer_size;

    long left_to_download;

    //int cacheEntryIndex;
    cacheEntry_t *cacheEntry;
    size_t cacheBytesWritten;

    int trackingId;
} connection_t;

void *handleConnection(connection_t *connection);



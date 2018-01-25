#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "logger.h"
#include "address_utils.h"
#include "common.h"
#include "connection.h"
#include "http_utils.h"
#include "cache.h"
#include "connection.h"
#include "hash_table.h"




static void *thread_exit_handler(connection_t *connection);

static void updatePollFDByConn(connection_t *conn, struct pollfd fds[2]);

static void writingRequest(connection_t *conn);

static int forwardingResponse(connection_t *connection, char *buff);

static int forwardingRequest(connection_t *connection, char *buff);

static int gettingRequest(connection_t *connection, char *buff);

static int respondingFromCache(connection_t *conn);

static int respondingFromProgessCache(connection_t *conn);



void *handleConnection(connection_t *connection)
{
    logg_track(LL_INFO, connection->trackingId, "Thread started");

    struct pollfd fds[2];
    char buff[BUFFER_SIZE];

    for(;;)
    {
        updatePollFDByConn(connection, fds);
        int polled = poll(fds, 2, 1000 * 2);

        if (polled < 0){
            logg(LL_WARNING, "eventLoop", "polling error");
            return thread_exit_handler(connection);
        }

        if (polled == 0)
            continue;


        switch (connection->connectionStatus)
        {
        case CS_GETTING_REQUEST:
            if (fds[0].revents & POLLHUP)
            {
                logg_track(LL_VERBOSE, connection->trackingId, "Client-side socket closed");

                return thread_exit_handler(connection);
            }
            else if (fds[0].revents & POLLIN)
            {
                if(gettingRequest(connection, buff) < 0){
                    return thread_exit_handler(connection);
                }
            }
            break;

        case CS_CONNECTING_TO_SERVER:
            if (fds[1].revents & POLLOUT)
            {
                connection->connectionStatus = CS_WRITING_REQUEST;
            }
            break;
        case CS_WRITING_REQUEST:
            if (fds[1].revents & POLLOUT)
            {
                writingRequest(connection);
            }
            break;
        case CS_FORWARDING_REQUEST:
            if (fds[0].revents & POLLIN && fds[1].revents & POLLOUT)
            {
                if(forwardingRequest(connection, buff) < 0){
                    return thread_exit_handler(connection);
                }
            }
            break;
        case CS_FORWARDING_RESPONSE:
            if (fds[0].revents & POLLOUT || connection->clientSocket < 0)
            {
                if(forwardingResponse(connection, buff) < 0){
                    return thread_exit_handler(connection);
                }
            }
            break;

        case CS_RESPONDING_FROM_CACHE:
            if (fds[0].revents & POLLOUT)
            {
                if(respondingFromCache(connection) < 0){
                    return thread_exit_handler(connection);
                }
            }
            break;
        case CS_RESPONDING_FROM_PROGESS_CACHE:
            if (fds[0].revents & POLLOUT)
            {
                if(respondingFromProgessCache(connection) < 0){
                    pthread_mutex_lock(&cacheLock);
                    connection->cacheEntry->waitersCount--;
                    pthread_mutex_unlock(&cacheLock);
                    return thread_exit_handler(connection);
                }
            }
            break;
        }



    }
}






static void *thread_exit_handler(connection_t *connection)
{
    if(connection != NULL){
        logg_track(LL_INFO, connection->trackingId, "Thread stopped");

        close(connection->clientSocket);

        if (connection->buffer_size > 0 && connection->buffer != NULL)
            free(connection->buffer);

        if(connection->cacheEntry != NULL && connection->cacheEntry->entryStatus == ES_DOWNLOADING
                && connection->cacheEntry->waitersCount < 1)
            connection->cacheEntry->entryStatus = ES_INVALID;

        free(connection);
    }else{
        logg_track(LL_INFO, -1, "Thread stopped");
    }

    pthread_exit(0);
    return NULL;
}


static int respondingFromProgessCache(connection_t *conn){
    size_t last_size = 0;
    //char *last_ptr = ;
    cacheEntry_t *entry = conn->cacheEntry;
    if(entry == NULL)
        return -1;
    while(entry->entryStatus == ES_DOWNLOADING){
        //usleep(10);
        if(last_size < entry->dataCount)
            if(write(conn->clientSocket, entry->data + last_size, entry->dataCount - last_size) < 0 && errno == EPIPE)
                return -1;

        last_size = entry->dataCount;
        //last_ptr = &last_ptr[last_size];
    }
    if(last_size < entry->dataCount)
        if(write(conn->clientSocket, entry->data + last_size, entry->dataCount - last_size) < 0)
            return -1;

    return -1;
}


static int gettingRequest(connection_t *connection, char *buff){

    logg_track(LL_VERBOSE, connection->trackingId, "Getting request");
    ssize_t readCount = read(connection->clientSocket, buff, BUFFER_SIZE);
    logg_track(LL_VERBOSE, connection->trackingId, "Got request");

    if (readCount == 0)
    {
        logg_track(LL_VERBOSE, connection->trackingId, "Client-side socket closed");
        return -1;
    }

    logg_track(LL_VERBOSE, connection->trackingId, "Got %zi bytes of request", readCount);

    //write(1, buff, readCount);

    connection->buffer_size += (size_t) readCount;
    char *tmp_buff = realloc(connection->buffer,
                             connection->buffer_size);
    if(tmp_buff == NULL)
        return -1;

    connection->buffer = tmp_buff;

    char *dest = connection->buffer + connection->buffer_size - readCount;
    memcpy(dest, buff, (size_t) readCount);

    if (connection->buffer_size > 3)
    {
        char *url = getUrlFromData(connection->buffer,
                                   connection->buffer_size);
        if (url == NULL)
            return 0;

        if (!isCacheMethod(connection->buffer))
        {
            logg_track(LL_WARNING, connection->trackingId,
                       "Only forward request");
            connection->cacheEntry = NULL; // Do not save to cache, as other client already working on it
            connection->connectionStatus = CS_CONNECTING_TO_SERVER;
            connection->serverSocket = initServerSocketToUrl(url);
            editRequestToSendToServer(connection);
            free(url);
            return 0;
        }
        else
        {

            //int j = findInCache(url);
            cacheEntry_t *entry = (cacheEntry_t *) ht_get(&cache, url);
            if(entry != NULL && entry->entryStatus == ES_INVALID){
                pthread_mutex_lock(&cacheLock);
                ht_delete(&cache, url);
                entry = NULL;
                pthread_mutex_unlock(&cacheLock);
            }
            if(entry != NULL){
                if (entry->entryStatus == ES_VALID)
                {
                    logg_track(LL_VERBOSE, connection->trackingId,
                               "Found cache entry for %s, responding from cache", url);

                    free(connection->buffer);
                    connection->buffer = NULL;

                    connection->cacheEntry = entry;
                    connection->cacheBytesWritten = 0;
                    connection->connectionStatus = CS_RESPONDING_FROM_CACHE;
                }
                else if (entry->entryStatus == ES_DOWNLOADING)
                {
                    logg_track(LL_VERBOSE, connection->trackingId,
                               "Found not finished cache entry for %s",
                               url);

                    free(connection->buffer);
                    connection->buffer = NULL;
                    connection->cacheEntry = entry;
                    connection->connectionStatus = CS_RESPONDING_FROM_PROGESS_CACHE;
                    pthread_mutex_lock(&cacheLock);
                    entry->waitersCount++;
                    pthread_mutex_unlock(&cacheLock);

                }

                free(url);

            }
            else
            {

                logg_track(LL_VERBOSE, connection->trackingId,
                           "Cache entry for %s not found, responding from server", url);
                //int newCache = getFreeCacheIndex();
                cacheEntry_t *newCache = malloc(sizeof(cacheEntry_t));
                if(newCache != NULL){

                    newCache->url = url;
                    newCache->data = NULL;
                    newCache->entryStatus = ES_DOWNLOADING;
                    newCache->dataCount = 0;
                    newCache->waitersCount = 0;
                    pthread_mutex_lock(&cacheLock);
                    ht_set(&cache, url, (Pointer) newCache);
                    pthread_mutex_unlock(&cacheLock);
                }



                connection->cacheEntry = newCache;
                connection->connectionStatus = CS_CONNECTING_TO_SERVER;
                connection->serverSocket = initServerSocketToUrl(url);
                editRequestToSendToServer(connection);

                if(newCache == NULL){
                    logg_track(LL_VERBOSE, connection->trackingId,
                               "Cache full", url);
                    free(url);
                }
            }

        }

    }
    return 0;
}


static void updatePollFDByConn(connection_t *conn, struct pollfd fds[2]){
    fds[0].fd = conn->clientSocket;
    fds[1].fd = conn->serverSocket;
    switch (conn->connectionStatus)
    {
    case CS_GETTING_REQUEST:
        fds[0].events = POLLIN;
        fds[1].events = 0;
        break;

    case CS_CONNECTING_TO_SERVER:
    case CS_WRITING_REQUEST:
        fds[0].events = 0;
        fds[1].events = POLLOUT;
        break;
    case CS_FORWARDING_REQUEST:
        fds[0].events = POLLIN;
        fds[1].events = POLLOUT;
        break;
    case CS_FORWARDING_RESPONSE:
        fds[0].events = POLLOUT;
        fds[1].events = POLLIN;
        break;

    case CS_RESPONDING_FROM_PROGESS_CACHE:
    case CS_RESPONDING_FROM_CACHE:
        fds[0].events = POLLOUT;
        fds[1].events = 0;
        break;
    }
}


static void writingRequest(connection_t *conn){
    logg_track(LL_VERBOSE, conn->trackingId,
               "Writing %zu bytes", conn->buffer_size);

    write(conn->serverSocket, conn->buffer,
          conn->buffer_size);

    logg_track(LL_VERBOSE, conn->trackingId,
               "Wrote %zu bytes", conn->buffer_size);

    //write(1, conn->buffer, conn->buffer_size);


    if ((conn->buffer[conn->buffer_size - 3] == '\n' &&
         conn->buffer[conn->buffer_size - 1] == '\n') ||
            (conn->buffer[conn->buffer_size - 1] != '\n' &&
             !isCacheMethod(conn->buffer)
             ))
    {
        conn->buffer_size = 0;
        conn->left_to_download = -1;
        conn->connectionStatus = CS_FORWARDING_RESPONSE;
    }
    else
    {
        conn->connectionStatus = CS_FORWARDING_REQUEST;
    }
}


static int forwardingResponse(connection_t *connection, char *buff){
    logg_track(LL_VERBOSE, connection->trackingId, "Reading response");
    ssize_t readCount = read(connection->serverSocket, buff, BUFFER_SIZE);
    if (readCount <= 0)
    {
        logg_track(LL_INFO, connection->trackingId, "Transmission over, socket closed");
        if (connection->cacheEntry != NULL)
            connection->cacheEntry->entryStatus = ES_VALID;

        close(connection->serverSocket);

        return -1;
    }

    logg_track(LL_VERBOSE, connection->trackingId,
               "Forwarding %zi  bytes of response", readCount);
    if(connection->clientSocket > 0 &&
            write(connection->clientSocket, buff, (size_t) readCount) < 0 && errno == EPIPE){
        connection->clientSocket = -1;
    }


    //write(1, buff, (size_t) readCount);
    logg_track(LL_VERBOSE, connection->trackingId,
               "Forwarded %zi bytes of response", readCount);

    if (connection->cacheEntry != NULL)
    {
        if(connection->cacheEntry->waitersCount < 1 && connection->clientSocket < 0){
            //may be free
            connection->cacheEntry->entryStatus = ES_INVALID;
            close(connection->serverSocket);
            return -1;
        }

        cacheEntry_t *entry = connection->cacheEntry;

        entry->dataCount += (size_t) readCount;
        entry->data = realloc(entry->data,
                              entry->dataCount);
        char *dest = entry->data + entry->dataCount - readCount;
        memcpy(dest, buff, (size_t) readCount);
    }

    if (connection->left_to_download == -1)
    {
        connection->buffer_size += (size_t) readCount;
        connection->buffer = realloc(connection->buffer,
                                     connection->buffer_size);
        char *dest = connection->buffer + connection->buffer_size - readCount;
        memcpy(dest, buff, (size_t) readCount);

        for (size_t j = 0; j < connection->buffer_size - 2; ++j)
        {
            if (connection->buffer[j] == '\n' &&
                    connection->buffer[j + 2] == '\n')
            {
                int statusCode = getResponseCodeFromData(connection->buffer);
                long contentLength = getContentLengthFromData(connection->buffer,
                                                              connection->buffer_size);

                logg_track(LL_INFO, connection->trackingId,
                           "Response headers acquired, response code: %d, Content-Length: %li",
                           statusCode, contentLength);

                if (contentLength != -1)
                {
                    if (statusCode != 200)
                    {
                        //free(connection->cacheEntry->data);
                        //connection->cacheEntry->data = NULL;
                        connection->cacheEntry->entryStatus = ES_INVALID;
                        connection->cacheEntry = NULL;
                    }

                    connection->left_to_download =
                            contentLength - connection->buffer_size + (j + 3);

                    if (connection->left_to_download <= 0)
                    {
                        logg_track(LL_INFO, connection->trackingId,
                                   "Transmission over, Content-Length reached");
                        if (connection->cacheEntry != NULL)
                            connection->cacheEntry->entryStatus = ES_VALID;

                        close(connection->serverSocket);
                        return -1;
                    }

                    free(connection->buffer);
                    connection->buffer = NULL;
                    connection->buffer_size = 0;
                }
                else
                {
                    //free(connection->cacheEntry->data);
                    //connection->cacheEntry->data = NULL;
                    connection->cacheEntry->entryStatus = ES_INVALID;


                    //connection->cacheEntry = NULL;

                    if (!isResponseHasPayload(statusCode))
                    {
                        logg_track(LL_INFO, connection->trackingId,
                                   "Transmission over, no payload for %d response", statusCode);

                        close(connection->serverSocket);

                        return -1;
                    }
                }

                break;
            }
        }
    }
    else
    {
        connection->left_to_download -= readCount;
        if (connection->left_to_download <= 0)
        {
            logg_track(LL_INFO, connection->trackingId,
                       "Transmission over, Content-Length reached");
            if (connection->cacheEntry != NULL)
                connection->cacheEntry->entryStatus = ES_VALID;

            close(connection->serverSocket);
            return -1;
        }
    }
    return 0;
}


static int forwardingRequest(connection_t *connection, char *buff){
    //logg_track(LL_VERBOSE, connection->trackingId, "Reading request");
    ssize_t readCount = read(connection->clientSocket, buff, BUFFER_SIZE);
    if (readCount > 0)
    {
        logg_track(LL_VERBOSE, connection->trackingId,
                   "Forwarding %zi  bytes of request", readCount);
        write(connection->serverSocket, buff, (size_t) readCount);
        logg_track(LL_VERBOSE, connection->trackingId,
                   "Forwarded %zi bytes of request", readCount);
    }



    if ((buff[readCount - 3] == '\n' &&
         buff[readCount - 1] == '\n') ||
            (buff[readCount - 1] != '\n'  && !isCacheMethod(buff))
            )
    {
        connection->buffer_size = 0;
        connection->left_to_download = -1;
        connection->connectionStatus = CS_FORWARDING_RESPONSE;
    }
    return 0;
}


static int respondingFromCache(connection_t *conn){
    cacheEntry_t *entry = conn->cacheEntry;
    if(entry == NULL)
        return -1;

    char *sendData = entry->data + conn->cacheBytesWritten;
    size_t bytesToWrite = entry->dataCount - conn->cacheBytesWritten;
    if (bytesToWrite > WRITE_BY)
    {
        bytesToWrite = WRITE_BY;
    }

    logg_track(LL_VERBOSE, conn->trackingId,
               "Responding from cache %d bytes", bytesToWrite);
    ssize_t bytesWritten = write(conn->clientSocket, sendData, bytesToWrite);


    logg_track(LL_VERBOSE, conn->trackingId,
               "Wrote %zi bytes from cache", bytesWritten);

    conn->cacheBytesWritten += bytesWritten;
    if (conn->cacheBytesWritten >= entry->dataCount)
    {
        logg_track(LL_INFO, conn->trackingId,
                   "Transmission over, responded from cache");

        close(conn->serverSocket);

        return -1;
    }

    return 0;
}

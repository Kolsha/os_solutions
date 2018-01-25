#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <assert.h>
#include <signal.h>

#include "logger.h"
#include "address_utils.h"
#include "connection.h"
#include "cache.h"

void readArgs(int argc, const char *argv[], uint16_t *outLocalPort)
{
    if (argc < 2)
        logg(LL_ERROR, "readArgs", "Usage: %s localPort", argv[0]);

    char *end = NULL;
    long localPort = strtol(argv[1], &end, 10);

    if (*end != '\0' || localPort <= 0 || localPort >= UINT16_MAX)
        logg(LL_ERROR, "readArgs", "Incorrect Port format");

    *outLocalPort = (uint16_t) localPort;
}

static SOCKET listenSocket;


void cache_dtor(Pointer data){
    if(data == NULL)
        return ;
    cacheEntry_t *entry = (cacheEntry_t *) data;
    free(entry->data);
    free(entry->url);
    free(data);
}

static void exit_handler(void)
{
    close(listenSocket);
    pthread_mutex_destroy(&cacheLock);
    ht_destroy(&cache);
}


typedef struct _heap_info
{
  void* ar_ptr; /* Arena for this heap. */
  struct _heap_info *prev; /* Previous heap. */
  size_t size;   /* Current size in bytes. */
  size_t mprotect_size; /* Size in bytes that has been mprotected
                           PROT_READ|PROT_WRITE.  */
  /* Make sure the following data is properly aligned, particularly
     that sizeof (heap_info) + 2 * SIZE_SZ is a multiple of
     MALLOC_ALIGNMENT. */
  char pad[10];
} heap_info;




int main(int argc, const char *argv[])
{
    if (pthread_mutex_init(&cacheLock, NULL) != 0)
    {
        logg(LL_ERROR, "mutex", "cacheLock init fail");
        return 1;
    }

    printf("%d", sizeof(heap_info));
    exit(0);

    signal(SIGPIPE, SIG_IGN);


    assert(ht_init(&cache, CACHE_SIZE, NULL, cache_dtor) == 1);

    uint16_t localPort;
    readArgs(argc, argv, &localPort);

    atexit(exit_handler);
    srand((unsigned int) time(NULL));

    listenSocket = initializeListenSocket(getListenAddress(localPort));
    logg(LL_INFO, NULL, "Server started at port no. [%s%d%s]","\033[92m", localPort,"\033[0m");

    for(;;)
    {
        logg(LL_VERBOSE, "accept", "Accepting request..");
        SOCKET connectedSocket = accept(listenSocket, (struct sockaddr *) NULL, NULL);

        if (connectedSocket >= 0)
        {
            logg(LL_VERBOSE, "accept", "Accepted");

            connection_t *conn = malloc(sizeof(connection_t));

            conn->clientSocket = connectedSocket;
            //server socket
            conn->buffer = NULL;
            conn->buffer_size = 0;
            conn->connectionStatus = CS_GETTING_REQUEST;
            conn->trackingId = rand() % 9000 + CACHE_SIZE;

            logg_track(LL_INFO, conn->trackingId, "Accepted incoming connection");

            pthread_t thread;
            if(pthread_create(&thread, NULL, (void * (*)(void *)) handleConnection, conn) != 0){
                logg(LL_VERBOSE, "accept", "pthread_create fail");
                close(connectedSocket);
                free(conn);
            }
        }
    }

}

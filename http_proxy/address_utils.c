#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include "address_utils.h"
#include "http_utils.h"
#include "logger.h"

struct sockaddr_in getListenAddress(uint16_t port)
{
    struct sockaddr_in result;

    result.sin_addr.s_addr = htonl(INADDR_ANY);
    result.sin_family = AF_INET;
    result.sin_port = htons(port);

    return result;
}

struct sockaddr_in getServerAddress(char *ip, uint16_t port)
{
    struct sockaddr_in result;

    result.sin_family = AF_INET;
    result.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &result.sin_addr) <= 0)
    {
        log_error("getServerAddress", "Inet_pton error occurred");
    }

    return result;
}

int getIpByHostname(char *hostname, char *ip)
{
    struct hostent *he;
    struct in_addr **addr_list;

    if ((he = gethostbyname(hostname)) == NULL)
    {
        perror("gethostbyname");
        return 1;
    }

    addr_list = (struct in_addr **) he->h_addr_list;

    if (addr_list[0] != NULL)
    {
        strcpy(ip, inet_ntoa(*addr_list[0]));
        return 0;
    }

    return 1;
}


SOCKET initializeListenSocket(struct sockaddr_in listenAddress)
{
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (listenSocket < 0)
        logg(LL_ERROR, "initializeListenSocket", "can't create socket");

    if (bind(listenSocket, (struct sockaddr *) &listenAddress, sizeof(listenAddress)))
        logg(LL_ERROR, "initializeListenSocket", "can't bind socket");


    if (listen(listenSocket, 100))
        logg(LL_ERROR, "initializeListenSocket", "can't start listening");

    return listenSocket;
}


SOCKET initializeServerSocket(struct sockaddr_in address)
{
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (clientSocket < 0)
        logg(LL_ERROR, "initializeServerSocket", "can't create socket");

    if (connect(clientSocket, (struct sockaddr *) &address, sizeof(address)) < 0 && errno != EINPROGRESS)
        logg(LL_ERROR, "initializeServerSocket", "can't connect");

    return clientSocket;
}

SOCKET initServerSocketToUrl(char *fullUrl)
{
    char *host = getHostFromUrl(fullUrl);
    char ip[20] = {'\0'};
    getIpByHostname(host, ip);
    free(host);
    struct sockaddr_in address = getServerAddress(ip, 80);
    SOCKET serverSocket = initializeServerSocket(address);
    return serverSocket;
}

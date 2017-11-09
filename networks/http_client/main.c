#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include "urlparser.h"

#define BUF_SIZE 255

int establishConnection(struct addrinfo *info) {
    if (info == NULL) return -1;

    int clientfd;
    for (;info != NULL; info = info->ai_next) {
        if ((clientfd = socket(info->ai_family,
                               info->ai_socktype,
                               info->ai_protocol)) < 0) {
            perror("[establishConnection:socket] \n");
            continue;
        }

        if (connect(clientfd, info->ai_addr, info->ai_addrlen) < 0) {
            close(clientfd);
            perror("[establishConnection:connect] \n");
            continue;
        }

        freeaddrinfo(info);
        return clientfd;
    }

    freeaddrinfo(info);
    return -1;
}

struct addrinfo *getHostInfo(char* host, char* port) {
    int r;
    struct addrinfo hints, *getaddrinfo_res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if ((r = getaddrinfo(host, port, &hints, &getaddrinfo_res))) {
        fprintf(stderr, "[getHostInfo:getaddrinfo] %s\n", gai_strerror(r));
        return NULL;
    }

    return getaddrinfo_res;
}

// return client_fd
int http_get(char *url)
{
    /* Parse url */
    struct parsed_url *purl = parse_url(url);
    if(purl == NULL)
    {
        printf("Unable to parse url");
        return -1;
    }

    /* Declare variable */
    char http_headers[BUFSIZ] = {0};

    /* Build query/headers */
    if(purl->path != NULL)
    {
        if(purl->query != NULL)
        {
            sprintf(http_headers, "GET /%s?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n\r\n", purl->path, purl->query, purl->host);
        }
        else
        {
            sprintf(http_headers, "GET /%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n\r\n", purl->path, purl->host);
        }
    }
    else
    {
        if(purl->query != NULL)
        {
            sprintf(http_headers, "GET /?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n\r\n", purl->query, purl->host);
        }
        else
        {
            sprintf(http_headers, "GET / HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n\r\n", purl->host);
        }
    }


    int clientfd = establishConnection(getHostInfo(purl->host, purl->port));
    if (clientfd == -1) {
        printf("Failed to connect\n");
        return -1;
    }


    send(clientfd, http_headers, strlen(http_headers), 0);

    parsed_url_free(purl);

    return clientfd;
}



int do_request(){
    char url[BUF_SIZE] = {0};
    int clientfd = -1;
    char buf[BUF_SIZE];
    printf("Please URL: ");
    if(scanf("%s", url) < 1){
        return 0;
    }

    clientfd = http_get(url);
    if(clientfd < 0){
        return 0;
    }


    while (recv(clientfd, buf, BUF_SIZE, 0) > 0) {
        fputs(buf, stdout);
        memset(buf, 0, BUF_SIZE);
    }

    close(clientfd);

    return 1;

}




int main() {

    while(do_request()){}

    return 0;
}

/*
 * nibir2@gmail.com
 * Парыгин Александр
 */

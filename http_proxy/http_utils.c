#include <stdlib.h>
#include <stdio.h>

#include "http_utils.h"


int isCacheMethod(char *httpData)
{
    return (strstr(httpData, "GET") == httpData) || (strstr(httpData, "HEAD") == httpData);
}

char *getUrlFromData(char *httpData, size_t dataLength)
{
    if (dataLength < 6)
    {
        return NULL;
    }

    size_t startUrl = 0;
    for (; httpData[startUrl] != ' '; ++startUrl)
    {
        if (startUrl + 1 >= dataLength)
        {
            return NULL;
        }
    }
    startUrl++;

    size_t endUrl = startUrl;
    for (; httpData[endUrl] != ' '; ++endUrl)
    {
        if (endUrl + 1 >= dataLength)
        {
            return NULL;
        }
    }

    char *result = malloc(sizeof(char) * (endUrl - startUrl + 1));
    if(result == NULL){
        return NULL;
    }

    memcpy(result, httpData + startUrl, (size_t) (endUrl - startUrl));
    result[endUrl - startUrl] = '\0';

    return result;
}

char *getHostFromUrl(char *url)
{
    url += 7; // Skip http://

    size_t endHost = 0;
    for (; url[endHost] != '/' && url[endHost] != '\0'; ++endHost) ;

    char * result = malloc(sizeof(char) * (endHost + 1));
    if(result == NULL){
        return NULL;
    }

    memcpy(result, url, endHost);
    result[endHost] = '\0';

    return result;
}

int getResponseCodeFromData(char *httpData)
{
    httpData += 9;

    char *end = NULL;

    int response = (int) strtol(httpData, &end, 10);
    if (end == httpData || response <= 0)
    {
        return -1;
    }
    return response;
}

long getContentLengthFromData(char *httpData, size_t dataLength)
{
    char   *headerStart = "Content-Length:";
    size_t headerStartLen = strlen(headerStart);

    size_t i = 0;
    while (1)
    {
        while (httpData[i] != '\n')
        {
            if (i + 1 >= dataLength)
            {
                return -1;
            }
            i++;
        }

        i++;

        int matches = 1;
        for (size_t j = 0; j < headerStartLen; ++j)
        {
            if (httpData[i + j] != headerStart[j])
            {
                matches = 0;
                break;
            }
        }

        if (matches)
        {
            i += headerStartLen;

            char *end = NULL;

            long contentLength = strtol(httpData + i, &end, 10);
            if (end == httpData + i || contentLength <= 0)
            {

                return -1;
            }
            return contentLength;
        }
    }
}


int isResponseHasPayload(int statusCode)
{

    return !((statusCode == 204) ||
             (statusCode == 304) ||
             (statusCode >= 100 && statusCode < 200));
}





// Edit request to proxy server to make it valid to be send to target server
void editRequestToSendToServer(connection_t *connection)
{
    int firstSpace = 0;
    while (connection->buffer[firstSpace] != ' ')
    {
        firstSpace++;
    }

    int firstSlash = firstSpace + 8; // +8 to skip http://
    while (connection->buffer[firstSlash] != '/')
    {
        firstSlash++;
    }

    memmove(connection->buffer + firstSpace + 1, connection->buffer + firstSlash,
            connection->buffer_size - firstSlash);
    connection->buffer_size -= firstSlash - (firstSpace + 1);
}

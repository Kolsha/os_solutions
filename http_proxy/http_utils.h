#pragma once
#include <stdlib.h>
#include <memory.h>
#include "connection.h"




int isCacheMethod(char * httpData);


char *getUrlFromData(char *httpData, size_t  dataLength);

char *getHostFromUrl(char *url);

int getResponseCodeFromData(char *httpData);

long getContentLengthFromData(char *httpData, size_t dataLength);

int isMethodHasPayload(char *method);

int isResponseHasPayload(int statusCode);

void editRequestToSendToServer(connection_t *connection);



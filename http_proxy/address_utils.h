#pragma once

#include "common.h"


struct sockaddr_in getListenAddress(uint16_t port);

struct sockaddr_in getServerAddress(char *ip, uint16_t port);

int getIpByHostname(char *hostname, char *ip);

SOCKET initializeListenSocket(struct sockaddr_in listenAddress);

SOCKET initializeServerSocket(struct sockaddr_in address);

SOCKET initServerSocketToUrl(char *fullUrl);

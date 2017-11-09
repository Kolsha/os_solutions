#pragma once
#include <stdio.h>

#define CONNMAX 1000
#define BYTES 1024

typedef char bool;
#define true 1;
#define false 0;

#define MIME_TYPE_FN "mime-types"

#define SERVER_STRING "Server: Kolsha/0.1.0\r\n"

bool initServer(int argc, char* argv[]);
void listen_server();
void respond(int n);



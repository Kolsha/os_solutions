#include <stdio.h>
#include <signal.h>
#include "my_server.h"


int main(int argc, char* argv[])
{
    if(!init_server(argc, argv)){
        fprintf(stderr, "Failed init server\n");
        return 1;
    }

    listen_server();
    return 0;
}



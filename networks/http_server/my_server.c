#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
//#include<sys/types.h>
//#include<sys/stat.h>
//#include<sys/socket.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<fcntl.h>
#include<time.h>
#include<signal.h>

#include "my_server.h"


static char *ROOT = NULL;
static int server_sock = -1, clients[CONNMAX] = {-1};

static FILE *stdlog = NULL;


bool start_server(char *);


void handle_http_code(size_t n, int code){
    char buf[1024] = {'\0'};
    if(clients[n] == -1){
        return ;
    }
    switch (code) {
    case 200:
        sprintf(buf, "HTTP/1.0 200 OK\n");
        send(clients[n], buf, strlen(buf), 0);
        sprintf(buf, SERVER_STRING);
        send(clients[n], buf, strlen(buf), 0);
        return ;
        break;
    case 404:
        sprintf(buf, "HTTP/1.0 404 NOT FOUND\n");
        send(clients[n], buf, strlen(buf), 0);

        break;
    case 400:
        sprintf(buf, "HTTP/1.0 400 Bad Request\n");
        send(clients[n], buf, strlen(buf), 0);
        break;
    case 501:
        sprintf(buf, "HTTP/1.0 501 Method Not Implemented\n");
        send(clients[n], buf, strlen(buf), 0);
        break;
    default:
        sprintf(buf, "HTTP/1.0 500 Internal Server Error\n");
        send(clients[n], buf, strlen(buf), 0);
        break;
    }
    sprintf(buf, SERVER_STRING);
    send(clients[n], buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\n");
    send(clients[n], buf, strlen(buf), 0);

}


char *get_mime_type(char *name) {
    const char delimiters[] = " ";
    const char *mime_type_def = "text/html";



    char *mime_type = malloc (128 * sizeof(char)) ;
    if(mime_type == NULL){
        return NULL;
    }
    strcpy(mime_type, mime_type_def);
    char *ext = strrchr(name, '.');
    if(ext == NULL){
        return mime_type;
    }
    ext++;

    char line[128] = {'\0'};
    char *token = NULL;


    FILE *mime_type_file = fopen(MIME_TYPE_FN, "r");
    if (mime_type_file != NULL) {
        while(fgets(line, sizeof(line), mime_type_file) != NULL) {

            if((token = strtok(line, delimiters)) != NULL) {
                if(strcmp(token, ext) == 0) {
                    token = strtok(NULL, delimiters);
                    strcpy(mime_type, token);
                    break;
                }
            }
        }
        fclose( mime_type_file );
    } else {
        perror("Mime-type open");
    }
    return mime_type;
}

static void sig_handler(int signo)
{
    if (signo == SIGINT || signo == SIGQUIT){
        printf("\nPoka)0)\n");
        close(server_sock);
        fclose(stdlog);
        exit(0);
    }
}

bool init_server(int argc, char *argv[])
{
    char c = '\0';
    char PORT[6] = "1234";
    ROOT = getenv("PWD");

    while ((c = getopt (argc, argv, "p:r:")) != -1)
        switch (c)
        {
        case 'r':
            ROOT = malloc(strlen(optarg));
            strcpy(ROOT, optarg);
            break;
        case 'p':
            strcpy(PORT, optarg);
            break;
        default:
            fprintf(stderr, "Wrong arguments given\n");

        }



    for (size_t i = 0; i < CONNMAX; i++){
        clients[i] = -1;
    }

    if(!start_server(PORT)){
        return false;
    }

    printf("Server started at port no. %s%s%s with root directory as %s%s%s\n","\033[92m",PORT,"\033[0m","\033[92m",ROOT,"\033[0m");
    {
        char path[255] = {'\0'};
        strcpy(path, ROOT);
        strcpy(&path[strlen(ROOT)], "/log.log");

        stdlog = fopen(path, "a");
        if(stdlog == NULL){
            stdlog = stdout;
        }
    }

    if (signal(SIGINT, sig_handler) == SIG_ERR)
        printf("\ncan't catch SIGINT\n");
    if (signal(SIGQUIT, sig_handler) == SIG_ERR)
        printf("\ncan't catch SIGQUIT\n");



    return true;
}


bool start_server(char *port)
{
    if(port == NULL){
        fprintf(stderr, "Port is empty\n");
        return false;
    }

    struct addrinfo hints, *res, *p;

    memset (&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo(NULL, port, &hints, &res) != 0)
    {
        perror("getaddrinfo() error");
        return 0;
    }

    for (p = res; p != NULL; p = p->ai_next)
    {
        server_sock = socket(p->ai_family, p->ai_socktype, 0);
        if (server_sock == -1){
            continue;
        }
        if (bind(server_sock, p->ai_addr, p->ai_addrlen) == 0){
            break;
        }
    }
    if (p == NULL)
    {
        perror ("socket() or bind()");
        return false;
    }

    freeaddrinfo(res);


    if (listen(server_sock, CONNMAX) != 0)
    {
        perror("listen() error");
        return false;
    }

    return true;
}


void respond(int n)
{
    clock_t time = clock();
    const size_t msg_sz = 99999;
    char msg[msg_sz], *reqline[3], data_to_send[BYTES], path[255];
    int rcvd, bytes_read;

    memset( (void*)msg, '\0', msg_sz );

    rcvd = recv(clients[n], msg, msg_sz, 0);

    if (rcvd < 0){
        fprintf(stderr,("recv() error\n"));
    }
    else if (rcvd == 0){
        fprintf(stderr, "Client disconnected upexpectedly.\n");
    }
    else{
        fprintf(stdlog, "%s", msg);
        reqline[0] = strtok (msg, " \t\n");
        if ( strncmp(reqline[0], "GET\0", 4)==0 )
        {
            reqline[1] = strtok (NULL, " \t");
            reqline[2] = strtok (NULL, " \t\n");
            if ( strncmp( reqline[2], "HTTP/1.0", 8)!=0 && strncmp( reqline[2], "HTTP/1.1", 8)!=0 )
            {

                handle_http_code(n, 400);
            }
            else
            {
                if ( strncmp(reqline[1], "/", 2)==0 ){
                    reqline[1] = "/index.html";
                }


                strcpy(path, ROOT);
                strcpy(&path[strlen(ROOT)], reqline[1]);
                fprintf(stdlog, "file: %s\n", path);

                FILE *fd = fopen(path, "r");
                if (fd != NULL)    //FILE FOUND
                {
                    handle_http_code(n, 200);
                    char buffer[255];

                    { // File size || Content Length
                        fseek (fd , 0 , SEEK_END);
                        size_t file_size = ftell(fd);
                        rewind (fd);
                        sprintf(buffer, "Content-Length: %zu\n" , file_size);
                        send(clients[n], buffer, strlen(buffer), 0);
                    }

                    char *mime = get_mime_type(path);
                    if(mime != NULL){
                        sprintf(buffer, "Content-Type: %s\n", mime);
                        free(mime);

                        send(clients[n], buffer, strlen(buffer), 0);
                    }

                    sprintf(buffer, "\n");
                    send(clients[n], buffer, strlen(buffer), 0);


                    while ((bytes_read = fread(data_to_send, 1, BYTES, fd)) > 0){ //( (bytes_read = read(fd, data_to_send, BYTES)) > 0 ){
                        send(clients[n], data_to_send, bytes_read, 0);
                    }

                    fclose(fd);

                }
                else{  //FILE NOT FOUND
                    handle_http_code(n , 404);
                }
            }
        }else{
            handle_http_code(n, 501);
        }
    }

    //Closing SOCKET
    shutdown(clients[n], SHUT_RDWR);
    close(clients[n]);
    clients[n] = -1;
    time = clock() - time;
    printf("Time of processing: %f\n", (double) time / CLOCKS_PER_SEC);
}

void listen_server()
{
    struct sockaddr_in client_name;
    socklen_t client_name_len = sizeof(client_name);
    size_t slot = 0;

    for(;;){

        clients[slot] = accept(server_sock, (struct sockaddr *) &client_name, &client_name_len);

        if (clients[slot] < 0){
            perror ("accept() error");
        }
        else{
            char *ip = inet_ntoa(client_name.sin_addr);
            printf("Client: %s\n", ip);
            if ( fork() == 0 ){
                respond(slot);
                exit(0);
            }
        }

        while (clients[slot] != -1) slot = (slot + 1) % CONNMAX;
    }

    close(server_sock);
    fclose(stdlog);
}

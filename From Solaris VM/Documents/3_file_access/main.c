#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>


int main (int argc, char **argv){

    printf("real user ID %ld \n", getuid());
    printf("effective user ID %ld \n", geteuid());

    if (argc != 2) {
        fprintf(stderr, "Need file");
        return EXIT_FAILURE;
    }

    char *file_name = argv[1];
    if(file_name == NULL){
        fprintf(stderr, "Nulled file name");
        return EXIT_FAILURE;
    }
    FILE * fs = fopen(file_name, "r");
    if (fs == NULL){
        perror("File not open");
    }else{
        printf("File successfully opened\n");
    }

    fclose(fs);

    uid_t uid = getuid();
    setuid(uid);

    printf("after using setuid \n");
    printf("real user ID %ld \n", getuid());
    printf("effective user ID %ld \n", geteuid());

    fs = fopen(file_name, "r");
    if (fs == NULL){
        perror("File not open");
    }else{
        printf("File successfully opened\n");
    }

    fclose(fs);

    return EXIT_SUCCESS;
}


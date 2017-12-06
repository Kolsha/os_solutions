#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "dslist.h"


void print_arr(Pointer data, Pointer user_data){
    assert(data != NULL);
    assert(user_data == NULL);
    printf("%s\n", (char*) data);

}


int main() {


    DLList *list = NULL;
    printf("Please, enter string:\n");
    for(;;){
        char *tmp_str = read_line();
        if(tmp_str == NULL){
            perror("Read error, view list");
            break;
        }
        if(0 == strcmp(tmp_str, ".")){
            free(tmp_str);
            break;
        }
        DLList *tmp_list = dslist_append(list, (Pointer)tmp_str);
        if(tmp_list != NULL){
            list = tmp_list;
        }

    }

    if(list == NULL){
        perror("Empty list, exit");
        return EXIT_FAILURE;
    }
    system("clear");
    printf("Your lines: \n");

    dslist_foreach(list, print_arr, NULL);
    dslist_free(list);

    return EXIT_SUCCESS;
}

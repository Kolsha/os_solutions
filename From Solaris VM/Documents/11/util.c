#include "util.h"

#include <stdio.h>
#include <stdlib.h>

void * checkCalloc(size_t num, size_t size) {
    void *ptr = calloc(num, size);

    if(ptr == NULL) {
        perror("Calloc error");
    }

    return ptr;
}


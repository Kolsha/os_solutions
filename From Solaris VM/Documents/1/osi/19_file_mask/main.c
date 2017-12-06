#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <fnmatch.h>
#include <glob.h>
#include <assert.h>

char * read_line() {
    size_t str_len = 0;
    size_t buf_size = 16; // some initial size
    char *buf = malloc(buf_size * sizeof(char));
    if (buf == NULL) {
        return NULL;
    }

    for (;;) {
        // read next part
        if (fgets(buf + str_len, buf_size - str_len, stdin) == NULL) {
            if (feof(stdin) && str_len > 0) {
                // read all given input
                break;
            } else {
                // no input at all or some error
                free(buf);
                return NULL;
            }
        }

        size_t added_len = strlen(buf + str_len);
        if (added_len == 0) {
            // there is some input but its length is zero
            // (e.g. "\0foo\n")
            // treat it as error
            free(buf);
            return NULL;
        }
        str_len += added_len;
        assert(str_len + 1 <= buf_size);
        assert(buf[str_len] == '\0');

        // check if we finished with reading
        if ((str_len + 1 < buf_size) ||     // read less than maximum
            (buf[str_len - 1] == '\n')) {   // read '\n' as last character
            break;
        }

        // otherwise resize buffer for reading next part
        size_t new_buf_size = buf_size * 2;
        if (new_buf_size <= buf_size) {
            // integer overflow, we will be out of memory
            free(buf);
            return NULL;
        }
        char *new_buf = realloc(buf, new_buf_size);
        if (new_buf == NULL) {
            free(buf);
            return NULL;
        }
        buf_size = new_buf_size;
        buf = new_buf;
    }

    assert(str_len > 0);

    // trim trailing '\n' if any
    if (buf[str_len - 1] == '\n') {
        buf[str_len - 1] = '\0';
    }

    // cut buffer to actual length
    char *result = realloc(buf, str_len + 1);
    if (result == NULL) {
        free(buf);
        return NULL;
    }
    return result;
}

int errfunc(const char *epath, int eerrno) {
    fprintf(stderr, "Glob error\n");
    return 0;
}



void findEntry(char *searchMask) {
    glob_t results;

    int code = glob(searchMask, GLOB_NOCHECK, errfunc, &results);
    if(code != EXIT_SUCCESS) {
        printf("%s\n", searchMask);
    }

    int i = 0;
    while(results.gl_pathv[i]) {
        printf("%s\n", results.gl_pathv[i]);
        ++i;
    }

    globfree(&results);
}

int main() {

    printf("Enter mask please: ");
    char *searchMask = read_line();
    if(searchMask == NULL) {
        return EXIT_FAILURE;
    }

    findEntry(searchMask);

    free(searchMask);
    return EXIT_SUCCESS;
}

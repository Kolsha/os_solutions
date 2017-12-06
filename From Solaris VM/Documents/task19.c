#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <assert.h>
#include <dirent.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>

char *read_line() {
    size_t str_len = 0;
    size_t buf_size = 16; // Initial size
    char *buf = malloc(sizeof(char) * buf_size);
    if (buf == NULL) {
        fprintf(stderr, "Failed to allocate memory for a line");
        exit(1);
    }
    for (;;) {
        // read next part
        if (!fgets(buf + str_len, buf_size - str_len, stdin)) {
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
            // treat as error
            free(buf);
            return NULL;
        }
        str_len += added_len;
        assert(str_len + 1 <= buf_size);
        assert(buf[str_len] == '\0');

        // check if we finished reading
        if (str_len + 1 < buf_size ||  // read less than maximum
            buf[str_len - 1] == '\n') { // read '\n' as last character
            break;
        }

        // otherwise resize buffer for reading next part
        size_t new_buf_size = buf_size * 2;
        char *new_buf = realloc(buf, new_buf_size);
        if (!new_buf) {
            fprintf(stderr, "Failed to allocate memory for a line");
            free(buf);
            exit(1);
        }
        buf_size = new_buf_size;
        buf = new_buf;
    }
    assert(str_len > 0);

    //trim trailing '\n' if any
    if (buf[str_len - 1] == '\n') {
        buf[str_len - 1] = '\0';
    }

    // cut buffer to actual length
    char *new_buf = realloc(buf, str_len + 1);
    if (!new_buf) {
        fprintf(stderr, "Failed to allocate memory for a line");
        free(buf);
        exit(1);
    }
    return new_buf;
}

int pattern_match(const char *mask, const char *str) {
    char const *star = NULL, *prestar_str = NULL;
    for (;;) {
        char c = *str++;
        char d = *mask++;

        switch (d) {
            case '?':
                if (c == '\0') {
                    return 0;
                }
                break;
            case '*':
                if (*mask == '\0') {
                    return 1;
                }
                star = mask;
                prestar_str = --str;  // To allow zero-length matching
                break;
            case '/':
                d = *mask++;
                // Fallthrough
            default:
                if (c == d) {
                    if (d == '\0') {
                        return 1;
                    }
                    break;
                }

                if (c == '\0' || !star) {
                    return 0;
                }
                /* Try again from last *, one character later in str. */
                mask = star;
                str = ++prestar_str;
        }
    }
}

char *slashBeforeWildcards(char *file) {
    char *ret = NULL;
    for (int i = 0; file[i] != '\0'; ++i) {
        if (file[i] == '/') {
            ret = file + i;
        } else if (file[i] == '?' || file[i] == '*') {
            break;
        }
    }
    return ret;
}

char *nextSlash(char *file) {
    for (int i = 0; file[i] != '\0'; ++i) {
        if (file[i] == '/') {
            return file + i;
        }
    }
    return NULL;
}

int listDir(const char *name, char *mask) {
    DIR *dir = opendir(name);
    if (!dir) {
        perror(name);
        return 0;
    }
    struct dirent *entry;
    char *slash = nextSlash(mask);
    const char *fmt = name[strlen(name) - 1] == '/' ? "%s%s" : "%s/%s";
    // if (pattern_match(mask, "")) {
        // printf("%s\n", name);
    // }
    int found = 0;
    while ((entry = readdir(dir))) {
        char path[PATH_MAX + 1];
        snprintf(path, sizeof(path), fmt, name, entry->d_name);

        // if (!strcmp(".", entry->d_name) || !strcmp("..", entry->d_name)) {
          // continue;
        // }

        if (strcmp(".", entry->d_name) && strcmp("..", entry->d_name) &&
            pattern_match(mask, entry->d_name)) {
            printf("%s\n", path);
            found = 1;
        }

        struct stat sb;
        if (lstat(path, &sb) == -1) {
            perror(path);
            continue;
        }
        if (S_ISDIR(sb.st_mode) && slash) {
          // Match only current dir in the mask
            *slash = '\0';
            int match = 0;
            if (pattern_match(mask, entry->d_name)) {
                match = 1;
            }
            *slash = '/';
            if (match) {
                if (listDir(path, slash + 1)) {
                    found = 1;
                }
            }
        }
    }
    closedir(dir);
    return found;
}

int findFiles(char *mask) {
    char *slash = slashBeforeWildcards(mask);
    char name[NAME_MAX + 1];
    if (slash) {
        if ((slash - mask) + 1 >= sizeof(name)) {
            return 0;
        }
        snprintf(name, slash - mask + 2, "%s", mask);
        return listDir(name, slash + 1);
    } else {
        snprintf(name, sizeof(name), "%s", ".");
        return listDir(name, mask);
    }
}

int main() {
    printf("Enter the mask:\n> ");
    char *mask = read_line();

    if (!mask) {
      printf("No input\n");
      exit(1);
    }
    if (!findFiles(mask)) {
        printf("No matches found: %s\n", mask);
    }

    free(mask);
    return 0;
}

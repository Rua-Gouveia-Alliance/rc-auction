#define _XOPEN_SOURCE 500
#include "util.h"
#include <ctype.h>
#include <ftw.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

bool is_numeric(const char *str) {
    while (*str) {
        if (!isdigit(*str)) {
            return false; // Return false if any character is not a digit
        }
        str++;
    }
    return true; // Return true if all characters are digits
}

bool is_alphanumeric(const char *str) {
    while (*str) {
        if (!isalnum(*str) && *str != '-' && *str != '_') {
            return false; // Return false if any character is not alphanumeric
        }
        str++;
    }
    return true; // Return true if all characters are alphanumeric
}

char *get_filename(char *dir, char *id, char *ext) {
    int total_size = strlen(dir) + strlen(id) + strlen(ext);
    char *filename = (char *)malloc((total_size + 1) * sizeof(char));
    memset(filename, 0, (total_size + 1) * sizeof(char));

    strcpy(filename, dir);
    strcpy(filename + strlen(dir), id);
    strcpy(filename + strlen(dir) + strlen(id), ext);
    return filename;
}

bool path_exists(char *path) {
    struct stat st = {0};
    if (stat(path, &st) == -1)
        return false;
    return true;
}

int nftw_remove(const char *path, const struct stat *sb, int typeflag,
                struct FTW *ftwbuf) {
    int rv = remove(path);

    if (rv)
        perror(path);

    return rv;
}

int rmrf(char *path) {
    return nftw(path, nftw_remove, 64, FTW_DEPTH | FTW_PHYS);
}

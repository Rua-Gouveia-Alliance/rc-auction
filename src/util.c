#include "util.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

char *get_filename(char *dir, char *id, char *ext, int size) {
    char *filename = (char *)malloc((size + 1) * sizeof(char));
    memset(filename, 0, (size + 1) * sizeof(char));
    if (filename == NULL) {
        printf("No more memory, shutting down.\n");
        exit(EXIT_FAILURE);
    }

    sprintf(filename, "%s%s%s", dir, id, ext);
    return filename;
}

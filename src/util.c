#include "util.h"
#include "server/db.h"
#include <ctype.h>
#include <ftw.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
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

char *time_to_str(time_t time) {
    char *time_str = malloc((DATE_TIME_SIZE + 1) * sizeof(char));
    struct tm *current_time = gmtime(&time);
    sprintf(time_str, "%4d-%02d-%02d %02d:%02d:%02d",
            current_time->tm_year + 1900, current_time->tm_mon + 1,
            current_time->tm_mday, current_time->tm_hour, current_time->tm_min,
            current_time->tm_sec);
    time_str[DATE_TIME_SIZE] = '\0';
    return time_str;
}

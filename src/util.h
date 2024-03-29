#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>
#include <time.h>

bool is_numeric(const char *str);

bool is_alphanumeric(const char *str);

char *get_filename(char *dir, char *id, char *ext);

char *get_filename_from_path(char *path);

bool path_exists(char *path);

char *time_to_str(time_t time);

char *i_to_aid(int i);

int count_entries(char *path, int type);

char *remove_extension(char *file);

bool is_lowercase(const char *str);

long int file_size(char *file);

#endif

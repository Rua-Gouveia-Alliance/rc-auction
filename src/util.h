#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>

bool is_numeric(const char *str);

bool is_alphanumeric(const char *str);

char *get_filename(char *dir, char *id, char *ext, int size);

#endif

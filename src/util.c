#include "util.h"
#include <ctype.h>
#include <stdbool.h>

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
        if (!isalnum(*str)) {
            return false; // Return false if any character is not alphanumeric
        }
        str++;
    }
    return true; // Return true if all characters are alphanumeric
}
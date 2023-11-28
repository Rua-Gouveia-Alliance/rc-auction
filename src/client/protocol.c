#include "protocol.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *login_msg(char *uid, char *password) {
    // setup buffer
    char *msg = malloc(20 * sizeof(char));
    memset(msg, 0, 20 * sizeof(char));

    // copy data
    strcpy(msg, "LIN ");
    strcpy(&msg[4], uid);
    strcpy(&msg[11], password);

    return msg;
}
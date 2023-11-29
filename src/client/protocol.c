#include "protocol.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *login_req(char *uid, char *password) {
    // setup buffer
    char *msg = malloc((LOGIN_MSG_SIZE + 1) * sizeof(char));
    memset(msg, 0, (LOGIN_MSG_SIZE + 1) * sizeof(char));

    // copy data
    strcpy(msg, LOGIN_CMD);
    strcpy(&msg[CMD_SIZE + 1], uid);
    msg[CMD_SIZE + 1 + UID_SIZE] = ' ';
    strcpy(&msg[CMD_SIZE + 1 + UID_SIZE + 1], password);
    msg[CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE] = '\n';

    return msg;
}

char *logout_req(char *uid, char *password) {
    // setup buffer
    char *msg = malloc((LOGOUT_MSG_SIZE + 1) * sizeof(char));
    memset(msg, 0, (LOGOUT_MSG_SIZE + 1) * sizeof(char));

    // copy data
    strcpy(msg, LOGOUT_CMD);
    strcpy(&msg[CMD_SIZE + 1], uid);
    msg[CMD_SIZE + 1 + UID_SIZE] = ' ';
    strcpy(&msg[CMD_SIZE + 1 + UID_SIZE + 1], password);
    msg[CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE] = '\n';

    return msg;
}
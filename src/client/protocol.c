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

bool login_res(char *response, bool print) {
    if (strcmp(response, LOGIN_RES_OK) == 0) {
        if (print)
            printf("login successful\n");
        return true;
    } else if (strcmp(response, LOGIN_RES_NOK) == 0) {
        if (print)
            printf("wrong password\n");
        return false;
    } else if (strcmp(response, LOGIN_RES_REG) == 0) {
        if (print)
            printf("new user registred\nlogin successful\n");
        return true;
    } else {
        if (print)
            printf("error: wrong server response format\n");
        return false;
    }
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

bool logout_res(char *response, bool print) {
    if (strcmp(response, LOGOUT_RES_OK) == 0) {
        if (print)
            printf("logout successful\n");
        return true;
    } else if (strcmp(response, LOGOUT_RES_NOK) == 0) {
        if (print)
            printf("error: user not logged in\n");
        return false;
    } else if (strcmp(response, LOGOUT_RES_UNR) == 0) {
        if (print)
            printf("error: user not registered\n");
        return true;
    } else {
        if (print)
            printf("error: wrong server response format\n");
        return false;
    }
}
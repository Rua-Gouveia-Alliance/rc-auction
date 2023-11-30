#include "protocol.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *login_req(char *uid, char *password) {
    // setup buffer
    char *msg = malloc((LIN_SIZE + 1) * sizeof(char));
    memset(msg, 0, (LIN_SIZE + 1) * sizeof(char));

    // copy data
    strcpy(msg, LIN_REQ);
    strcpy(&msg[CMD_SIZE + 1], uid);
    msg[CMD_SIZE + 1 + UID_SIZE] = ' ';
    strcpy(&msg[CMD_SIZE + 1 + UID_SIZE + 1], password);
    msg[CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE] = '\n';

    return msg;
}

bool login_res(char *response, bool print) {
    if (strcmp(response, RLI_OK) == 0) {
        if (print)
            printf("login successful\n");
        return true;
    } else if (strcmp(response, RLI_NOK) == 0) {
        if (print)
            printf("wrong password\n");
        return false;
    } else if (strcmp(response, RLI_REG) == 0) {
        if (print)
            printf("new user registered\nlogin successful\n");
        return true;
    } else {
        if (print)
            printf("error: wrong server response format\n");
        return false;
    }
}

char *logout_req(char *uid, char *password) {
    // setup buffer
    char *msg = malloc((LOU_SIZE + 1) * sizeof(char));
    memset(msg, 0, (LOU_SIZE + 1) * sizeof(char));

    // copy data
    strcpy(msg, LOU_REQ);
    strcpy(&msg[CMD_SIZE + 1], uid);
    msg[CMD_SIZE + 1 + UID_SIZE] = ' ';
    strcpy(&msg[CMD_SIZE + 1 + UID_SIZE + 1], password);
    msg[CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE] = '\n';

    return msg;
}

bool logout_res(char *response, bool print) {
    if (strcmp(response, RLO_OK) == 0) {
        if (print)
            printf("logout successful\n");
        return true;
    } else if (strcmp(response, RLO_NOK) == 0) {
        if (print)
            printf("error: user not logged in\n");
        return false;
    } else if (strcmp(response, RLO_UNR) == 0) {
        if (print)
            printf("error: user not registered\n");
        return true;
    } else {
        if (print)
            printf("error: wrong server response format\n");
        return false;
    }
}

char *unregister_req(char *uid, char *password) {
    // setup buffer
    char *msg = malloc((UNR_SIZE + 1) * sizeof(char));
    memset(msg, 0, (UNR_SIZE + 1) * sizeof(char));

    // copy data
    strcpy(msg, UNR_REQ);
    strcpy(&msg[CMD_SIZE + 1], uid);
    msg[CMD_SIZE + 1 + UID_SIZE] = ' ';
    strcpy(&msg[CMD_SIZE + 1 + UID_SIZE + 1], password);
    msg[CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE] = '\n';

    return msg;
}

bool unregister_res(char *response, bool print) {
    if (strcmp(response, RUR_OK) == 0) {
        if (print)
            printf("unregister successful\n");
        return true;
    } else if (strcmp(response, RUR_NOK) == 0) {
        if (print)
            printf("error: user not logged in\n");
        return false;
    } else if (strcmp(response, RUR_UNR) == 0) {
        if (print)
            printf("error: user not registered\n");
        return true;
    } else {
        if (print)
            printf("error: wrong server response format\n");
        return false;
    }
}
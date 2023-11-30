#include "protocol.h"
#include "../util.h"
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

char *list_req() {
    // setup buffer
    char *msg = malloc((LST_SIZE + 1) * sizeof(char));
    memset(msg, 0, (LST_SIZE + 1) * sizeof(char));

    // copy data
    strcpy(msg, LST_REQ);
    return msg;
}

void list_res(char *response, bool print) {
    int last_pos;
    char *token, *list_pos;

    last_pos = strlen(response) - 1;
    if (strncmp(response, RLS_OK, RLS_OK_SIZE) == 0) {
        // check \n
        if (response[last_pos] != '\n') {
            if (print)
                printf("error: wrong server response format\n");
            return;
        }
        response[last_pos] = '\0'; // replace \n with \0

        list_pos = response + RLS_OK_SIZE;
        token = strtok(list_pos, " ");
        while (token != NULL) {
            // AID
            if (strlen(token) != 3 || !is_numeric(token)) {
                if (print)
                    printf("error: wrong server response format\n");
                return;
            }
            if (print)
                printf("%s", token);

            // status
            token = strtok(NULL, " ");
            if (token == NULL || strlen(token) != 1 ||
                (*token != '0' && *token != '1')) {
                if (print)
                    printf("error: wrong server response format\n");
                return;
            }
            if (print)
                printf(" (%s)\n", *token == '1' ? "active" : "inactive");

            // next token
            token = strtok(NULL, " ");
        }
    } else if (strcmp(response, RLS_NOK) == 0) {
        if (print)
            printf("no auctions started yet\n");
    } else {
        if (print)
            printf("error: wrong server response format\n");
    }
}
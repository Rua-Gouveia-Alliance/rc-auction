#include "protocol.h"
#include "../util.h"
#include "db.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int interpret_req(char *msg) {
    if (strncmp(msg, LIN_REQ, CODE_SIZE) == 0)
        return LIN;
    else if (strncmp(msg, LOU_REQ, CODE_SIZE) == 0)
        return LOU;
    else if (strncmp(msg, UNR_REQ, CODE_SIZE) == 0)
        return UNR;
    else if (strncmp(msg, LMA_REQ, CODE_SIZE) == 0)
        return LMA;
    else if (strncmp(msg, LMB_REQ, CODE_SIZE) == 0)
        return LMB;
    else if (strncmp(msg, LST_REQ, CODE_SIZE) == 0)
        return LST;
    else if (strncmp(msg, SRC_REQ, CODE_SIZE) == 0)
        return SRC;
    else if (strncmp(msg, CLS_REQ, CODE_SIZE) == 0)
        return CLS;
    else if (strncmp(msg, BID_REQ, CODE_SIZE) == 0)
        return BID;
    else if (strncmp(msg, SAS_REQ, CODE_SIZE) == 0)
        return SAS;
    else if (strncmp(msg, OPA_REQ, CODE_SIZE) == 0)
        return OPA;
    else
        return -1;
}

void parse_lin(char *msg, char *uid, char *password) {
    memset(uid, 0, UID_SIZE + 1);
    memset(password, 0, PASS_SIZE + 1);

    strncpy(uid, msg + CODE_SIZE + 1, UID_SIZE);
    strncpy(password, msg + CODE_SIZE + 1 + UID_SIZE + 1, PASS_SIZE);
}

void parse_lou(char *msg, char *uid, char *password) {
    memset(uid, 0, UID_SIZE + 1);
    memset(password, 0, PASS_SIZE + 1);

    strncpy(uid, msg + CODE_SIZE + 1, UID_SIZE);
    strncpy(password, msg + CODE_SIZE + 1 + UID_SIZE + 1, PASS_SIZE);
}

void parse_unr(char *msg, char *uid, char *password) {
    memset(uid, 0, UID_SIZE + 1);
    memset(password, 0, PASS_SIZE + 1);

    strncpy(uid, msg + CODE_SIZE + 1, UID_SIZE);
    strncpy(password, msg + CODE_SIZE + 1 + UID_SIZE + 1, PASS_SIZE);
}

void parse_opa(char *msg, char *uid, char *pass, char *name, char *start_value,
               char *timeactive, char *fname) {
    memset(uid, 0, UID_SIZE + 1);
    memset(pass, 0, PASS_SIZE + 1);
    memset(name, 0, NAME_SIZE + 1);
    memset(start_value, 0, START_VAL_SIZE + 1);
    memset(timeactive, 0, TIME_SIZE + 1);

    strncpy(uid, msg + CODE_SIZE + 1, UID_SIZE);
    strncpy(pass, msg + CODE_SIZE + 1 + UID_SIZE + 1, PASS_SIZE);

    char *token =
        strtok(msg + CODE_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE + 1, " ");
    strcpy(name, token);
    token = strtok(NULL, " ");
    strcpy(start_value, token);
    token = strtok(NULL, " ");
    strcpy(timeactive, token);
    token = strtok(NULL, " ");
    strcpy(fname, token);
}

void parse_cls(char *msg, char *uid, char *pass, char *aid) {
    memset(uid, 0, UID_SIZE + 1);
    memset(pass, 0, PASS_SIZE + 1);
    memset(aid, 0, AID_SIZE + 1);

    strncpy(uid, msg + CODE_SIZE + 1, UID_SIZE);
    strncpy(pass, msg + CODE_SIZE + 1 + UID_SIZE + 1, PASS_SIZE);
    strncpy(aid, msg + CODE_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE + 1, AID_SIZE);
}

char *list_res(char *code, char* list) {
    // setup buffer
    int msg_size = CODE_SIZE + 1 + 2 + 1 + strlen(list);
    char *msg = malloc((msg_size + 1) * sizeof(char));
    memset(msg, 0, (msg_size + 1) * sizeof(char));

    // copy data
    strcpy(msg, code);
    msg[CODE_SIZE] = ' ';
    strcpy(&msg[CODE_SIZE + 1], "OK");
    msg[CODE_SIZE + 1 + 2] = ' ';
    strcpy(&msg[CODE_SIZE + 1 + 2 + 1], list);
    msg[msg_size] = '\n';

    return msg;
}

char *default_res(char *code, char *status) {
    // setup buffer
    char *msg = malloc((RES_SIZE + 1) * sizeof(char));
    memset(msg, 0, (RES_SIZE + 1) * sizeof(char));

    // copy data
    strcpy(msg, code);
    msg[CODE_SIZE] = ' ';
    strcpy(&msg[CODE_SIZE + 1], status);

    return msg;
}

char *opa_ok_res(char *aid) {
    char *msg = malloc((OPA_RES_SIZE + 1) * sizeof(char));
    memset(msg, 0, (OPA_RES_SIZE + 1) * sizeof(char));

    strcpy(msg, OPA_RES);
    msg[CODE_SIZE] = ' ';
    strcpy(&msg[CODE_SIZE + 1], "OK");
    msg[CODE_SIZE + 1 + 2] = ' ';
    strcpy(&msg[CODE_SIZE + 1 + 2 + 1], aid);
    msg[CODE_SIZE + 1 + 2 + 1 + AID_SIZE] = '\n';
    return msg;
}

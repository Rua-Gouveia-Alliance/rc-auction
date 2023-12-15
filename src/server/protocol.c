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

int parse_lin(char *msg, char *uid, char *password) {
    memset(uid, 0, UID_SIZE + 1);
    memset(password, 0, PASS_SIZE + 1);

    char msg_cpy[DEFAULT_SIZE];
    memset(msg_cpy, 0, DEFAULT_SIZE * sizeof(char));
    strncpy(msg_cpy, msg, strlen(msg));

    char *token = strtok(msg_cpy, " ");
    if (token == NULL || strlen(token) != CODE_SIZE ||
        strcmp(token, LIN_REQ) != 0) {
        return -1;
    }

    token = strtok(NULL, " ");
    if (token == NULL || strlen(token) != UID_SIZE)
        return -1;
    strcpy(uid, token);

    token = strtok(NULL, " ");
    if (token == NULL || strlen(token) != PASS_SIZE + 1)
        return -1;
    strncpy(password, token, strlen(token) - 1);

    return 0;
}

int parse_lou(char *msg, char *uid, char *password) {
    memset(uid, 0, UID_SIZE + 1);
    memset(password, 0, PASS_SIZE + 1);

    char msg_cpy[DEFAULT_SIZE];
    memset(msg_cpy, 0, DEFAULT_SIZE * sizeof(char));
    strncpy(msg_cpy, msg, strlen(msg));

    char *token = strtok(msg_cpy, " ");
    if (token == NULL || strlen(token) != CODE_SIZE ||
        strcmp(token, LOU_REQ) != 0) {
        return -1;
    }

    token = strtok(NULL, " ");
    if (token == NULL || strlen(token) != UID_SIZE)
        return -1;
    strcpy(uid, token);

    token = strtok(NULL, " ");
    if (token == NULL || strlen(token) != PASS_SIZE + 1)
        return -1;
    strncpy(password, token, strlen(token) - 1);

    return 0;
}

int parse_unr(char *msg, char *uid, char *password) {
    memset(uid, 0, UID_SIZE + 1);
    memset(password, 0, PASS_SIZE + 1);

    char msg_cpy[DEFAULT_SIZE];
    memset(msg_cpy, 0, DEFAULT_SIZE * sizeof(char));
    strncpy(msg_cpy, msg, strlen(msg));

    char *token = strtok(msg_cpy, " ");
    if (token == NULL || strlen(token) != CODE_SIZE ||
        strcmp(token, UNR_REQ) != 0) {
        return -1;
    }

    token = strtok(NULL, " ");
    if (token == NULL || strlen(token) != UID_SIZE)
        return -1;
    strcpy(uid, token);

    token = strtok(NULL, " ");
    if (token == NULL || strlen(token) != PASS_SIZE + 1)
        return -1;
    strncpy(password, token, strlen(token) - 1);

    return 0;
}

char *fpath_from_roa(char *msg, char *fname) {
    char temp[AID_SIZE + 1];
    memset(temp, 0, AID_SIZE);
    strncpy(temp, msg + CODE_SIZE + 1 + strlen(STATUS_OK) - 1 + 1, AID_SIZE);

    char *auction = auction_dir(temp);
    char *fauc = get_filename(auction, fname, "");
    free(auction);
    return fauc;
}

bool is_roa_ok(char *msg) {
    return strncmp(msg, OPA_RES, CODE_SIZE) == 0 &&
           strncmp(msg + CODE_SIZE + 1, STATUS_OK, strlen(STATUS_OK) - 1) == 0;
}

int parse_opa(char *msg, char *uid, char *pass, char *name, char *start_value,
              char *timeactive, char *fname, char *fsize) {
    int i = 0;

    char msg_cpy[DEFAULT_SIZE];
    memset(msg_cpy, 0, DEFAULT_SIZE * sizeof(char));
    strncpy(msg_cpy, msg, strlen(msg));

    char *token = strtok(msg_cpy, " ");
    if (token == NULL || strlen(token) != CODE_SIZE ||
        strcmp(token, OPA_REQ) != 0)
        return -1;
    i += CODE_SIZE + 1;

    token = strtok(NULL, " ");
    if (token == NULL || strlen(token) != UID_SIZE)
        return -1;
    if (uid != NULL) {
        memset(uid, 0, UID_SIZE + 1);
        strcpy(uid, token);
    }
    i += UID_SIZE + 1;

    token = strtok(NULL, " ");
    if (token == NULL || strlen(token) != PASS_SIZE)
        return -1;
    if (pass != NULL) {
        memset(pass, 0, PASS_SIZE + 1);
        strcpy(pass, token);
    }
    i += PASS_SIZE + 1;

    token = strtok(NULL, " ");
    if (token == NULL || strlen(token) > NAME_SIZE)
        return -1;
    if (name != NULL) {
        memset(name, 0, NAME_SIZE + 1);
        strcpy(name, token);
    }
    i += strlen(token) + 1;

    token = strtok(NULL, " ");
    if (token == NULL || strlen(token) > START_VAL_SIZE)
        return -1;
    if (start_value != NULL) {
        memset(start_value, 0, START_VAL_SIZE + 1);
        strcpy(start_value, token);
    }
    i += strlen(token) + 1;

    token = strtok(NULL, " ");
    if (token == NULL || strlen(token) > TIME_SIZE)
        return -1;
    if (timeactive != NULL) {
        memset(timeactive, 0, TIME_SIZE + 1);
        strcpy(timeactive, token);
    }
    i += strlen(token) + 1;

    token = strtok(NULL, " ");
    if (token == NULL || strlen(token) > FNAME_SIZE)
        return -1;
    if (fname != NULL) {
        memset(fname, 0, FNAME_SIZE + 1);
        strcpy(fname, token);
    }
    i += strlen(token) + 1;

    token = strtok(NULL, " ");
    if (token == NULL || strlen(token) > FSIZE_SIZE)
        return -1;
    if (fsize != NULL) {
        memset(fsize, 0, FSIZE_SIZE + 1);
        strcpy(fsize, token);
    }
    i += strlen(token) + 1;

    return i;
}

int parse_cls(char *msg, char *uid, char *pass, char *aid) {
    memset(uid, 0, UID_SIZE + 1);
    memset(pass, 0, PASS_SIZE + 1);
    memset(aid, 0, AID_SIZE + 1);

    char msg_cpy[DEFAULT_SIZE];
    memset(msg_cpy, 0, DEFAULT_SIZE * sizeof(char));
    strncpy(msg_cpy, msg, strlen(msg));

    char *token = strtok(msg_cpy, " ");
    if (token == NULL || strlen(token) != CODE_SIZE ||
        strcmp(token, CLS_REQ) != 0) {
        return -1;
    }

    token = strtok(NULL, " ");
    if (token == NULL || strlen(token) != UID_SIZE)
        return -1;
    strcpy(uid, token);

    token = strtok(NULL, " ");
    if (token == NULL || strlen(token) != PASS_SIZE)
        return -1;
    strcpy(pass, token);

    token = strtok(NULL, " ");
    if (token == NULL || strlen(token) != AID_SIZE + 1)
        return -1;
    strncpy(aid, token, strlen(token) - 1);

    return 0;
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

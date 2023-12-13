#include "protocol.h"
#include "../util.h"
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
    strncpy(uid, msg + CODE_SIZE + 1, UID_SIZE);
    strncpy(password, msg + CODE_SIZE + 1 + UID_SIZE + 1, PASS_SIZE);
    msg[CODE_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE] = '\0';
}

char *default_res(char *code, char *status) {
    // setup buffer
    char *msg = malloc((RES_SIZE) * sizeof(char));
    memset(msg, 0, (RES_SIZE) * sizeof(char));

    // copy data
    strcpy(msg, code);
    strcpy(&msg[CODE_SIZE + 1], status);
    msg[CODE_SIZE + 1 + STATUS_SIZE] = '\n';

    return msg;
}

char *open_auction_res(char *res, char *status, char *list);

char *auction_list_res(char *res, char *status, char *list);

char *bid_list_res(char *res, char *status, char *list);

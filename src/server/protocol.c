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

bool valid_fname(char* fname) {
    char* token;
    
    if (strlen(fname) < 5 || strlen(fname) > 24)
        return false;

    // File name
    token = strtok(fname, ".");
    if (strlen(token) < 1 || !is_alphanumeric(token))
        return false;

    // File extension
    token = strtok(NULL, " ");
    if (strlen(token) != 3 || !is_lowercase(token))
        return false;

    return true;
}

bool valid_aid(char* aid) {
    if (!is_numeric(aid))
        return false;

    int parsed_aid = atoi(aid);
    return parsed_aid > 0 && parsed_aid < 100;
}

bool valid_uid(char* uid) {
    return is_numeric(uid) && strlen(uid) == UID_SIZE;
}

bool valid_password(char* password) {
    return is_alphanumeric(password) && strlen(password) == PASS_SIZE;
}

int parse_lin(char *msg, char *uid, char *password) {
    int last_pos = strlen(msg) - 1;
    char *token;

    // check \n
    if (msg[last_pos] != '\n') {
        return -1;
    }
    msg[last_pos] = '\0'; // replace \n with \0

    memset(uid, 0, UID_SIZE + 1);
    memset(password, 0, PASS_SIZE + 1);

    token = strtok(msg, " ");

    // UID
    token = strtok(NULL, " ");
    if (!valid_uid(token))
        return -1;

    strncpy(uid, token, UID_SIZE);
    uid[UID_SIZE] = '\0';

    // password
    token = strtok(NULL, " ");
    if (!valid_password(token))
        return -1;

    strncpy(password, token, PASS_SIZE);
    password[PASS_SIZE] = '\0';

    return 0;
}

int parse_lou(char *msg, char *uid, char *password) {
    int last_pos = strlen(msg) - 1;
    char *token;

    // check \n
    if (msg[last_pos] != '\n') {
        return -1;
    }
    msg[last_pos] = '\0'; // replace \n with \0

    memset(uid, 0, UID_SIZE + 1);
    memset(password, 0, PASS_SIZE + 1);

    token = strtok(msg, " ");

    // UID
    token = strtok(NULL, " ");
    if (!valid_uid(token))
        return -1;

    strncpy(uid, token, UID_SIZE);
    uid[UID_SIZE] = '\0';

    // password
    token = strtok(NULL, " ");
    if (!valid_password(token))
        return -1;

    strncpy(password, token, PASS_SIZE);
    password[PASS_SIZE] = '\0';

    return 0;
}

int parse_unr(char *msg, char *uid, char *password) {
    int last_pos = strlen(msg) - 1;
    char *token;

    // check \n
    if (msg[last_pos] != '\n') {
        return -1;
    }
    msg[last_pos] = '\0'; // replace \n with \0

    memset(uid, 0, UID_SIZE + 1);
    memset(password, 0, PASS_SIZE + 1);

    token = strtok(msg, " ");

    // UID
    token = strtok(NULL, " ");
    if (!valid_uid(token))
        return -1;

    strncpy(uid, token, UID_SIZE);
    uid[UID_SIZE] = '\0';

    // password
    token = strtok(NULL, " ");
    if (!valid_password(token))
        return -1;

    strncpy(password, token, PASS_SIZE);
    password[PASS_SIZE] = '\0';

    return 0;
}

int parse_opa(char *msg, char *uid, char *pass, char *name, char *start_value,
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
    return 0;
}

int parse_cls(char *msg, char *uid, char *password, char *aid) {
    int last_pos = strlen(msg) - 1;
    char *token;

    // check \n
    if (msg[last_pos] != '\n') {
        return -1;
    }
    msg[last_pos] = '\0'; // replace \n with \0

    memset(uid, 0, UID_SIZE + 1);
    memset(password, 0, PASS_SIZE + 1);
    memset(aid, 0, AID_SIZE + 1);

    token = strtok(msg, " ");

    // UID
    token = strtok(NULL, " ");
    if (!valid_uid(token))
        return -1;

    strncpy(uid, token, UID_SIZE);
    uid[UID_SIZE] = '\0';

    // password
    token = strtok(NULL, " ");
    if (!valid_password(token))
        return -1;

    strncpy(password, token, PASS_SIZE);
    password[PASS_SIZE] = '\0';

    // AID
    token = strtok(NULL, " ");
    if (!valid_aid(token))
        return -1;

    strncpy(aid, token, AID_SIZE);
    password[AID_SIZE] = '\0';

    return 0;
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

char* bad_syntax_res() {
    char *msg = malloc((CODE_SIZE + 2) * sizeof(char));
    memset(msg, 0, (CODE_SIZE + 2) * sizeof(char));
    strcpy(msg, STATUS_ERR);
    return msg;
}

char* server_error_res() {
    char *msg = malloc((CODE_SIZE + 2) * sizeof(char));
    memset(msg, 0, (CODE_SIZE + 2) * sizeof(char));
    strcpy(msg, STATUS_ERR);
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

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdbool.h>

#define UID_SIZE 6
#define PASS_SIZE 8

#define CMD_SIZE 3

#define LOGIN_CMD "LIN "
#define LOGIN_MSG_SIZE (CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE + 1)
#define LOGIN_RES_OK "RLI OK\n"
#define LOGIN_RES_NOK "RLI NOK\n"
#define LOGIN_RES_REG "RLI REG\n"

#define LOGOUT_CMD "LOU "
#define LOGOUT_MSG_SIZE (CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE + 1)
#define LOGOUT_RES_OK "RLO OK\n"
#define LOGOUT_RES_NOK "RLO NOK\n"
#define LOGOUT_RES_UNR "RLO UNR\n"

// LIN UID(6-numeric) password(8-alphanumeric)\n
char *login_req(char *uid, char *password);
bool login_res(char *response, bool print);

char *logout_req(char *uid, char *password);
bool logout_res(char *response, bool print);

#endif
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdbool.h>

#define UID_SIZE 6
#define PASS_SIZE 8

#define RECV_SIZE_DEFAULT 128
#define RECV_SIZE_LST (6 * 1024)

#define CMD_SIZE 3

#define LIN_REQ "LIN "
#define LIN_SIZE (CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE + 1)
#define RLI_OK "RLI OK\n"
#define RLI_NOK "RLI NOK\n"
#define RLI_REG "RLI REG\n"

#define LOU_REQ "LOU "
#define LOU_SIZE (CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE + 1)
#define RLO_OK "RLO OK\n"
#define RLO_NOK "RLO NOK\n"
#define RLO_UNR "RLO UNR\n"

#define UNR_REQ "UNR "
#define UNR_SIZE (CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE + 1)
#define RUR_OK "RUR OK\n"
#define RUR_NOK "RUR NOK\n"
#define RUR_UNR "RUR UNR\n"

#define LST_REQ "LST\n"
#define LST_SIZE (CMD_SIZE + 1)
#define RLS_OK "RLS OK "
#define RLS_OK_SIZE 7
#define RLS_NOK "RLS NOK\n"

// LIN UID(6-numeric) password(8-alphanumeric)\n
char *login_req(char *uid, char *password);
bool login_res(char *response, bool print);

char *logout_req(char *uid, char *password);
bool logout_res(char *response, bool print);

char *unregister_req(char *uid, char *password);
bool unregister_res(char *response, bool print);

char *list_req();
void list_res(char *response, bool print);

#endif
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdbool.h>

#define UID_SIZE 6
#define PASS_SIZE 8
#define AID_SIZE 3

#define RECV_SIZE_DEFAULT 128
#define RECV_SIZE_LST (6 * 1024)

#define CMD_SIZE 3

#define LIN_REQ "LIN "
#define LIN_SIZE (CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE + 1)

#define LOU_REQ "LOU "
#define LOU_SIZE (CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE + 1)

#define UNR_REQ "UNR "
#define UNR_SIZE (CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE + 1)

#define LST_REQ "LST\n"
#define LST_SIZE (CMD_SIZE + 1)
#define RLS_OK "RLS OK "
#define RLS_OK_SIZE 7

#define CLS_REQ "CLS "
#define CLS_SIZE (CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE + 1 + AID_SIZE)

#define BID_REQ "BID "
#define BID_SIZE (CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE + 1 + AID_SIZE)

#define STATUS_OK "OK\n"
#define STATUS_NOK "NOK\n"
#define STATUS_REG "REG\n"
#define STATUS_UNR "UNR\n"
#define STATUS_NLG "NLG\n"
#define STATUS_EAU "EAU\n"
#define STATUS_EOW "EOW\n"
#define STATUS_END "END\n"
#define STATUS_ACC "ACC\n"
#define STATUS_REF "REF\n"
#define STATUS_ILG "ILG\n"

// LIN UID(6-numeric) password(8-alphanumeric)\n
char *login_req(char *uid, char *password);
bool login_res(char *response, bool print);

char *logout_req(char *uid, char *password);
bool logout_res(char *response, bool print);

char *unregister_req(char *uid, char *password);
bool unregister_res(char *response, bool print);

char *list_req();
void list_res(char *response, bool print);

char* close_req(char* uid, char* password, char* aid);
bool close_res(char* response, bool print);

char* bid_req(char* uid, char* password, char* aid, char* value);
bool bid_res(char *response, bool print);

#endif
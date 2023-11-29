#ifndef PROTOCOL_H
#define PROTOCOL_H

#define UID_SIZE 6
#define PASS_SIZE 8

#define CMD_SIZE 3

#define LOGIN_CMD "LIN "
#define LOGIN_MSG_SIZE (CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE + 1)

#define LOGOUT_CMD "LOU "
#define LOGOUT_MSG_SIZE (CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE + 1)

// LIN UID(6-numeric) password(8-alphanumeric)\n
char *login_req(char *uid, char *password);

char *logout_req(char *uid, char *password);

#endif
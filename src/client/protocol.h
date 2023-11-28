#ifndef PROTOCOL_H
#define PROTOCOL_H

#define UID_SIZE 6
#define PASS_SIZE 8
#define LOGIN_MSG_SIZE 19

// LIN UID(6-numeric) password(8-alphanumeric)
char *login_msg(char *uid, char *password);

#endif
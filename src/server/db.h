#ifndef DB_H
#define DB_H

#include <stdbool.h>

#define DEFAULT_PERMS 0700

#define DB_DIR "DB/"
#define AUCTIONS_DIR DB_DIR "AUCTIONS/"
#define USERS_DIR DB_DIR "USERS/"
#define PASS_SUFIX "_pass.txt"
#define LOGIN_SUFIX "_login.txt"
#define HOSTED_DIR "HOSTED/"
#define BIDDED_DIR "BIDDED/"

int create_defaults();

char *user_dir(char *uid);
char *user_bidded_dir(char *uid);
char *user_hosted_dir(char *uid);
bool user_registered(char *uid);
int user_register(char *uid, char *password);
int user_unregister(char *uid);
bool user_loggedin(char *uid);
int user_login(char *uid);
int user_logout(char *uid);
char *user_password(char *uid);
bool user_ok_password(char *uid, char *password);

char *auction_dir(char *aid);

#endif

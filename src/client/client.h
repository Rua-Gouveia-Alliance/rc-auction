#ifndef CLIENT_H
#define CLIENT_H

#include <stdbool.h>

typedef struct User {
    char uid[7];
    char password[9];
} User;

void login(char *uid, char *password);

void logout();

void unregister();

void open_auc(char *name, char *asset_fname, char *start_value,
              char *timeactive);

void close_auc(char *aid);

void myauctions();

void mybids();

void list();

void show_asset(char *aid);

void bid(char *aid, char *value);

void show_record(char *aid);

bool prompt_command(char *command);

int prompt();

#endif
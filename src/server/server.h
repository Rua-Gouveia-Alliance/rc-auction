#include "protocol.h"

#ifndef SERVER_H
#define SERVER_H

#define TIMEOUT_SECS 10
#define DEFAULT_LIMIT 50
#define NO_LIMIT 999

char *login(char *uid, char *password);

char *logout(char *uid, char *password);

char *unregister(char *uid, char *password);

char *open_auc(char *uid, char *password, char *name, char *start_value,
               char *timeactive, char *fname);

char *close_auc(char *uid, char *password, char *aid);

char *user_auctions(char *uid);

char *user_bids(char *uid);

char *list();

char *show_asset(char *aid);

char *bid(char *uid, char *password, char *aid, char *value);

char *show_record(char *aid);

bool treat_request(char *request, int socket);

void handle_sockets();

#endif

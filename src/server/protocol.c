#include "protocol.h"
#include "../util.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


char *default_res(char* code, char* status) {
    // setup buffer
    char *msg = malloc((RES_SIZE) * sizeof(char));
    memset(msg, 0, (RES_SIZE) * sizeof(char));

    // copy data
    strcpy(msg, code);
    strcpy(&msg[CODE_SIZE + 1], status);
    msg[CODE_SIZE + 1 + STATUS_SIZE] = '\n';

    return msg;
}

char *open_auction_req(char* res, char* status, char* list);

char *auction_list_res(char* res, char* status, char* list);

char *bid_list_res(char* res, char* status, char* list);

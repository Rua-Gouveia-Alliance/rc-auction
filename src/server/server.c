#include "server.h"
#include <arpa/inet.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void login(char *uid, char *password) {
    return;
}

void logout() {
    return;
}

void unregister() {
    return;
}

bool exit_prompt() {
    return true;
}

void open_auc(char *name, char *asset_fname, char *start_value, char *timeactive) {
    return;
}

void close_auc(char *aid) {
    return;
}

void myauctions() {
    return;
}

void mybids() {
    return;
}

void list() {
    return;
}

void show_asset(char *aid) {
    return;
}

void bid(char *aid, char *value) {
    return;
}

void show_record(char *aid) {
    return;
}

void treat_request(char* request, bool verbose) {
    if (verbose)
        printf("Received request: %s", request);

    return;
}

int main(int argc, char *argv[]) {
    char asport[5] = "58012"; // 58000 + group number
    bool verbose = false;

    int opt;
    while ((opt = getopt(argc, argv, "p:v")) != -1) {
        switch (opt) {
        case 'p':
            strncpy(asport, optarg, 5);
            break;
        case 'v':
            verbose = true;
            break;
        case '?':
            printf("Unknown argument: %c\n", opt);
            break;
        }
    }
}

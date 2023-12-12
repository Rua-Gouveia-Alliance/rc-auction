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
#include <fcntl.h>

char* get_filename(char* dir, char* id, char* ext, int size) {
    char* filename = (char*)malloc(size*sizeof(char));

    sprintf(filename, "%s%s%s", dir, id, ext);
    
    return filename;
}

// TODO: Add server replies to client
void login(char *uid, char *password) {
    int pass_fd, logged_in_fd;
    char *response, *pass_filename, *logged_in_filename, *recorded_pass;
    
    pass_filename = get_filename(USERS_DIR, uid, PASS_FILENAME_EXT, PASS_FILENAME_SIZE);
    logged_in_filename = get_filename(USERS_DIR, uid, LOGGED_IN_FILENAME_EXT,LOGGED_IN_FILENAME_SIZE);

    pass_fd = open(pass_filename, O_RDONLY);
    logged_in_fd = open(logged_in_filename, O_RDONLY);
    
    if (pass_fd == -1) {
    // User is not registered yet
        pass_fd = open(pass_filename, O_CREAT | O_WRONLY);
        write(pass_fd, password, strlen(password));
        logged_in_fd = open(logged_in_filename, O_CREAT);
        response = default_res(LIN_RES, STATUS_UNR);

    } else if (logged_in_fd == -1) {
    // User registered but not logged in
        recorded_pass = (char*)malloc(PASS_SIZE + 1);
        read(pass_fd, recorded_pass, PASS_SIZE);
        recorded_pass[PASS_SIZE] = '\0';
        
        if (strcmp(recorded_pass, password) == 0) {
        // Correct password, logging in        
            logged_in_fd = open(logged_in_filename, O_CREAT);
            response = default_res(LIN_RES, STATUS_OK);
        } else {
        // Failed password

        }
        free(recorded_pass);

    } else {
    // User registered and already logged in

    }
    
    free(response);
    free(pass_filename);
    free(logged_in_filename);

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

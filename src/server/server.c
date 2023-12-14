#include "server.h"
#include "../util.h"
#include "connection.h"
#include "protocol.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

static char asport[] = "58012"; // 58000 + group number
static bool verbose = true;     // TODO: default is false

int check_password(int pass_fd, char *password) {
    ssize_t n;
    char *recorded_pass;
    bool right_password;

    recorded_pass = (char *)malloc(PASS_SIZE + 1);
    if (recorded_pass == NULL) {
        printf("No more memory, shutting down.\n");
        exit(EXIT_FAILURE);
    }

    n = read(pass_fd, recorded_pass, PASS_SIZE);
    if (n == -1) {
        free(recorded_pass);
        return -1;
    }

    recorded_pass[PASS_SIZE] = '\0';
    right_password = strcmp(recorded_pass, password) == 0;
    free(recorded_pass);

    return right_password;
}

char *login(char *uid, char *password) {
    ssize_t n;
    int pass_fd, logged_in_fd;
    char *response, *pass_filename, *logged_in_filename;

    pass_filename =
        get_filename(USERS_DIR, uid, PASS_FILENAME_EXT, PASS_FILENAME_SIZE);
    logged_in_filename = get_filename(USERS_DIR, uid, LOGGED_IN_FILENAME_EXT,
                                      LOGGED_IN_FILENAME_SIZE);

    pass_fd = open(pass_filename, O_RDONLY);
    logged_in_fd = open(logged_in_filename, O_RDONLY);

    if (pass_fd == -1) {
        // User is not registered yet
        pass_fd = open(pass_filename, O_CREAT | O_WRONLY, 0666);
        if (pass_fd == -1) {
            free(pass_filename);
            free(logged_in_filename);
            return default_res(LIN_RES, STATUS_ERR);
        }

        n = write(pass_fd, password, strlen(password));
        if (n == -1) {
            free(pass_filename);
            free(logged_in_filename);
            return default_res(LIN_RES, STATUS_ERR);
        }

        logged_in_fd = open(logged_in_filename, O_CREAT, 0666);
        if (logged_in_fd == -1) {
            free(pass_filename);
            free(logged_in_filename);
            return default_res(LIN_RES, STATUS_ERR);
        }

        free(pass_filename);
        free(logged_in_filename);
        return default_res(LIN_RES, STATUS_REG);
    } else if (logged_in_fd == -1) {
        // User registered but not logged in
        int right_password = check_password(pass_fd, password);
        if (right_password == -1) {
            free(pass_filename);
            free(logged_in_filename);
            return default_res(LIN_RES, STATUS_ERR);
        } else if (right_password) {
            // Correct password, logging in
            logged_in_fd = open(logged_in_filename, O_CREAT, 0666);
            if (logged_in_fd == -1) {
                free(pass_filename);
                free(logged_in_filename);
                return default_res(LIN_RES, STATUS_ERR);
            }

            free(pass_filename);
            free(logged_in_filename);
            return default_res(LIN_RES, STATUS_OK);
        }

        // Failed password
        free(pass_filename);
        free(logged_in_filename);
        return default_res(LIN_RES, STATUS_NOK);
    }

    // User registered and already logged in
    free(pass_filename);
    free(logged_in_filename);
    return default_res(LIN_RES, STATUS_OK);
}

char *logout(char *uid, char *password) {
    int pass_fd, logged_in_fd, n;
    char *response, *pass_filename, *logged_in_filename;

    pass_filename =
        get_filename(USERS_DIR, uid, PASS_FILENAME_EXT, PASS_FILENAME_SIZE);
    logged_in_filename = get_filename(USERS_DIR, uid, LOGGED_IN_FILENAME_EXT,
                                      LOGGED_IN_FILENAME_SIZE);

    pass_fd = open(pass_filename, O_RDONLY);
    logged_in_fd = open(logged_in_filename, O_RDONLY);

    if (pass_fd == -1) {
        // User is not registered yet
        free(pass_filename);
        free(logged_in_filename);
        return default_res(LOU_RES, STATUS_UNR);
    } else if (logged_in_fd == -1) {
        // User registered but not logged in
        free(pass_filename);
        free(logged_in_filename);
        return default_res(LOU_RES, STATUS_NOK);
    }

    // User registered and logged in
    int right_password = check_password(pass_fd, password);
    if (right_password == -1) {
        free(pass_filename);
        free(logged_in_filename);
        return default_res(LOU_RES, STATUS_ERR);
    } else if (right_password) {
        // Correct password, logging out
        n = remove(logged_in_filename);
        if (n == -1) {
            free(pass_filename);
            free(logged_in_filename);
            return default_res(LOU_RES, STATUS_ERR);
        }

        free(pass_filename);
        free(logged_in_filename);
        return default_res(LOU_RES, STATUS_OK);
    }

    // Wrong password
    free(pass_filename);
    free(logged_in_filename);
    return default_res(LOU_RES, STATUS_ERR);
}

char *unregister(char *uid, char *password) {
    int pass_fd, logged_in_fd, n;
    char *response, *pass_filename, *logged_in_filename;

    pass_filename =
        get_filename(USERS_DIR, uid, PASS_FILENAME_EXT, PASS_FILENAME_SIZE);
    logged_in_filename = get_filename(USERS_DIR, uid, LOGGED_IN_FILENAME_EXT,
                                      LOGGED_IN_FILENAME_SIZE);

    pass_fd = open(pass_filename, O_RDONLY);
    logged_in_fd = open(logged_in_filename, O_RDONLY);

    if (pass_fd == -1) {
        // User is not registered yet
        free(pass_filename);
        free(logged_in_filename);
        return default_res(UNR_RES, STATUS_UNR);
    } else if (logged_in_fd == -1) {
        // User registered but not logged in
        free(pass_filename);
        free(logged_in_filename);
        return default_res(UNR_RES, STATUS_NOK);
    }

    // User registered and logged in
    int right_password = check_password(pass_fd, password);
    if (right_password == -1) {
        free(pass_filename);
        free(logged_in_filename);
        return default_res(UNR_RES, STATUS_ERR);
    } else if (right_password) {
        // Correct password, unregistering
        n = remove(logged_in_filename);
        if (n == -1) {
            free(pass_filename);
            free(logged_in_filename);
            return default_res(UNR_RES, STATUS_ERR);
        }

        n = remove(pass_filename);
        if (n == -1) {
            free(pass_filename);
            free(logged_in_filename);
            return default_res(UNR_RES, STATUS_ERR);
        }

        free(pass_filename);
        free(logged_in_filename);
        return default_res(UNR_RES, STATUS_OK);
    }

    // Failed password
    free(pass_filename);
    free(logged_in_filename);
    return default_res(UNR_RES, STATUS_ERR);
}

char *open_auc(char *name, char *asset_fname, char *start_value,
               char *timeactive) {
    return NULL;
}

char *close_auc(char* uid, char* password, char *aid) {
    int pass_fd, logged_in_fd, n;
    char *response, *pass_filename, *logged_in_filename;

    pass_filename =
        get_filename(USERS_DIR, uid, PASS_FILENAME_EXT, PASS_FILENAME_SIZE);
    logged_in_filename = get_filename(USERS_DIR, uid, LOGGED_IN_FILENAME_EXT,
                                      LOGGED_IN_FILENAME_SIZE);

    pass_fd = open(pass_filename, O_RDONLY);
    logged_in_fd = open(logged_in_filename, O_RDONLY);

    free(pass_filename);
    free(logged_in_filename);

    if (pass_fd == -1) {
        // User is not registered yet
        return default_res(CLS_RES, STATUS_NOK);
    } else if (logged_in_fd == -1) {
        // User registered but not logged in
        return default_res(CLS_RES, STATUS_NLG);
    }

    // User registered and logged in
    int right_password = check_password(pass_fd, password);
    if (right_password == -1) {
        return default_res(CLS_RES, STATUS_NOK);
    } else if (right_password) {
        // Correct password, going to remove auction
        ssize_t n;
        struct stat sb;
        int start_fd, end_fd;
        char* dirname, *start_filename, *end_filename, *right_uid;

        dirname = get_filename(AUCTIONS_DIR, aid, "", AUCTION_DIRNAME_SIZE);
        if (!(stat(dirname, &sb) == 0 && S_ISDIR(sb.st_mode))) {
            // Auction does not exist
            free(dirname);
            return default_res(CLS_RES, STATUS_EAU);
        }

        start_filename = get_filename(dirname, "", START_FILENAME, START_FILENAME_SIZE);
        end_filename = get_filename(dirname, "", END_FILENAME, END_FILENAME_SIZE);
        end_fd = open(end_filename, O_RDONLY);
        start_fd = open(start_filename, O_RDONLY);

        free(start_filename);
        free(dirname);

        if (start_fd == - 1) {
            // Failed to open START.txt
            free(end_filename);
            return default_res(CLS_RES, STATUS_ERR);
        }

        right_uid = (char*)malloc(sizeof(char)*(UID_SIZE + 1));
        if (right_uid == NULL) {
            free(end_filename);
            printf("No more memory, shutting down.\n");
            exit(EXIT_FAILURE);
        }

        n = read(start_fd, right_uid, UID_SIZE);
        if (n == -1) {
            free(end_filename);
            return default_res(CLS_RES, STATUS_ERR);
        }

        right_uid[UID_SIZE] = '\0';
        if (strcmp(uid, right_uid) != 0) {
            // Auction not owned by user
            free(end_filename);
            return default_res(CLS_RES, STATUS_EOW);
        }

        if (end_fd != - 1) {
            // Auction already closed
            free(end_filename);
            return default_res(CLS_RES, STATUS_END);
        }

        // Closing auction
        end_fd = open(end_filename, O_CREAT);
        free(end_filename);
        return default_res(CLS_RES, STATUS_OK);
    }

    // Failed password
    return default_res(CLS_RES, STATUS_NOK);
}

char *myauctions() { return NULL; }

char *mybids() { return NULL; }

char *list() { return NULL; }

char *show_asset(char *aid) { return NULL; }

char *bid(char *aid, char *value) { return NULL; }

char *show_record(char *aid) { return NULL; }

void treat_request(char *request, int socket) {
    int id;
    char *response;

    if (verbose)
        printf("Received request: %s", request);

    id = interpret_req(request);
    switch (id) {
    case LIN: {
        char uid[UID_SIZE + 1];
        char pass[PASS_SIZE + 1];

        parse_lin(request, uid, pass);
        response = login(uid, pass);
        if (verbose)
            printf("%s\n", response);

        send_udp(socket, response, NULL);
        break;
    }
    case LOU: {
        char uid[UID_SIZE + 1];
        char pass[PASS_SIZE + 1];

        parse_lou(request, uid, pass);
        response = logout(uid, pass);
        if (verbose)
            printf("%s\n", response);

        send_udp(socket, response, NULL);
        break;
    }
    case UNR: {
        char uid[UID_SIZE + 1];
        char pass[PASS_SIZE + 1];

        parse_unr(request, uid, pass);
        response = unregister(uid, pass);
        if (verbose)
            printf("%s\n", response);

        send_udp(socket, response, NULL);
        break;
    }
    case LMA:
        break;
    case LMB:
        break;
    case LST:
        break;
    case SRC:
        break;
    case CLS:
        break;
    case BID:
        break;
    case SAS:
        break;
    case OPA:
        break;
    default:
        break;
    }

    free(response);
    free(request);
    return;
}

void handle_sockets() {
    int tcp_main, udp_sock, out_select, temp_sock;
    bool done, ok;
    fd_set current_sockets, ready_sockets;
    struct timeval timeout;
    char *buffer;

    tcp_main = setup_tcp(asport);
    udp_sock = setup_udp(asport);

    FD_ZERO(&current_sockets);
    FD_SET(tcp_main, &current_sockets);
    FD_SET(udp_sock, &current_sockets);

    for (;;) {
        ready_sockets = current_sockets;
        memset((void *)&timeout, 0, sizeof(timeout));
        timeout.tv_sec = 10;

        out_select = select(FD_SETSIZE, &ready_sockets, NULL, NULL,
                            (struct timeval *)&timeout);

        switch (out_select) {
        case 0:
            // TODO: timeout
            break;
        case -1:
            printf("Socket select failed\n");
            exit(EXIT_FAILURE);
            break;
        default:
            for (int i = 0; i < FD_SETSIZE; i++) {
                if (FD_ISSET(i, &ready_sockets)) {
                    if (i == tcp_main) {
                        // new tcp connecton
                        temp_sock = accept_new_tcp(tcp_main);
                        if (temp_sock != -1)
                            FD_SET(temp_sock, &current_sockets);
                    } else if (i == udp_sock) {
                        buffer = receive_udp(udp_sock);
                        if (buffer != NULL)
                            treat_request(buffer, i);
                    } else {
                        buffer = receive_tcp(i);
                        if (buffer != NULL) {
                            FD_CLR(i, &current_sockets);
                            treat_request(buffer, i);
                        }
                    }
                }
            }
            break;
        }
    }
    return;
}

int main(int argc, char *argv[]) {

    int opt;
    while ((opt = getopt(argc, argv, "p:v")) != -1) {
        switch (opt) {
        case 'p':
            // TODO: check for valid port num
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

    handle_sockets();
}

#include "server.h"
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
#include <unistd.h>

static char asport[] = "58012"; // 58000 + group number
static bool verbose = true;     // TODO: default is false

char *get_filename(char *dir, char *id, char *ext, int size) {
    char *filename = (char *)malloc((size + 1) * sizeof(char));
    memset(filename, 0, (size + 1) * sizeof(char));
    if (filename == NULL) {
        printf("No more memory, shutting down.\n");
        exit(EXIT_FAILURE);
    }

    sprintf(filename, "%s%s%s", dir, id, ext);
    return filename;
}

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
            printf("ooiiii1\n");
            free(pass_filename);
            free(logged_in_filename);
            return default_res(LIN_RES, STATUS_ERR);
        }

        n = write(pass_fd, password, strlen(password));
        if (n == -1) {
            printf("ooiiii2\n");
            free(pass_filename);
            free(logged_in_filename);
            return default_res(LIN_RES, STATUS_ERR);
        }

        logged_in_fd = open(logged_in_filename, O_CREAT, 0666);
        if (logged_in_fd == -1) {
            printf("ooiiii3\n");
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
            printf("ooiiii4\n");
            free(pass_filename);
            free(logged_in_filename);
            return default_res(LIN_RES, STATUS_ERR);
        } else if (right_password) {
            // Correct password, logging in
            logged_in_fd = open(logged_in_filename, O_CREAT, 0666);
            if (logged_in_fd == -1) {
                printf("ooiiii5\n");
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
        response = default_res(LOU_RES, STATUS_UNR);

    } else if (logged_in_fd == -1) {
        // User registered but not logged in
        response = default_res(LOU_RES, STATUS_NOK);

    } else {
        // User registered and logged in
        int right_password = check_password(pass_fd, password);

        if (right_password == -1) {
            response = default_res(LOU_RES, STATUS_ERR);
        } else if (right_password) {
            // Correct password, logging out
            response = default_res(LOU_RES, STATUS_OK);
            n = remove(logged_in_filename);
            if (n == -1)
                response = default_res(LOU_RES, STATUS_ERR);
        } else {
            // Failed password
            response = default_res(LOU_RES, STATUS_ERR);
        }

        response = default_res(LOU_RES, STATUS_OK);
        n = remove(logged_in_filename);
        if (n == -1)
            response = default_res(LOU_RES, STATUS_ERR);
    }

    free(pass_filename);
    free(logged_in_filename);
    return response;
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
        response = default_res(UNR_RES, STATUS_UNR);

    } else if (logged_in_fd == -1) {
        // User registered but not logged in
        response = default_res(UNR_RES, STATUS_NOK);

    } else {
        // User registered and logged in
        int right_password = check_password(pass_fd, password);

        if (right_password == -1) {
            response = default_res(UNR_RES, STATUS_ERR);
        } else if (right_password) {
            // Correct password, unregistering
            response = default_res(UNR_RES, STATUS_OK);

            n = remove(logged_in_filename);
            if (n == -1) {
                response = default_res(UNR_RES, STATUS_ERR);
            }

            n = remove(pass_filename);
            if (n == -1) {
                response = default_res(UNR_RES, STATUS_ERR);
            }

        } else {
            // Failed password
            response = default_res(UNR_RES, STATUS_ERR);
        }

        response = default_res(UNR_RES, STATUS_OK);
        n = remove(logged_in_filename);
        if (n == -1) {
            response = default_res(UNR_RES, STATUS_ERR);
        }
    }

    free(pass_filename);
    free(logged_in_filename);
    return response;
}

char *open_auc(char *name, char *asset_fname, char *start_value,
               char *timeactive) {
    return NULL;
}

char *close_auc(char *aid) { return NULL; }

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
        printf("%s\n", response);
        send_udp(socket, response, NULL);
        break;
    }
    case LOU:
        break;
    case UNR:
        break;
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

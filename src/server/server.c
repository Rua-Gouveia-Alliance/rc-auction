// #define _XOPEN_SOURCE
// E preciso acrescentar a linha acima para o strptime funcionar, mas fode as cenas das sockets todas
#include "server.h"
#include "../util.h"
#include "connection.h"
#include "db.h"
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
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

static char asport[] = "58012"; // 58000 + group number
static bool verbose = true;     // TODO: default is false

<<<<<<< HEAD
int create_end_file(char* fname, time_t end_datetime, int end_duration) {
    ssize_t n;
    int end_fd;
    char datetime[DATE_TIME_SIZE+2], duration[DUR_SIZE + 1];
    
    if (datetime == NULL || duration == NULL) {
        printf("No more memory, shutting down.\n");
        exit(EXIT_FAILURE);
    }
    
    end_fd = open(fname, O_CREAT);
    if (end_fd == -1) {
        return -1;
    }
    
    n = strftime(datetime, DATE_TIME_SIZE + 1, "%Y-%m-%d %H:%M:%S ", localtime(&end_datetime));
    if (n == 0) {
        remove(fname);
        return -1;
    }
    
    n = write(end_fd, datetime, DATE_TIME_SIZE+1);
    if (n == -1) {
        remove(fname);
        return -1;
    }

    n = sprintf(duration, "%d", end_duration);
    if (n < 0 ) {
        remove(fname);
        return -1;
    }

    n = write(end_fd, duration, DUR_SIZE);
    if (n == -1) {
        remove(fname);
        return -1;
    }

    return end_fd;
}

int parse_start_file(int fd, char* uid, char* name,char* asset_fname,char* start_value,char* timeactive,char* start_datetime) {
    ssize_t n;
    char *token, *contents;
    
    contents = (char*)malloc(sizeof(char)*START_CONTENTS_SIZE + 1);
    n = read(fd, contents, START_CONTENTS_SIZE + 1);
    if (n == -1)  {
        return - 1;
    }

    // UID
    token = strtok(contents, " ");
    if (token == NULL)  {
        return - 1;
    }
    if (uid != NULL) {
        strcpy(uid, token);
    }

    // name
    token = strtok(contents, " ");
    if (token == NULL)  {
        return - 1;
    }
    if (name != NULL) {
        strcpy(name, token);
    }

    // asset_fname
    token = strtok(contents, " ");
    if (token == NULL)  {
        return - 1;
    }
    if (asset_fname != NULL) {
        strcpy(asset_fname, token);
    }

    // start_value
    token = strtok(contents, " ");
    if (token == NULL)  {
        return - 1;
    }
    if (start_value != NULL) {
        strcpy(start_value, token);
    }

    // timeactive
    token = strtok(contents, " ");
    if (token == NULL)  {
        return - 1;
    }
    if (timeactive != NULL) {
        strcpy(timeactive, token);
    }

    // start_datetime
    token = strtok(contents, " ");
    if (token == NULL)  {
        return - 1;
    }
    if (start_datetime != NULL) {
        strcpy(start_datetime, token);
        start_datetime[DATE_SIZE] = ' ';

        token = strtok(contents, " ");
        if (token == NULL)  {
            return - 1;
        }

        strcpy(start_datetime + DATE_SIZE + 1, token);
    }
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

=======
>>>>>>> 8ef66859a586002d4ff6117bb4019e49fb70c8da
char *login(char *uid, char *password) {
    char *response;

    if (!user_registered(uid)) {
        if (user_register(uid, password) == -1)
            return default_res(LIN_RES, STATUS_ERR);

        if (user_login(uid) == -1)
            return default_res(LIN_RES, STATUS_ERR);

        return default_res(LIN_RES, STATUS_REG);
    } else if (!user_loggedin(uid)) {
        if (user_ok_password(uid, password)) {
            if (user_login(uid) == -1)
                return default_res(LIN_RES, STATUS_ERR);

            return default_res(LIN_RES, STATUS_OK);
        }

        // Failed password
        return default_res(LIN_RES, STATUS_NOK);
    }

    // User registered and already logged in
    return default_res(LIN_RES, STATUS_OK);
}

char *logout(char *uid, char *password) {
    char *response;

    if (!user_registered(uid))
        return default_res(LOU_RES, STATUS_UNR);
    else if (!user_loggedin(uid))
        return default_res(LOU_RES, STATUS_NOK);

    // User registered and logged in
    if (user_ok_password(uid, password)) {
        if (user_logout(uid) == -1)
            return default_res(LOU_RES, STATUS_ERR);

        return default_res(LOU_RES, STATUS_OK);
    }

    // Wrong password
    return default_res(LOU_RES, STATUS_ERR);
}

char *unregister(char *uid, char *password) {
    char *response;

    if (!user_registered(uid))
        return default_res(UNR_RES, STATUS_UNR);
    else if (!user_ok_password(uid, password))
        return default_res(UNR_RES, STATUS_ERR);
    else if (!user_loggedin(uid))
        return default_res(UNR_RES, STATUS_NOK);

    if (user_logout(uid) == -1)
        return default_res(UNR_RES, STATUS_ERR);

    if (user_unregister(uid) == -1)
        return default_res(UNR_RES, STATUS_ERR);

    return default_res(UNR_RES, STATUS_OK);
}

char *open_auc(char *name, char *asset_fname, char *start_value,
               char *timeactive) {
    return NULL;
}

char *close_auc(char *uid, char *password, char *aid) {
    char *response;
    ssize_t n;
    struct stat sb;
    int start_fd, end_fd;
    char *dirname, *start_filename, *end_filename, *right_uid;

    if (!user_registered(uid))
        return default_res(CLS_RES, STATUS_NOK);
    else if (!user_loggedin(uid))
        return default_res(CLS_RES, STATUS_NLG);
    else if (!user_ok_password(uid, password))
        return default_res(CLS_RES, STATUS_NOK);
    else if (!auction_exists(aid))
        return default_res(CLS_RES, STATUS_EAU);
    else if (!auction_is_owner(aid, uid))
        return default_res(CLS_RES, STATUS_EOW);
    else if (auction_closed(aid))
        return default_res(CLS_RES, STATUS_END);

<<<<<<< HEAD
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

        // Maybe the auction is already finished but we haven't created the file yet
        int duration;
        double time_diff;
        char start_time[DATE_TIME_SIZE + 1], duration_str[DUR_SIZE + 1];
        struct tm tm_end_time;
        time_t current_time, end_time;

        lseek(start_fd, 0, SEEK_SET);
        n = parse_start_file(start_fd, NULL, NULL, NULL, NULL, duration_str, start_time);
        if (n == - 1) {
            free(end_filename);
            return default_res(CLS_RES, STATUS_ERR);
        }

        strptime(start_time, "%Y-%m-%d %H:%M:%S", &tm_end_time);

        duration = atoi(duration_str);
        if (duration == 0) {
            free(end_filename);
            return default_res(CLS_RES, STATUS_ERR);
        }

        tm_end_time.tm_sec += duration;
        end_time = mktime(&tm_end_time);
        if (end_time == -1) {
            free(end_filename);
            return default_res(CLS_RES, STATUS_ERR);
        }

        current_time = time(NULL);
        time_diff = difftime(end_time, current_time);
        if (time_diff < 0) {
            end_fd = create_end_file(end_filename, end_time, duration);
            free(end_filename);

            if (end_fd == -1) {
                default_res(CLS_RES, STATUS_ERR);
            }
            return default_res(CLS_RES, STATUS_END);
        }

        // Closing auction
        end_fd = create_end_file(end_filename, current_time, duration - ((int) time_diff));
        if (end_fd == -1) {
            default_res(CLS_RES, STATUS_ERR);
        }
        free(end_filename);
        return default_res(CLS_RES, STATUS_OK);
    }

    // Failed password
    return default_res(CLS_RES, STATUS_NOK);
=======
    if (auction_close(aid) == -1)
        return default_res(CLS_RES, STATUS_ERR);
    return default_res(CLS_RES, STATUS_OK);
>>>>>>> 8ef66859a586002d4ff6117bb4019e49fb70c8da
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
    create_defaults();

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

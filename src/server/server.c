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
#include <time.h>
#include <unistd.h>
#include <dirent.h>

static char asport[] = "58012"; // 58000 + group number
static bool verbose = true;     // TODO: default is false

char *login(char *uid, char *password) {
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

char *open_auc(char *uid, char *password, char *name, char *start_value,
               char *timeactive, char *fname) {
    char *aid, *response;
    if (!user_registered(uid))
        return default_res(OPA_RES, STATUS_NOK);
    else if (!user_loggedin(uid))
        return default_res(OPA_RES, STATUS_NLG);
    else if (!user_ok_password(uid, password))
        return default_res(OPA_RES, STATUS_NOK);

    if ((aid = auction_open(uid, name, start_value, timeactive, fname)) == NULL)
        return default_res(OPA_RES, STATUS_ERR);
    response = opa_ok_res(aid);
    free(aid);
    return response;
}

char *close_auc(char *uid, char *password, char *aid) {
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

    if (auction_close_now(aid) == -1)
        return default_res(CLS_RES, STATUS_ERR);
    return default_res(CLS_RES, STATUS_OK);
}

char *user_auctions(char* uid) { 
    if (!user_registered(uid))
        return default_res(LMA_RES, STATUS_NLG);
    else if (!user_loggedin(uid))
        return default_res(LMA_RES, STATUS_NLG);

    char* dir = user_hosted_dir(uid);
    if (count_entries(dir, DT_REG) == 0) {
        free(dir);
        return default_res(LMA_RES, STATUS_NOK);
    }

    char* auctions = list_auctions(dir, DEFAULT_LIMIT);
    char* response = list_res(LMA_RES, auctions);

    free(dir);
    free(auctions);

    return response;
}

char *user_bids(char* uid) {
    if (!user_registered(uid))
        return default_res(LMB_RES, STATUS_NLG);
    else if (!user_loggedin(uid))
        return default_res(LMB_RES, STATUS_NLG);

    char* dir = user_bidded_dir(uid);
    if (count_entries(dir, DT_REG) == 0) {
        free(dir);
        return default_res(LMB_RES, STATUS_NOK);
    }

    char* auctions = list_auctions(dir, DEFAULT_LIMIT);
    char* response = list_res(LMB_RES, auctions);

    free(dir);
    free(auctions);

    return response;
}

char *list() {
    char* dir = AUCTIONS_DIR;
    if (count_entries(dir, DT_REG) == 0) {
        return default_res(LST_RES, STATUS_NOK);
    }

    char* auctions = list_auctions(dir, NO_LIMIT);
    char* response = list_res(LST_RES, auctions);

    free(auctions);

    return response;
}

char *show_asset(char *aid) { return NULL; }

char *bid(char* uid, char* password, char *aid, char *value) {
    int value_ok;
    if (auction_closed(aid))
        return default_res(BID_RES, STATUS_NOK);
    else if (!user_loggedin(uid))
        return default_res(BID_RES, STATUS_NLG);
    else if (!user_ok_password(uid, password))
        return default_res(BID_RES, STATUS_ERR);
    else if ((value_ok = bid_value_ok(aid, value)) == -1)
        return default_res(BID_RES, STATUS_ERR);
    else if (!value_ok)
        return default_res(BID_RES, STATUS_REF);
    else if (auction_is_owner(aid, uid))
        return default_res(BID_RES, STATUS_ILG);

    if (make_bid(uid, aid, value) == -1)
        return default_res(BID_RES, STATUS_ERR);

    return default_res(BID_RES, STATUS_OK);
}

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

        if (parse_lin(request, uid, pass) != -1)
            response = login(uid, pass);
        else
            response = default_res(LIN_RES, STATUS_ERR);

        if (verbose)
            printf("%s\n", response);

        send_udp(socket, response, NULL);
        break;
    }
    case LOU: {
        char uid[UID_SIZE + 1];
        char pass[PASS_SIZE + 1];

        if (parse_lou(request, uid, pass) != -1)
            response = logout(uid, pass);
        else
            response = default_res(LOU_RES, STATUS_ERR);

        if (verbose)
            printf("%s\n", response);

        send_udp(socket, response, NULL);
        break;
    }
    case UNR: {
        char uid[UID_SIZE + 1];
        char pass[PASS_SIZE + 1];

        if (parse_unr(request, uid, pass) != -1)
            response = unregister(uid, pass);
        else
            response = default_res(UNR_RES, STATUS_ERR);

        if (verbose)
            printf("%s\n", response);

        send_udp(socket, response, NULL);
        break;
    }
    case OPA: {
        char uid[UID_SIZE + 1];
        char pass[PASS_SIZE + 1];
        char name[NAME_SIZE + 1];
        char start_value[START_VAL_SIZE + 1];
        char timeactive[TIME_SIZE + 1];
        char fname[FNAME_SIZE + 1];

        if (parse_opa(request, uid, pass, name, start_value, timeactive, fname) != -1)
            response = open_auc(uid, pass, name, start_value, timeactive, fname);
        else
            response = default_res(OPA_RES, STATUS_ERR);

        if (verbose)
            printf("%s\n", response);

        send_udp(socket, response, NULL);
        break;
    }
    case CLS: {
        char uid[UID_SIZE + 1];
        char pass[PASS_SIZE + 1];
        char aid[AID_SIZE + 1];


        if (parse_cls(request, uid, pass, aid) != -1)
            response = close_auc(uid, pass, aid);
        else
            response = default_res(CLS_RES, STATUS_ERR);

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
    case BID:
        break;
    case SAS:
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

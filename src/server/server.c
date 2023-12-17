#include "server.h"
#include "../util.h"
#include "connection.h"
#include "db.h"
#include "protocol.h"
#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

static char asport[] = "58012"; // 58000 + group number
static bool verbose = true;     // TODO: default is false
static int tcp_main, udp_sock;

char *login(char *uid, char *password) {
    if (!user_registered(uid)) {
        if (user_register(uid, password) == -1)
            return server_error_res();

        if (user_login(uid) == -1)
            return server_error_res();

        return default_res(LIN_RES, STATUS_REG);
    } else if (!user_loggedin(uid)) {
        if (user_ok_password(uid, password)) {
            if (user_login(uid) == -1)
                return server_error_res();

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
            return server_error_res();

        return default_res(LOU_RES, STATUS_OK);
    }

    // Wrong password
    return default_res(LOU_RES,
                       STATUS_ERR); // TODO: Decidir o que responder aqui
}

char *unregister(char *uid, char *password) {
    if (!user_registered(uid))
        return default_res(UNR_RES, STATUS_UNR);
    else if (!user_ok_password(uid, password))
        return default_res(UNR_RES,
                           STATUS_ERR); // TODO: Decidir o que responder aqui
    else if (!user_loggedin(uid))
        return default_res(UNR_RES, STATUS_NOK);

    if (user_logout(uid) == -1)
        return server_error_res();

    if (user_unregister(uid) == -1)
        return server_error_res();

    return default_res(UNR_RES, STATUS_OK);
}

char *open_auc(char *uid, char *password, char *name, char *start_value,
               char *timeactive, char *fname) {
    char *aid, *response;
    if (!user_registered(uid)) {
        return default_res(OPA_RES, STATUS_NOK);
    } else if (!user_loggedin(uid))
        return default_res(OPA_RES, STATUS_NLG);
    else if (!user_ok_password(uid, password)) {
        return default_res(OPA_RES, STATUS_NOK);
    }

    if ((aid = auction_open(uid, name, start_value, timeactive, fname)) == NULL)
        return server_error_res();
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
        return server_error_res();
    return default_res(CLS_RES, STATUS_OK);
}

char *user_auctions(char *uid) {
    if (!user_registered(uid))
        return default_res(LMA_RES, STATUS_NLG);
    else if (!user_loggedin(uid))
        return default_res(LMA_RES, STATUS_NLG);

    char *dir = user_hosted_dir(uid);
    if (count_entries(dir, DT_REG) == 0) {
        free(dir);
        return default_res(LMA_RES, STATUS_NOK);
    }

    char *auctions = list_auctions(dir, NO_LIMIT);
    char *response = list_res(LMA_RES, auctions);

    free(dir);
    free(auctions);

    return response;
}

char *user_bids(char *uid) {
    if (!user_registered(uid))
        return default_res(LMB_RES, STATUS_NLG);
    else if (!user_loggedin(uid))
        return default_res(LMB_RES, STATUS_NLG);

    char *dir = user_bidded_dir(uid);
    if (count_entries(dir, DT_REG) == 0) {
        free(dir);
        return default_res(LMB_RES, STATUS_NOK);
    }

    char *auctions = list_auctions(dir, NO_LIMIT);
    char *response = list_res(LMB_RES, auctions);

    free(dir);
    free(auctions);

    return response;
}

char *list() {
    char *dir = AUCTIONS_DIR;
    if (count_entries(dir, DT_DIR) == 0)
        return default_res(LST_RES, STATUS_NOK);

    char *auctions = list_auctions(dir, NO_LIMIT);
    char *response = list_res(LST_RES, auctions);

    free(auctions);
    return response;
}

char *show_asset(char *aid) {
    char *fname, fsize[FSIZE_SIZE + 1], *path, *response;
    long int size;
    memset(fsize, 0, (FSIZE_SIZE + 1) * sizeof(char));

    if ((fname = auction_asset_fname(aid)) == NULL)
        return default_res(SAS_RES, STATUS_NOK);
    if ((path = auction_asset_path(aid)) == NULL)
        return default_res(SAS_RES, STATUS_NOK);

    size = file_size(path);
    sprintf(fsize, "%ld", size);
    return sas_ok_res(fname, fsize);
}

char *bid(char *uid, char *password, char *aid, char *value) {
    int value_ok;
    if (auction_closed(aid))
        return default_res(BID_RES, STATUS_NOK);
    else if (!user_loggedin(uid))
        return default_res(BID_RES, STATUS_NLG);
    else if (!user_ok_password(uid, password))
        return default_res(BID_RES,
                           STATUS_ERR); // TODO: Decidir o que responder aqui
    else if ((value_ok = bid_value_ok(aid, value)) == -1)
        return server_error_res();
    else if (!value_ok)
        return default_res(BID_RES, STATUS_REF);
    else if (auction_is_owner(aid, uid))
        return default_res(BID_RES, STATUS_ILG);

    if (make_bid(uid, aid, value) == -1)
        return server_error_res();

    return default_res(BID_RES, STATUS_ACC);
}

char *show_record(char *aid) {
    if (!auction_exists(aid))
        return default_res(SRC_RES, STATUS_NOK);

    auction_update(aid);

    char *start_info = fmt_start_info(aid);
    char *bids = list_bids(aid, DEFAULT_LIMIT);
    char *end_info = fmt_end_info(aid);
    char *ok_res = src_ok_res(start_info, bids, end_info);

    free(start_info);

    if (bids != NULL)
        free(bids);

    if (end_info != NULL)
        free(end_info);

    return ok_res;
}

bool treat_request(char *request, int socket) {
    int id;
    char *response;

    id = interpret_req(request);
    switch (id) {
    case LIN: {
        char uid[UID_SIZE + 1];
        char pass[PASS_SIZE + 1];

        if (parse_lin(request, uid, pass) != -1) {
            if (verbose) {
                printf("-----------------------------\n");
                printf("Login\nuid: %s\npass: %s\n", uid, pass);
            }
            response = login(uid, pass);
        } else
            response = bad_syntax_res();

        if (verbose)
            printf("Sending: %s\n", response);
        printf("-----------------------------\n\n");

        send_udp(socket, response, NULL, verbose);
        break;
    }
    case LOU: {
        char uid[UID_SIZE + 1];
        char pass[PASS_SIZE + 1];

        if (parse_lou(request, uid, pass) != -1) {
            if (verbose) {
                printf("-----------------------------\n");
                printf("Logout\nuid: %s\npass: %s\n", uid, pass);
            }
            response = logout(uid, pass);
        } else
            response = bad_syntax_res();

        if (verbose)
            printf("Sending: %s\n", response);
        printf("-----------------------------\n\n");

        send_udp(socket, response, NULL, verbose);
        break;
    }
    case UNR: {
        char uid[UID_SIZE + 1];
        char pass[PASS_SIZE + 1];

        if (parse_unr(request, uid, pass) != -1) {
            if (verbose) {
                printf("-----------------------------\n");
                printf("Unregister\nuid: %s\npass: %s\n", uid, pass);
            }
            response = unregister(uid, pass);
        } else
            response = bad_syntax_res();

        if (verbose)
            printf("Sending: %s\n", response);
        printf("-----------------------------\n\n");

        send_udp(socket, response, NULL, verbose);
        break;
    }
    case OPA: {
        char uid[UID_SIZE + 1];
        char pass[PASS_SIZE + 1];
        char name[NAME_SIZE + 1];
        char start_value[START_VAL_SIZE + 1];
        char timeactive[TIME_SIZE + 1];
        char fname[FNAME_SIZE + 1];

        if (parse_opa(request, uid, pass, name, start_value, timeactive, fname,
                      NULL) != -1) {
            if (verbose) {
                printf("-----------------------------\n");
                printf("Open Auction\nuid: %s\npass: %s\nname: %s\nstart "
                       "value: %s\ntime active: %s\nfname: %s\n",
                       uid, pass, name, start_value, timeactive, fname);
            }
            response =
                open_auc(uid, pass, name, start_value, timeactive, fname);
        } else {
            if (verbose) {
                printf("-----------------------------\n");
                printf("Bad Format\nReceived: %s\n", request);
            }
            response = bad_syntax_res();
        }

        if (verbose)
            printf("Sending: %s\n", response);
        printf("-----------------------------\n\n");

        if (is_req_ok(response)) {
            free(request);
            return prepare_freceive(socket, true, response, verbose);
        } else {
            return prepare_freceive(socket, false, response, verbose);
        }
        break;
    }
    case CLS: {
        char uid[UID_SIZE + 1];
        char pass[PASS_SIZE + 1];
        char aid[AID_SIZE + 1];

        if (parse_cls(request, uid, pass, aid) != -1) {
            if (verbose) {
                printf("-----------------------------\n");
                printf("Close Auction\nuid: %s\npass: %s\naid: %s\n", uid, pass,
                       aid);
            }
            response = close_auc(uid, pass, aid);
        } else {
            if (verbose) {
                printf("-----------------------------\n");
                printf("Bad Format\nReceived: %s\n", request);
            }
            response = bad_syntax_res();
        }

        if (verbose)
            printf("Sending: %s\n", response);
        printf("-----------------------------\n\n");

        send_tcp(socket, response, verbose);
        break;
    }
    case LMA: {
        char uid[UID_SIZE + 1];

        if (parse_lma(request, uid) != -1) {
            if (verbose) {
                printf("-----------------------------\n");
                printf("User Auctions\nuid: %s\n", uid);
            }
            response = user_auctions(uid);
        } else {
            if (verbose) {
                printf("-----------------------------\n");
                printf("Bad Format\nReceived: %s\n", request);
            }
            response = bad_syntax_res();
        }

        if (verbose)
            printf("Sending: %s\n", response);
        printf("-----------------------------\n\n");

        send_udp(socket, response, NULL, verbose);
        break;
    }
    case LMB: {
        char uid[UID_SIZE + 1];

        if (parse_lmb(request, uid) != -1) {
            if (verbose) {
                printf("-----------------------------\n");
                printf("User Bids\nuid: %s\n", uid);
            }
            response = user_bids(uid);
        } else {
            if (verbose) {
                printf("-----------------------------\n");
                printf("Bad Format\nReceived: %s\n", request);
            }
            response = bad_syntax_res();
        }

        if (verbose)
            printf("Sending: %s\n", response);
        printf("-----------------------------\n\n");

        send_udp(socket, response, NULL, verbose);
        break;
    }
    case LST: {
        if (verbose) {
            printf("-----------------------------\n");
            printf("List\n");
        }

        response = list();

        if (verbose)
            printf("Sending: %s\n", response);
        printf("-----------------------------\n\n");

        send_udp(socket, response, NULL, verbose);
        break;
    }
    case SAS: {
        char aid[AID_SIZE + 1];
        char *path;

        if (parse_sas(request, aid) != -1) {
            if (verbose) {
                printf("-----------------------------\n");
                printf("Show Asset\naid: %s\n", aid);
            }
            response = show_asset(aid);
        } else {
            if (verbose) {
                printf("-----------------------------\n");
                printf("Bad Format\nReceived: %s\n", request);
            }
            response = bad_syntax_res();
        }

        if (verbose)
            printf("Sending: %s\n", response);
        printf("-----------------------------\n\n");

        if (is_req_ok(response)) {
            path = auction_asset_path(aid);
            send_tcp_file(socket, response, path, verbose);
            free(path);
        } else {
            send_tcp(socket, response, verbose);
        }
        break;
    }
    case BID: {
        char uid[UID_SIZE + 1];
        char pass[PASS_SIZE + 1];
        char aid[AID_SIZE + 1];
        char value[START_VAL_SIZE + 1];

        if (parse_bid(request, uid, pass, aid, value) != -1) {
            if (verbose) {
                printf("-----------------------------\n");
                printf("Bid\nuid: %s\npath: %s\naid: %s\nvalue: %s\n", uid,
                       pass, aid, value);
            }
            response = bid(uid, pass, aid, value);
        } else {
            if (verbose) {
                printf("-----------------------------\n");
                printf("Bad Format\nReceived: %s\n", request);
            }
            response = bad_syntax_res();
        }

        if (verbose)
            printf("Sending: %s\n", response);
        printf("-----------------------------\n\n");

        send_tcp(socket, response, verbose);
        break;
    }
    case SRC: {
        char aid[AID_SIZE + 1];

        if (parse_src(request, aid) != -1) {
            if (verbose) {
                printf("-----------------------------\n");
                printf("Show Record\naid: %s\n", aid);
            }
            response = show_record(aid);
        } else {
            if (verbose) {
                printf("-----------------------------\n");
                printf("Bad Format\nReceived: %s\n", request);
            }
            response = bad_syntax_res();
        }

        if (verbose)
            printf("Sending: %s\n", response);
        printf("-----------------------------\n\n");

        send_udp(socket, response, NULL, verbose);
        break;
    }
    default: {
        if (verbose) {
            printf("-----------------------------\n");
            printf("Bad Format\nReceived: %s\n", request);
        }

        response = bad_syntax_res();

        if (verbose)
            printf("Sending: %s\n", response);
        printf("-----------------------------\n\n");

        if (socket == udp_sock)
            send_udp(socket, response, NULL, verbose);
        else
            send_tcp(socket, response, verbose);
        break;
    }
    }

    free(response);
    free(request);
    return true;
}

void handle_sockets() {
    int out_select, temp_sock;
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
        case 0: {
            // timeout
            for (int i = 0; i < FD_SETSIZE; i++) {
                if (FD_ISSET(i, &ready_sockets)) {
                    if (i != tcp_main && i != udp_sock) {
                        char *bad_syntax = bad_syntax_res();

                        if (verbose) {
                            printf("info: timeout on port %d (tcp)\n", i);
                            printf("Sending: %s\n", bad_syntax);
                        }

                        send_tcp(i, bad_syntax, verbose);
                        free(bad_syntax);
                        FD_CLR(i, &current_sockets);
                        if (verbose)
                            printf("info: removed port %d (tcp) from "
                                   "set\n",
                                   i);
                    }
                }
            }
            break;
        }
        case -1:
            printf("error: socket select failed\n");
            exit(EXIT_FAILURE);
            break;
        default:
            for (int i = 0; i < FD_SETSIZE; i++) {
                if (FD_ISSET(i, &ready_sockets)) {
                    if (i == tcp_main) {
                        // new tcp connecton
                        temp_sock = accept_new_tcp(tcp_main, verbose);
                        if (temp_sock != -1) {
                            if (verbose)
                                printf("info: new conn on port %d (tcp)\n",
                                       temp_sock);
                            FD_SET(temp_sock, &current_sockets);
                        }
                    } else if (i == udp_sock) {
                        buffer = receive_udp(udp_sock, verbose);
                        if (buffer != NULL) {
                            if (verbose)
                                printf("info: packet received (udp)\n");
                            treat_request(buffer, i);
                        }
                    } else {
                        buffer = receive_tcp(i, verbose);
                        if (buffer != NULL) {
                            if (buffer[0] == '\n') {
                                // OPA finished
                                free(buffer);
                                FD_CLR(i, &current_sockets);
                                if (verbose)
                                    printf("info: finished receiving file. "
                                           "removed port %d (tcp) from "
                                           "set\n",
                                           i);
                            } else if (buffer[0] == '\0') {
                                // critical connection error
                                char *error_res = server_error_res();
                                if (verbose)
                                    printf("info: sending (socket %d): %s\n", i,
                                           error_res);

                                send_tcp(i, error_res, verbose);
                                free(error_res);
                                FD_CLR(i, &current_sockets);
                                if (verbose)
                                    printf("info: removed port %d (tcp) from "
                                           "set\n",
                                           i);
                            } else {
                                if (verbose)
                                    printf("info: packet received on port %d "
                                           "(tcp)\n",
                                           i);
                                if (treat_request(buffer, i)) {
                                    FD_CLR(i, &current_sockets);
                                    if (verbose)
                                        printf("info: removed port %d (tcp) "
                                               "from set\n",
                                               i);
                                }
                            }
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

    printf("Auction Server\n");
    printf("Server PORT: %s\n\n", asport);
    handle_sockets();
}

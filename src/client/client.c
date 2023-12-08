#include "client.h"
#include "../util.h"
#include "connection.h"
#include "protocol.h"
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static char asip[128] = "127.0.0.1";
static char asport[6] = "58012"; // 58000 + group number
static User user = {false, "", ""};

void login(char *uid, char *password) {
    char *request, *response;
    bool ok;

    if (user.logged_in) {
        printf("error: already logged in\n");
        return;
    }

    // check for valid arguments
    // UID
    if (strlen(uid) != UID_SIZE || !is_numeric(uid)) {
        printf("error: wrong uid format\n");
        return;
    }
    // password
    if (strlen(password) != PASS_SIZE || !is_alphanumeric(password)) {
        printf("error: wrong password format\n");
        return;
    }

    request = login_req(uid, password);
    response = use_udp(asip, asport, request, LIN_SIZE, RECV_SIZE_DEFAULT);

    ok = login_res(response, true);
    if (!ok) {
        free(request);
        free(response);
        return;
    }

    // clear current uid and password
    memset(user.uid, 0, sizeof(user.uid));
    memset(user.password, 0, sizeof(user.password));
    // copy new uid and password
    strcpy(user.uid, uid);
    strcpy(user.password, password);
    user.logged_in = true;

    free(request);
    free(response);
    return;
}

void logout() {
    char *request, *response;
    bool ok;

    if (!user.logged_in) {
        printf("error: not logged in\n");
        return;
    }

    request = logout_req(user.uid, user.password);
    response = use_udp(asip, asport, request, LOU_SIZE, RECV_SIZE_DEFAULT);

    ok = logout_res(response, true);
    if (!ok) {
        free(request);
        free(response);
        return;
    }

    // TODO: should we logout if logout goes wrong?
    // clear current uid and password
    memset(user.uid, 0, sizeof(user.uid));
    memset(user.password, 0, sizeof(user.password));
    user.logged_in = false;

    free(request);
    free(response);
    return;
}

void unregister() {
    char *request, *response;
    bool ok;

    if (!user.logged_in) {
        printf("error: not logged in\n");
        return;
    }

    request = unregister_req(user.uid, user.password);
    response = use_udp(asip, asport, request, UNR_SIZE, RECV_SIZE_DEFAULT);

    ok = unregister_res(response, true);
    if (!ok) {
        free(request);
        free(response);
        return;
    }

    // TODO: should we logout if unregistered goes wrong?
    // clear current uid and password
    memset(user.uid, 0, sizeof(user.uid));
    memset(user.password, 0, sizeof(user.password));
    user.logged_in = false;

    free(request);
    free(response);
    return;
}

bool exit_prompt() {
    if (user.logged_in) {
        printf("error: please logout first\n");
        return false;
    }
    return true;
}

void open_auc(char *name, char *asset_fname, char *start_value,
              char *timeactive) {}

void close_auc(char *aid) {
    char *request, *response;

    if (!user.logged_in) {
        printf("error: not logged in\n");
        return;
    }

    request = close_req(user.uid, user.password, aid);
    // TODO: este nome sqe nao faz sentido, rever esta parte do recvsize
    response = use_tcp(asip, asport, request, CLS_SIZE, RECV_SIZE_DEFAULT);

    close_res(response, true);

    free(request);
    free(response);
    return;
}

void myauctions() {}

void mybids() {}

void list() {
    char *request, *response;

    request = list_req();
    response = use_udp(asip, asport, request, LST_SIZE, RECV_SIZE_LST);
    list_res(response, true);

    free(request);
    free(response);
    return;
}

void show_asset(char *aid) {
    bool ok;
    char *request;

    request = sas_req(aid);
    ok = transfer_file(asip, asport, request, SAS_SIZE);

    if(!ok)
        printf("error: auction server couldn't send the image\n");
    
    free(request);
    return;
}

void bid(char *aid, char *value) {
    int val_size = strlen(value);
    char *request, *response;

    if (!user.logged_in) {
        printf("error: not logged in\n");
        return;
    }

    request = bid_req(user.uid, user.password, aid, value);
    // TODO: este nome sqe nao faz sentido, rever esta parte do recvsize
    response = use_tcp(asip, asport, request, BID_SIZE + val_size + 1, RECV_SIZE_DEFAULT);

    bid_res(response, true);

    free(request);
    free(response);
    return;
}

void show_record(char *aid) {}

bool prompt_command(char *command) {
    // split arguments
    const char delimiter[] = " \t";
    char *token;
    char *prompt_args[5];
    int arg_count = 0;

    token = strtok(command, delimiter);
    while (token != NULL) {
        prompt_args[arg_count++] = token;
        token = strtok(NULL, delimiter);

        if (arg_count > 5) {
            printf("error: too many arguments\n");
            return false;
        }
    }

    // execute command
    if (strcmp(prompt_args[0], "login") == 0) {
        if (arg_count != 3) {
            printf("error: incorrect number of arguments\n");
            return false;
        }
        login(prompt_args[1], prompt_args[2]);
    } else if (strcmp(prompt_args[0], "logout") == 0) {
        if (arg_count != 1) {
            printf("error: incorrect number of arguments\n");
            return false;
        }
        logout();
    } else if (strcmp(prompt_args[0], "unregister") == 0) {
        if (arg_count != 1) {
            printf("error: incorrect number of arguments\n");
            return false;
        }
        unregister();
    } else if (strcmp(prompt_args[0], "exit") == 0) {
        if (arg_count != 1) {
            printf("error: incorrect number of arguments\n");
            return false;
        }
        return exit_prompt();
    } else if (strcmp(prompt_args[0], "open") == 0) {
        if (arg_count != 5) {
            printf("error: incorrect number of arguments\n");
            return false;
        }
        open_auc(prompt_args[1], prompt_args[2], prompt_args[3],
                 prompt_args[4]);
    } else if (strcmp(prompt_args[0], "close") == 0) {
        if (arg_count != 2) {
            printf("error: incorrect number of arguments\n");
            return false;
        }
        close_auc(prompt_args[1]);
    } else if (strcmp(prompt_args[0], "myauctions") == 0 ||
               strcmp(prompt_args[0], "ma") == 0) {
        if (arg_count != 1) {
            printf("error: incorrect number of arguments\n");
            return false;
        }
        myauctions();
    } else if (strcmp(prompt_args[0], "mybids") == 0 ||
               strcmp(command, "mb") == 0) {
        if (arg_count != 1) {
            printf("error: incorrect number of arguments\n");
            return false;
        }
        mybids();
    } else if (strcmp(prompt_args[0], "list") == 0 ||
               strcmp(command, "l") == 0) {
        if (arg_count != 1) {
            printf("error: incorrect number of arguments\n");
            return false;
        }
        list();
    } else if (strcmp(prompt_args[0], "show_asset") == 0 ||
               strcmp(prompt_args[0], "sa") == 0) {
        if (arg_count != 2) {
            printf("error: incorrect number of arguments\n");
            return false;
        }
        show_asset(prompt_args[1]);
    } else if (strcmp(prompt_args[0], "bid") == 0 ||
               strcmp(command, "b") == 0) {
        if (arg_count != 3) {
            printf("error: incorrect number of arguments\n");
            return false;
        }
        bid(prompt_args[1], prompt_args[2]);
    } else if (strcmp(prompt_args[0], "show_record") == 0 ||
               strcmp(prompt_args[0], "sr") == 0) {
        if (arg_count != 2) {
            printf("error: incorrect number of arguments\n");
            return false;
        }
        show_record(prompt_args[1]);
    } else
        printf("error: unknown command\n");

    return false;
}

int prompt() {
    char *lineptr = NULL;
    size_t n = 0;
    ssize_t line_size;
    bool exit = false;

    do {
        if (user.logged_in)
            printf("(%s) ", user.uid);
        printf("%s", "> ");

        line_size = getline(&lineptr, &n, stdin);
        // remove \n
        if (line_size > 0) {
            if (lineptr[line_size - 1] == '\n')
                lineptr[line_size - 1] = '\0';
        }

        exit = prompt_command(lineptr);
    } while (!exit);

    free(lineptr);
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "n:p:")) != -1) {
        switch (opt) {
        case 'n':
            // TODO: check for valid ip/url
            memset(asip, 0, sizeof(asip));
            strcpy(asip, optarg);
            break;
        case 'p':
            // TODO: check for valid port num
            memset(asport, 0, sizeof(asport));
            strncpy(asport, optarg, 5);
            break;
        case '?':
            printf("unknown argument: %c\n", opt);
            exit(EXIT_FAILURE);
            break;
        }
    }

    printf("Auction Client\n\n");
    printf("Server IP/URL: %s\n", asip);
    printf("Server PORT: %s\n\n", asport);
    return prompt();
}

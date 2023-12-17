#include "client.h"
#include "../util.h"
#include "connection.h"
#include "protocol.h"
#include <fcntl.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
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
    if (!is_valid_uid(uid)) {
        printf("error: wrong uid format\n");
        return;
    }
    if (!is_valid_password(password)) {
        printf("error: wrong password format\n");
        return;
    }

    request = login_req(uid, password);
    response = use_udp(asip, asport, request, LIN_SIZE, DEFAULT_SIZE);
    if (response == NULL) {
        free(request);
        return;
    }

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
    response = use_udp(asip, asport, request, LOU_SIZE, DEFAULT_SIZE);
    if (response == NULL) {
        free(request);
        return;
    }

    ok = logout_res(response, true);
    if (!ok)
        printf("error: server side logout failed\n");

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
    response = use_udp(asip, asport, request, UNR_SIZE, DEFAULT_SIZE);
    if (response == NULL) {
        free(request);
        return;
    }

    ok = unregister_res(response, true);
    if (!ok)
        printf("error: server side unregister failed\n");

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
    printf("Exiting...\n");
    return true;
}

void open_auc(char *name, char *asset_fname, char *start_value,
              char *timeactive) {
    int fd;
    char *request, *response;

    if (!user.logged_in) {
        printf("error: not logged in\n");
        return;
    }

    if (!is_valid_name(name)) {
        printf("erro: invalid name\n");
        return;
    }
    if (!is_valid_fname(asset_fname)) {
        printf("erro: invalid file name\n");
        return;
    }
    if (!is_valid_value(start_value)) {
        printf("erro: invalid start value\n");
        return;
    }
    if (!is_valid_timeactive(timeactive)) {
        printf("erro: invalid time active\n");
        return;
    }

    fd = open(asset_fname, O_RDONLY);
    if (fd == -1) {
        printf("error: failed to open file\n");
        return;
    }

    struct stat st;
    int failed = fstat(fd, &st);
    if (failed) {
        printf("error: failed to get file size\n");
        close(fd);
        return;
    }
    int file_bytes = st.st_size;
    if (file_bytes > MAX_FSIZE) {
        printf("error: file is too big\n");
        close(fd);
        return;
    }
    char *fsize = (char *)malloc(FSIZE_SIZE * sizeof(char));
    memset(fsize, 0, FSIZE_SIZE * sizeof(char));
    sprintf(fsize, "%d", file_bytes);

    request = opa_req(user.uid, user.password, name, start_value, timeactive,
                      asset_fname, fsize);
    response = send_file(asip, asport, request, strlen(request), asset_fname,
                         DEFAULT_SIZE);
    if (response == NULL) {
        free(fsize);
        free(request);
        return;
    }

    opa_res(response, true);

    close(fd);
    free(fsize);
    free(request);
    free(response);
    return;
}

void close_auc(char *raw_aid) {
    char *request, *response;
    char *aid = fmt_aid(raw_aid);

    if (!user.logged_in) {
        printf("error: not logged in\n");
        return;
    }

    if (!is_valid_aid(aid)) {
        printf("erro: invalid aid\n");
        return;
    }

    request = close_req(user.uid, user.password, aid);
    response = use_tcp(asip, asport, request, CLS_SIZE, DEFAULT_SIZE);
    if (response == NULL) {
        free(request);
        return;
    }

    close_res(response, true);

    free(request);
    free(response);
    if (aid != raw_aid)
        free(aid);
    return;
}

void myauctions() {
    char *request, *response;

    if (!user.logged_in) {
        printf("error: not logged in\n");
        return;
    }

    request = lma_req(user.uid);
    response = use_udp(asip, asport, request, LMA_SIZE, AUCTION_LIST_SIZE);
    if (response == NULL) {
        free(request);
        return;
    }

    lma_res(response, true);

    free(request);
    free(response);
    return;
}

void mybids() {
    char *request, *response;

    if (!user.logged_in) {
        printf("error: not logged in\n");
        return;
    }

    request = lmb_req(user.uid);
    response = use_udp(asip, asport, request, LMB_SIZE, AUCTION_LIST_SIZE);
    if (response == NULL) {
        free(request);
        return;
    }

    lmb_res(response, true);

    free(request);
    free(response);
    return;
}

void list() {
    char *request, *response;

    request = list_req();
    response = use_udp(asip, asport, request, LST_SIZE, AUCTION_LIST_SIZE);
    if (response == NULL) {
        free(request);
        return;
    }

    list_res(response, true);

    free(request);
    free(response);
    return;
}

void show_asset(char *raw_aid) {
    bool ok;
    char *request;
    char *aid = fmt_aid(raw_aid);

    if (!is_valid_aid(aid)) {
        printf("erro: invalid aid\n");
        return;
    }

    request = sas_req(aid);
    ok = transfer_file(asip, asport, request, SAS_SIZE, true);

    if (!ok)
        printf("error: failed to receive file\n");

    free(request);
    if (aid != raw_aid)
        free(aid);
    return;
}

void bid(char *raw_aid, char *value) {
    char *request, *response;
    char *aid = fmt_aid(raw_aid);

    if (!user.logged_in) {
        printf("error: not logged in\n");
        return;
    }

    if (!is_valid_aid(aid)) {
        printf("erro: invalid aid\n");
        return;
    }
    if (!is_valid_value(value)) {
        printf("erro: invalid value\n");
        return;
    }

    request = bid_req(user.uid, user.password, aid, value);
    response = use_tcp(asip, asport, request, strlen(request), DEFAULT_SIZE);
    if (response == NULL) {
        free(request);
        return;
    }

    bid_res(response, true);

    free(request);
    free(response);
    if (aid != raw_aid)
        free(aid);
    return;
}

void show_record(char *raw_aid) {
    char *request, *response;
    char *aid = fmt_aid(raw_aid);

    if (!is_valid_aid(aid)) {
        printf("erro: invalid aid\n");
        return;
    }

    request = src_req(aid);
    response = use_udp(asip, asport, request, SRC_SIZE, BID_LIST_SIZE);
    if (response == NULL) {
        free(request);
        return;
    }

    src_res(response, true);

    free(request);
    free(response);
    if (aid != raw_aid)
        free(aid);
    return;
}

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
            memset(asip, 0, sizeof(asip));
            strcpy(asip, optarg);
            break;
        case 'p': {
            int port_num = atoi(optarg);
            if (port_num < 1024 || port_num > 65535) {
                printf("%s is not a valid port\n", optarg);
                exit(EXIT_FAILURE);
            }

            memset(asport, 0, 6);
            strncpy(asport, optarg, 5);
            break;
        }
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

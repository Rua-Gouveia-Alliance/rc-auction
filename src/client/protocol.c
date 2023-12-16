#include "protocol.h"
#include "../util.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *login_req(char *uid, char *password) {
    // setup buffer
    char *msg = malloc((LIN_SIZE + 1) * sizeof(char));
    memset(msg, 0, (LIN_SIZE + 1) * sizeof(char));

    // copy data
    strcpy(msg, LIN_REQ);
    strcpy(&msg[CMD_SIZE + 1], uid);
    msg[CMD_SIZE + 1 + UID_SIZE] = ' ';
    strcpy(&msg[CMD_SIZE + 1 + UID_SIZE + 1], password);
    msg[CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE] = '\n';

    return msg;
}

bool login_res(char *response, bool print) {
    char *status = response + 4;

    if (strncmp(response, LIN_RES, 3) != 0) {
        if (print)
            printf("error: wrong server response format\n");
        return false;
    } else if (strcmp(status, STATUS_OK) == 0) {
        if (print)
            printf("login successful\n");
        return true;
    } else if (strcmp(status, STATUS_NOK) == 0) {
        if (print)
            printf("wrong password\n");
        return false;
    } else if (strcmp(status, STATUS_REG) == 0) {
        if (print)
            printf("new user registered\nlogin successful\n");
        return true;
    } else {
        if (print)
            printf("error: wrong server response format\n");
        return false;
    }
}

char *logout_req(char *uid, char *password) {
    // setup buffer
    char *msg = malloc((LOU_SIZE + 1) * sizeof(char));
    memset(msg, 0, (LOU_SIZE + 1) * sizeof(char));

    // copy data
    strcpy(msg, LOU_REQ);
    strcpy(&msg[CMD_SIZE + 1], uid);
    msg[CMD_SIZE + 1 + UID_SIZE] = ' ';
    strcpy(&msg[CMD_SIZE + 1 + UID_SIZE + 1], password);
    msg[CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE] = '\n';

    return msg;
}

bool logout_res(char *response, bool print) {
    char *status = response + 4;

    if (strncmp(response, LOU_RES, 3) != 0) {
        if (print)
            printf("error: wrong server response format\n");
        return false;
    } else if (strcmp(status, STATUS_OK) == 0) {
        if (print)
            printf("logout successful\n");
        return true;
    } else if (strcmp(status, STATUS_NOK) == 0) {
        if (print)
            printf("error: user not logged in\n");
        return false;
    } else if (strcmp(status, STATUS_UNR) == 0) {
        if (print)
            printf("error: user not registered\n");
        return true;
    } else {
        if (print)
            printf("error: wrong server response format\n");
        return false;
    }
}

char *unregister_req(char *uid, char *password) {
    // setup buffer
    char *msg = malloc((UNR_SIZE + 1) * sizeof(char));
    memset(msg, 0, (UNR_SIZE + 1) * sizeof(char));

    // copy data
    strcpy(msg, UNR_REQ);
    strcpy(&msg[CMD_SIZE + 1], uid);
    msg[CMD_SIZE + 1 + UID_SIZE] = ' ';
    strcpy(&msg[CMD_SIZE + 1 + UID_SIZE + 1], password);
    msg[CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE] = '\n';

    return msg;
}

bool unregister_res(char *response, bool print) {
    char *status = response + 4;

    if (strncmp(response, UNR_RES, 3) != 0) {
        if (print)
            printf("error: wrong server response format\n");
        return false;
    } else if (strcmp(status, STATUS_OK) == 0) {
        if (print)
            printf("unregister successful\n");
        return true;
    } else if (strcmp(status, STATUS_NOK) == 0) {
        if (print)
            printf("error: user not logged in\n");
        return false;
    } else if (strcmp(status, STATUS_UNR) == 0) {
        if (print)
            printf("error: user not registered\n");
        return true;
    } else {
        if (print)
            printf("error: wrong server response format\n");
        return false;
    }
}

void print_list(char *response) {
    int last_pos;
    char *token, *list_pos;

    // check \n
    last_pos = strlen(response) - 1;
    if (response[last_pos] != '\n') {
        printf("error: wrong server response format\n");
        return;
    }
    response[last_pos] = '\0'; // replace \n with \0

    list_pos = response + STATUS_OK_SIZE;
    token = strtok(list_pos, " ");
    while (token != NULL) {
        // AID
        if (strlen(token) != 3 || !is_numeric(token)) {
            printf("error: wrong server response format\n");
            return;
        }
        printf("%s", token);

        // status
        token = strtok(NULL, " ");
        if (token == NULL || strlen(token) != 1 ||
            (*token != '0' && *token != '1')) {
            printf("error: wrong server response format\n");
            return;
        }
        printf(" (%s)\n", *token == '1' ? "active" : "inactive");

        // next token
        token = strtok(NULL, " ");
    }
}

char *lmb_req(char *uid) {
    // setup buffer
    char *msg = malloc((LMA_SIZE + 1) * sizeof(char));
    memset(msg, 0, (LMA_SIZE + 1) * sizeof(char));

    // copy data
    strcpy(msg, LMB_REQ);
    strcpy(&msg[CMD_SIZE + 1], uid);
    msg[CMD_SIZE + 1 + UID_SIZE] = '\n';
    return msg;
}

void lmb_res(char *response, bool print) {
    char *status = response + 4;

    if (strncmp(response, LMB_RES, 3) != 0) {
        if (print)
            printf("error: wrong server response format\n");
    } else if (strncmp(status, STATUS_OK, 2) == 0) {
        if (print)
            print_list(response);
    } else if (strcmp(status, STATUS_NOK) == 0) {
        if (print)
            printf("user has no ongoing bids\n");
    } else if (strcmp(status, STATUS_NLG) == 0) {
        if (print)
            printf("user is not logged in\n");
    } else {
        if (print)
            printf("error: wrong server response format\n");
    }
}

void print_record(char *response) {
    int last_pos;
    char *token, *list_pos;

    // check \n
    last_pos = strlen(response) - 1;
    if (response[last_pos] != '\n') {
        printf("error: wrong server response format\n");
        return;
    }
    response[last_pos] = '\0'; // replace \n with \0

    list_pos = response + STATUS_OK_SIZE;
    token = strtok(list_pos, " ");

    if (token == NULL) {
        printf("error: wrong server response format\n");
        return;
    }

    // Printing auction related data
    printf("\nHosted by: %s\n", token);
    token = strtok(NULL, " ");

    if (token == NULL) {
        printf("error: wrong server response format\n");
        return;
    }

    printf("Auction name: %s\n", token);
    token = strtok(NULL, " ");

    if (token == NULL) {
        printf("error: wrong server response format\n");
        return;
    }

    printf("File name: %s\n", token);
    token = strtok(NULL, " ");

    if (token == NULL) {
        printf("error: wrong server response format\n");
        return;
    }

    printf("Start value: %s\n", token);
    token = strtok(NULL, " ");

    if (token == NULL) {
        printf("error: wrong server response format\n");
        return;
    }

    printf("Start date: %s ", token);
    token = strtok(NULL, " ");

    if (token == NULL) {
        printf("error: wrong server response format\n");
        return;
    }

    printf("%s\n", token);
    token = strtok(NULL, " ");

    if (token == NULL) {
        printf("error: wrong server response format\n");
        return;
    }

    printf("Duration: %ss\n", token);
    token = strtok(NULL, " ");

    if (token == NULL) {
        printf("\n");
        return;
    }

    if (token[0] == 'B')
        printf("\nBids:\n");
    else
        printf("\nNo bids were made\n");

    while (token[0] == 'B') {
        token = strtok(NULL, " ");

        if (token == NULL) {
            printf("\nerror: wrong server response format\n");
            return;
        }

        printf("User %s ", token);
        token = strtok(NULL, " ");

        if (token == NULL) {
            printf("\nerror: wrong server response format\n");
            return;
        }

        printf("bid %s on ", token);
        token = strtok(NULL, " ");

        if (token == NULL) {
            printf("\nerror: wrong server response format\n");
            return;
        }

        printf("%s ", token);
        token = strtok(NULL, " ");

        if (token == NULL) {
            printf("\nerror: wrong server response format\n");
            return;
        }

        printf("%s ", token);
        token = strtok(NULL, " ");

        if (token == NULL) {
            printf("\nerror: wrong server response format\n");
            return;
        }

        printf("%ss\n", token);
        token = strtok(NULL, " ");

        if (token == NULL) {
            printf("\n");
            return;
        }
    }

    printf("\n");
    token = strtok(NULL, " ");

    if (token == NULL) {
        printf("\nerror: wrong server response format\n");
        return;
    }

    printf("This auction finished on %s, ", token);
    token = strtok(NULL, " ");

    if (token == NULL) {
        printf("\nerror: wrong server response format\n");
        return;
    }

    printf("%s and lasted ", token);
    token = strtok(NULL, " ");

    if (token == NULL) {
        printf("\nerror: wrong server response format\n");
        return;
    }

    printf("%ss\n\n", token);
}

char *src_req(char *aid) {
    // setup buffer
    char *msg = malloc((SRC_SIZE + 1) * sizeof(char));
    memset(msg, 0, (SRC_SIZE + 1) * sizeof(char));

    // copy data
    strcpy(msg, SRC_REQ);
    strcpy(&msg[CMD_SIZE + 1], aid);
    msg[CMD_SIZE + 1 + AID_SIZE] = '\n';
    return msg;
}

void src_res(char *response, bool print) {
    char *status = response + 4;

    if (strncmp(response, SRC_RES, 3) != 0) {
        if (print)
            printf("error: wrong server response format\n");
    } else if (strncmp(status, STATUS_OK, 2) == 0) {
        if (print)
            print_record(response);
    } else if (strcmp(status, STATUS_NOK) == 0) {
        if (print)
            printf("the provided auction does not exist\n");
    } else {
        if (print)
            printf("error: wrong server response format\n");
    }
}

char *lma_req(char *uid) {
    // setup buffer
    char *msg = malloc((LMA_SIZE + 1) * sizeof(char));
    memset(msg, 0, (LMA_SIZE + 1) * sizeof(char));

    // copy data
    strcpy(msg, LMA_REQ);
    strcpy(&msg[CMD_SIZE + 1], uid);
    msg[CMD_SIZE + 1 + UID_SIZE] = '\n';
    return msg;
}

void lma_res(char *response, bool print) {
    char *status = response + 4;

    if (strncmp(response, LMA_RES, 3) != 0) {
        if (print)
            printf("error: wrong server response format\n");
    } else if (strncmp(status, STATUS_OK, 2) == 0) {
        if (print)
            print_list(response);
    } else if (strcmp(status, STATUS_NOK) == 0) {
        if (print)
            printf("user has no ongoing auctions\n");
    } else if (strcmp(status, STATUS_NLG) == 0) {
        if (print)
            printf("user is not logged in\n");
    } else {
        if (print)
            printf("error: wrong server response format\n");
    }
}

char *list_req() {
    // setup buffer
    char *msg = malloc((LST_SIZE + 1) * sizeof(char));
    memset(msg, 0, (LST_SIZE + 1) * sizeof(char));

    // copy data
    strcpy(msg, LST_REQ);
    return msg;
}

void list_res(char *response, bool print) {
    char *status = response + 4;

    if (strncmp(response, RLS_OK, STATUS_OK_SIZE) == 0) {
        if (print)
            print_list(response);
    } else if (strcmp(status, STATUS_NOK) == 0) {
        if (print)
            printf("no auctions started yet\n");
    } else {
        if (print)
            printf("error: wrong server response format\n");
    }
}

char *close_req(char *uid, char *password, char *aid) {
    // setup buffer
    char *msg = malloc((CLS_SIZE + 1) * sizeof(char));
    memset(msg, 0, (CLS_SIZE + 1) * sizeof(char));

    // copy data
    strcpy(msg, CLS_REQ);
    strcpy(&msg[CMD_SIZE + 1], uid);
    msg[CMD_SIZE + 1 + UID_SIZE] = ' ';
    strcpy(&msg[CMD_SIZE + 1 + UID_SIZE + 1], password);
    msg[CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE] = ' ';
    strcpy(&msg[CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE + 1], aid);
    msg[CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE + 1 + AID_SIZE] = '\n';

    return msg;
}

bool close_res(char *response, bool print) {
    char *status = response + 4;

    if (strncmp(response, CLS_RES, 3) != 0) {
        if (print)
            printf("error: wrong server response format\n");
        return false;
    } else if (strcmp(status, STATUS_OK) == 0) {
        if (print)
            printf("close successful\n");
        return true;
    } else if (strcmp(status, STATUS_NOK) == 0) {
        if (print)
            printf("error: user doesn't exist or wrong password\n");
        return false;
    } else if (strcmp(status, STATUS_NLG) == 0) {
        if (print)
            printf("error: user not logged in\n");
        return false;
    } else if (strcmp(status, STATUS_EAU) == 0) {
        if (print)
            printf("error: auction does not exist\n");
        return false;
    } else if (strcmp(status, STATUS_EOW) == 0) {
        if (print)
            printf("error: auction not owned\n");
        return false;
    } else if (strcmp(status, STATUS_END) == 0) {
        if (print)
            printf("error: auction has already finished\n");
        return false;
    } else {
        if (print)
            printf("error: wrong server response format\n");
        return false;
    }
}

char *bid_req(char *uid, char *password, char *aid, char *value) {
    // getting value size, needed for buffer
    int val_size = strlen(value);

    // setup buffer
    char *msg = malloc((BID_SIZE + val_size + 1) * sizeof(char));
    memset(msg, 0, (BID_SIZE + val_size + 1) * sizeof(char));

    // copy data
    strcpy(msg, BID_REQ);
    strcpy(&msg[CMD_SIZE + 1], uid);
    msg[CMD_SIZE + 1 + UID_SIZE] = ' ';
    strcpy(&msg[CMD_SIZE + 1 + UID_SIZE + 1], password);
    msg[CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE] = ' ';
    strcpy(&msg[CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE + 1], aid);
    msg[CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE + 1 + AID_SIZE] = ' ';
    strcpy(&msg[CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE + 1 + AID_SIZE + 1],
           value);
    msg[CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE + 1 + AID_SIZE + 1 + val_size] =
        '\n';

    return msg;
}

bool bid_res(char *response, bool print) {
    char *status = response + 4;

    if (strncmp(response, BID_RES, 3) != 0) {
        if (print)
            printf("error: wrong server response format\n");
        return false;
    } else if (strcmp(status, STATUS_ACC) == 0) {
        if (print)
            printf("bid successful\n");
        return true;
    } else if (strcmp(status, STATUS_NOK) == 0) {
        if (print)
            printf("error: auction not active\n");
        return false;
    } else if (strcmp(status, STATUS_REF) == 0) {
        if (print)
            printf("error: bid refused, there's a bigger bid already\n");
        return false;
    } else if (strcmp(status, STATUS_ILG) == 0) {
        if (print)
            printf("error: can't bid on own auction\n");
        return false;
    } else {
        if (print)
            printf("error: wrong server response format\n");
        return false;
    }
}

char *opa_req(char *uid, char *password, char *name, char *start_value,
              char *timeactive, char *fname, char *fsize) {
    // setup buffer
    char *msg = malloc((OPA_SIZE + 1) * sizeof(char));
    memset(msg, 0, (OPA_SIZE + 1) * sizeof(char));

    int name_size = strlen(name);
    int start_value_size = strlen(start_value);
    int timeactive_size = strlen(timeactive);
    int fname_size = strlen(fname);
    int fsize_size = strlen(fsize);

    // copy data
    strcpy(msg, OPA_REQ);
    strcpy(&msg[CMD_SIZE + 1], uid);
    msg[CMD_SIZE + 1 + UID_SIZE] = ' ';
    strcpy(&msg[CMD_SIZE + 1 + UID_SIZE + 1], password);
    msg[CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE] = ' ';
    strcpy(&msg[CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE + 1], name);
    msg[CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE + 1 + name_size] = ' ';
    strcpy(&msg[CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE + 1 + name_size + 1],
           start_value);
    msg[CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE + 1 + name_size + 1 +
        start_value_size] = ' ';
    strcpy(&msg[CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE + 1 + name_size + 1 +
                start_value_size + 1],
           timeactive);
    msg[CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE + 1 + name_size + 1 +
        start_value_size + 1 + timeactive_size] = ' ';
    strcpy(&msg[CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE + 1 + name_size + 1 +
                start_value_size + 1 + timeactive_size + 1],
           fname);
    msg[CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE + 1 + name_size + 1 +
        start_value_size + 1 + timeactive_size + 1 + fname_size] = ' ';
    strcpy(&msg[CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE + 1 + name_size + 1 +
                start_value_size + 1 + timeactive_size + 1 + fname_size + 1],
           fsize);
    msg[CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE + 1 + name_size + 1 +
        start_value_size + 1 + timeactive_size + 1 + fname_size + 1 +
        fsize_size] = ' ';

    return msg;
}

bool opa_res(char *response, bool print) {
    char *status = response + 4;

    if (strncmp(response, OPA_RES, 3) != 0) {
        if (print)
            printf("error: wrong server response format\n");
        return false;
    } else if (strncmp(status, STATUS_NOK, 3) == 0) {
        if (print)
            printf("failed to open auction\n");
        return false;
    }
    if (print)
        printf("opened auction with AID %s", status + 3);
    return true;
}

char *sas_req(char *aid) {
    // setup buffer
    char *msg = malloc((SAS_SIZE + 1) * sizeof(char));
    memset(msg, 0, (SAS_SIZE + 1) * sizeof(char));

    // copy data
    strcpy(msg, SAS_REQ);
    strcpy(&msg[CMD_SIZE + 1], aid);
    msg[CMD_SIZE + 1 + AID_SIZE] = '\n';

    return msg;
}

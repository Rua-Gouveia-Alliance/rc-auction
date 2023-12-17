#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdbool.h>

#define UID_SIZE 6
#define PASS_SIZE 8
#define AID_SIZE 3
#define NAME_SIZE 10
#define BID_VAL_SIZE 6
#define DUR_SIZE 5
#define FNAME_SIZE 24
#define FSIZE_SIZE 8

#define MAX_FSIZE 1024 * 1024 * 10

#define DEFAULT_SIZE 128
#define AUCTION_LIST_SIZE (6 * 1024)
#define AUCTION_START_SIZE                                                     \
    (UID_SIZE + 1 + NAME_SIZE + 1 + FNAME_SIZE + 1 + BID_VAL_SIZE + 1 +        \
     DUR_SIZE)
#define AUCTION_END_SIZE (19)
#define BID_INFO_SIZE (1 + 1 + UID_SIZE + 1 + BID_VAL_SIZE + 1 + 19 + 1 + 5)
#define BID_LIST_SIZE                                                          \
    (AUCTION_START_SIZE + 50 * BID_INFO_SIZE + AUCTION_END_SIZE)

#define CMD_SIZE 3
#define STATUS_OK_SIZE 7

#define LIN_REQ "LIN "
#define LIN_SIZE (CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE + 1)
#define LIN_RES "RLI"

#define LOU_REQ "LOU "
#define LOU_SIZE (CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE + 1)
#define LOU_RES "RLO"

#define UNR_REQ "UNR "
#define UNR_SIZE (CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE + 1)
#define UNR_RES "RUR"

#define LMA_REQ "LMA "
#define LMA_SIZE (CMD_SIZE + 1 + UID_SIZE + 1)
#define LMA_RES "RMA"

#define LMB_REQ "LMB "
#define LMB_SIZE (CMD_SIZE + 1 + UID_SIZE + 1)
#define LMB_RES "RMB"

#define LST_REQ "LST\n"
#define LST_SIZE (CMD_SIZE + 1)
#define RLS_OK "RLS OK "

#define SRC_REQ "SRC "
#define SRC_SIZE (CMD_SIZE + 1 + AID_SIZE + 1)
#define SRC_RES "RRC"

#define CLS_REQ "CLS "
#define CLS_SIZE (CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE + 1 + AID_SIZE + 1)
#define CLS_RES "RCL"

#define BID_REQ "BID "
#define BID_SIZE                                                               \
    (CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE + 1 + AID_SIZE + 1 +              \
     BID_VAL_SIZE + 1)
#define BID_RES "RBD"

#define SAS_REQ "SAS "
#define SAS_SIZE (CMD_SIZE + 1 + AID_SIZE + 1)
#define SAS_RES "RSA"

#define OPA_REQ "OPA "
#define OPA_SIZE                                                               \
    (CMD_SIZE + 1 + UID_SIZE + 1 + PASS_SIZE + 1 + NAME_SIZE + 1 +             \
     BID_VAL_SIZE + 1 + DUR_SIZE + 1 + FNAME_SIZE + 1 + FSIZE_SIZE + 1)
#define OPA_RES "ROA"

#define STATUS_OK "OK\n"
#define STATUS_NOK "NOK\n"
#define STATUS_REG "REG\n"
#define STATUS_UNR "UNR\n"
#define STATUS_NLG "NLG\n"
#define STATUS_EAU "EAU\n"
#define STATUS_EOW "EOW\n"
#define STATUS_END "END\n"
#define STATUS_ACC "ACC\n"
#define STATUS_REF "REF\n"
#define STATUS_ILG "ILG\n"

// LIN UID(6-numeric) password(8-alphanumeric)\n
char *login_req(char *uid, char *password);
bool login_res(char *response, bool print);

char *logout_req(char *uid, char *password);
bool logout_res(char *response, bool print);

char *unregister_req(char *uid, char *password);
bool unregister_res(char *response, bool print);

char *lmb_req(char *uid);
void lmb_res(char *response, bool print);

char *lma_req(char *uid);
void lma_res(char *response, bool print);

char *src_req(char *aid);
void src_res(char *response, bool print);

char *list_req();
void list_res(char *response, bool print);

char *close_req(char *uid, char *password, char *aid);
bool close_res(char *response, bool print);

char *bid_req(char *uid, char *password, char *aid, char *value);
bool bid_res(char *response, bool print);

char *sas_req(char *aid);

char *opa_req(char *uid, char *password, char *name, char *start_value,
              char *timeactive, char *fname, char *fsize);
bool opa_res(char *response, bool print);

char *fmt_aid(char *aid);

bool is_valid_uid(char *uid);
bool is_valid_aid(char *aid);
bool is_valid_password(char *password);
bool is_valid_name(char *name);
bool is_valid_fname(char *fname);
bool is_valid_timeactive(char *timeactive);
bool is_valid_value(char *value);

#endif

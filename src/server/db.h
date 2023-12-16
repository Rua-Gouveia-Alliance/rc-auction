#ifndef DB_H
#define DB_H

#include "protocol.h"
#include <stdbool.h>
#include <time.h>

#define DEFAULT_PERMS 0744

#define DATE_SIZE 10
#define TIME_SIZE 8
#define DATE_TIME_SIZE (DATE_SIZE + 1 + TIME_SIZE)
#define SECS_SIZE 10
#define INFO_SIZE                                                              \
    (UID_SIZE + 1 + NAME_SIZE + 1 + FNAME_SIZE + 1 + START_VAL_SIZE + 1 +      \
     DUR_SIZE + 1 + DATE_TIME_SIZE)
#define END_INFO_SIZE 0
#define BID_VALUE_SIZE 6
#define BID_INFO_LEN (6 + 1 + 6 + DATE_TIME_SIZE + SECS_SIZE)
#define AUCTION_STATE_SIZE (AID_SIZE + 1 + 1)

#define FMT_BID_INFO_LEN (1 + 1 + BID_INFO_LEN)
#define FMT_INFO_SIZE (INFO_SIZE)
#define FMT_END_INFO_SIZE (1 + 1 + END_INFO_SIZE)

#define DB_DIR "DB/"
#define USERS_DIR DB_DIR "USERS/"
#define PASS_SUFIX "_pass.txt"
#define LOGIN_SUFIX "_login.txt"
#define HOSTED_DIR "HOSTED/"
#define BIDDED_DIR "BIDDED/"
#define AUCTIONS_DIR DB_DIR "AUCTIONS/"
#define ASSET_DIR "ASSET/"
#define START_SUFIX "_start.txt"
#define END_SUFIX "_end.txt"
#define TXT_SUFIX ".txt"
#define BIDS_DIR "BIDS/"
#define CURRENT_BID "CURRENT_BID"

int create_defaults();

char *user_dir(char *uid);
char *user_bidded_dir(char *uid);
char *user_hosted_dir(char *uid);
bool user_registered(char *uid);
int user_register(char *uid, char *password);
int user_unregister(char *uid);
bool user_loggedin(char *uid);
int user_login(char *uid);
int user_logout(char *uid);
char *user_password(char *uid);
bool user_ok_password(char *uid, char *password);

char *auction_dir(char *aid);
char *auction_bids_dir(char *aid);
int auction_bid_count(char* aid);
bool auction_exists(char *aid);
char *auction_asset_dir(char *aid);
char *auction_asset_path(char *aid);
char *auction_start_info(char *aid);
char *auction_end_info(char *aid);
int auction_parse_info(char *aid, char *uid, char *name, char *asset_fname,
                       char *start_value, char *timeactive,
                       char *start_datetime, char *start_fulltime);
char *auction_new_info(char *uid, char *name, char *asset_fname,
                       char *start_value, char *timeactive,
                       char *start_datetime, time_t start_fulltime);
char *auction_uid(char *aid);
char *auction_name(char *aid);
char *auction_asset_fname(char *aid);
int auction_start_value(char *aid);
int auction_time_active(char *aid);
char *auction_start_datetime(char *aid);
time_t auction_start_fulltime(char *aid);
bool auction_is_owner(char *aid, char *uid);
bool auction_closed(char *aid);
int auction_close(char *aid, char *datetime, int elapsed);
int auction_close_now(char *aid);
int auction_update(char *aid);
int auction_count();
char *auction_open(char *uid, char *name, char *start_value, char *timeactive,
                   char *fname);

int make_bid(char *uid, char *aid, char *value);
int bid_value_ok(char *aid, char *value);

char *list_auctions(char *dir_path, int limit);
char *list_bids(char *aid, int limit);

char* fmt_start_info(char* aid);
char* fmt_end_info(char* aid);

#endif

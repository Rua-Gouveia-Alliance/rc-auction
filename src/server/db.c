#include "db.h"
#include "../util.h"
#include "protocol.h"
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

int create_defaults() {
    if (!path_exists(DB_DIR))
        mkdir(DB_DIR, DEFAULT_PERMS);

    if (!path_exists(USERS_DIR))
        mkdir(USERS_DIR, DEFAULT_PERMS);

    if (!path_exists(AUCTIONS_DIR))
        mkdir(AUCTIONS_DIR, DEFAULT_PERMS);

    return 0;
}

char *user_dir(char *uid) { return get_filename(USERS_DIR, uid, "/"); }

char *user_bidded_dir(char *uid) {
    char *user = user_dir(uid);
    char *bidded = get_filename(user, BIDDED_DIR, "");
    free(user);
    return bidded;
}

int add_to_bidded(char *uid, char *aid) {
    char *bidded_dir = user_bidded_dir(uid),
         *bidded_file = get_filename(bidded_dir, aid, TXT_SUFIX);
    int fd = open(bidded_file, O_CREAT, DEFAULT_PERMS);
    free(bidded_dir);
    free(bidded_file);

    if (fd == -1)
        return -1;

    close(fd);
    return 0;
}

char *user_hosted_dir(char *uid) {
    char *user = user_dir(uid);
    char *hosted = get_filename(user, HOSTED_DIR, "");
    free(user);
    return hosted;
}

bool user_registered(char *uid) {
    char *user = user_dir(uid);
    char *pass_user = get_filename(user, uid, PASS_SUFIX);

    if (path_exists(user) && path_exists(pass_user)) {
        free(user);
        free(pass_user);
        return true;
    }
    free(user);
    free(pass_user);
    return false;
}

int user_register(char *uid, char *password) {
    int fd;
    size_t n;
    char *user = user_dir(uid);
    char *pass_user = get_filename(user, uid, PASS_SUFIX);
    char *bidded_dir = user_bidded_dir(uid);
    char *hosted_dir = user_hosted_dir(uid);

    if (!path_exists(user)) {
        mkdir(user, DEFAULT_PERMS);
        mkdir(bidded_dir, DEFAULT_PERMS);
        mkdir(hosted_dir, DEFAULT_PERMS);
    }

    fd = open(pass_user, O_CREAT | O_WRONLY, DEFAULT_PERMS);
    if (fd == -1) {
        free(user);
        free(pass_user);
        free(bidded_dir);
        free(hosted_dir);
        return -1;
    }
    n = write(fd, password, strlen(password));

    free(user);
    free(pass_user);
    free(bidded_dir);
    free(hosted_dir);
    close(fd);
    return n;
}

int user_unregister(char *uid) {
    char *user = user_dir(uid);
    char *pass_user = get_filename(user, uid, PASS_SUFIX);
    int err;

    err = remove(pass_user);
    free(user);
    free(pass_user);
    return err;
}

bool user_loggedin(char *uid) {
    char *user = user_dir(uid);
    char *login_user = get_filename(user, uid, LOGIN_SUFIX);

    if (path_exists(login_user)) {
        free(user);
        free(login_user);
        return true;
    }
    free(user);
    free(login_user);
    return false;
}

int user_login(char *uid) {
    char *user = user_dir(uid);
    char *login_user = get_filename(user, uid, LOGIN_SUFIX);
    int err;

    err = open(login_user, O_CREAT | O_WRONLY, DEFAULT_PERMS);
    free(user);
    free(login_user);
    if (err != -1)
        close(err);
    return err;
}

int user_logout(char *uid) {
    char *user = user_dir(uid);
    char *login_user = get_filename(user, uid, LOGIN_SUFIX);
    int err;

    err = remove(login_user);
    free(user);
    free(login_user);
    return err;
}

char *user_password(char *uid) {
    char *user = user_dir(uid);
    char *pass_user = get_filename(user, uid, PASS_SUFIX);
    char *pass = malloc((PASS_SIZE + 1) * sizeof(char));
    memset(pass, 0, (PASS_SIZE + 1));
    int fd, n;

    fd = open(pass_user, O_RDONLY);
    if (fd == -1) {
        free(user);
        free(pass_user);
        free(pass);
        return NULL;
    }

    n = read(fd, pass, PASS_SIZE);
    if (n == -1) {
        free(user);
        free(pass_user);
        free(pass);
        close(fd);
        return NULL;
    }
    free(user);
    free(pass_user);
    close(fd);
    return pass;
}

bool user_ok_password(char *uid, char *password) {
    char *right_pass = user_password(uid);
    if (right_pass == NULL)
        return false;

    bool result = (strcmp(right_pass, password) == 0);
    free(right_pass);
    return result;
}

int set_current_bid(char *aid, char *value) {
    ssize_t n;
    char *dir = auction_dir(aid);
    char *bid_filename = get_filename(dir, CURRENT_BID, TXT_SUFIX);
    int fd = open(bid_filename, O_CREAT | O_WRONLY, DEFAULT_PERMS);
    if (fd == -1) {
        free(bid_filename);
        free(dir);
        return -1;
    }

    n = write(fd, value, BID_VALUE_SIZE);

    free(bid_filename);
    free(dir);
    close(fd);

    if (n == -1)
        return -1;

    return 1;
}

char *get_auction_state(char *aid) {
    char *auction_state =
        (char *)malloc(sizeof(char) * (AUCTION_STATE_SIZE + 1));
    int active = !auction_closed(aid);

    sprintf(auction_state, "%s %d", aid, active);

    return auction_state;
}

char *list_auctions(char *dir_path, int limit) {
    DIR *dir;
    struct dirent *entry;
    char *auctions, *auction_state;
    int i, auction_count;

    // Open the directory
    dir = opendir(dir_path);
    if (dir == NULL) {
        fprintf(stderr, "Error opening directory %s\n", dir_path);
        return NULL;
    }

    // Read directory entries
    i = 0;
    auction_count = count_entries(dir_path, DT_DIR);
    auction_count += count_entries(dir_path, DT_REG);
    if (auction_count == -1) {
        closedir(dir);
        return NULL;
    }

    auctions =
        malloc((auction_count * (AUCTION_STATE_SIZE + 1) + 1) * sizeof(char));
    memset(auctions, 0,
           (auction_count * (AUCTION_STATE_SIZE + 1) + 1) * sizeof(char));

    // TODO: ordenar
    while (((entry = readdir(dir)) != NULL)) {
        if (i > limit)
            break;

        if (strcmp(entry->d_name, "..") != 0 &&
            strcmp(entry->d_name, ".") != 0) {

            if (entry->d_type == DT_REG) {
                char *aid = remove_extension(entry->d_name);
                auction_state = get_auction_state(aid);
                free(aid);
            } else if (entry->d_type == DT_DIR) {
                auction_state = get_auction_state(entry->d_name);
            }

            strcpy(auctions + i * (AUCTION_STATE_SIZE + 1), auction_state);
            (auctions + i * (AUCTION_STATE_SIZE + 1))[AUCTION_STATE_SIZE] = ' ';
            i++;
            free(auction_state);
        }
    }

    auctions[auction_count * (AUCTION_STATE_SIZE + 1) - 1] = '\n';
    closedir(dir);
    return auctions;
}

char *auction_dir(char *aid) { return get_filename(AUCTIONS_DIR, aid, "/"); }

char *auction_bids_dir(char *aid) {
    char *auc = auction_dir(aid);
    char *bids = get_filename(auc, BIDS_DIR, "");
    free(auc);
    return bids;
}

bool auction_exists(char *aid) {
    char *auction = auction_dir(aid);

    if (path_exists(auction)) {
        free(auction);
        return true;
    }
    free(auction);
    return false;
}

char *auction_asset_dir(char *aid) {
    char *auction = auction_dir(aid);
    char *asset = get_filename(auction, ASSET_DIR, "");
    free(auction);
    return asset;
}

char *auction_asset_path(char *aid) {
    char *asset = auction_asset_dir(aid);
    char *path;
    DIR *dir;
    struct dirent *entry;

    // Open the directory
    dir = opendir(asset);
    if (dir == NULL) {
        fprintf(stderr, "Error opening directory %s\n", asset);
        return NULL;
    }

    entry = readdir(dir);
    if (entry == NULL)
        return NULL;

    while (((entry = readdir(dir)) != NULL)) {
        if (strcmp(entry->d_name, "..") != 0 &&
            strcmp(entry->d_name, ".") != 0) {
            path = get_filename(asset, entry->d_name, "");
            break;
        }
    }

    closedir(dir);
    free(asset);
    return path;
}

char *auction_info(char *aid) {
    char *auction = auction_dir(aid);
    char *start = get_filename(auction, aid, START_SUFIX);
    char *info = malloc((INFO_SIZE + 1) * sizeof(char));
    memset(info, 0, INFO_SIZE + 1);
    int fd, n;

    fd = open(start, O_RDONLY);
    if (fd == -1) {
        free(auction);
        free(start);
        free(info);
        return NULL;
    }

    n = read(fd, info, INFO_SIZE + 1);
    if (n == -1) {
        free(auction);
        free(start);
        free(info);
        close(fd);
        return NULL;
    }
    close(fd);
    return info;
}

int auction_parse_info(char *aid, char *uid, char *name, char *asset_fname,
                       char *start_value, char *timeactive,
                       char *start_datetime, char *start_fulltime) {
    ssize_t n;
    char *token, *info;
    char *pointers[] = {uid,        name,           asset_fname,   start_value,
                        timeactive, start_datetime, start_fulltime};

    info = auction_info(aid);
    token = strtok(info, " ");
    for (int i = 0; token != NULL; i++) {
        if (pointers[i] != NULL)
            strcpy(pointers[i], token);

        // datetime case
        if (i == 5) {
            token = strtok(NULL, " ");
            if (token != NULL) {
                if (pointers[i] != NULL) {
                    pointers[i][DATE_SIZE] = ' ';
                    strcpy(pointers[i] + DATE_SIZE + 1, token);
                }
            }
        }

        token = strtok(NULL, " ");
    }

    return 0;
}

char *auction_uid(char *aid) {
    char *uid = malloc((UID_SIZE + 1) * sizeof(char));
    memset(uid, 0, UID_SIZE + 1);
    auction_parse_info(aid, uid, NULL, NULL, NULL, NULL, NULL, NULL);
    uid[UID_SIZE] = '\0';
    return uid;
}

char *auction_name(char *aid) {
    char *name = malloc((NAME_SIZE + 1) * sizeof(char));
    auction_parse_info(aid, NULL, name, NULL, NULL, NULL, NULL, NULL);
    name[NAME_SIZE] = '\0';
    return name;
}

char *auction_asset_fname(char *aid) {
    char *fname = malloc((FNAME_SIZE + 1) * sizeof(char));
    auction_parse_info(aid, NULL, NULL, fname, NULL, NULL, NULL, NULL);
    fname[FNAME_SIZE] = '\0';
    return fname;
}

int auction_start_value(char *aid) {
    int start_value;
    char *value = malloc((START_VAL_SIZE + 1) * sizeof(char));
    auction_parse_info(aid, NULL, NULL, NULL, value, NULL, NULL, NULL);
    value[START_VAL_SIZE] = '\0';

    // TODO: atoi can throw error
    start_value = atoi(value);
    free(value);
    return start_value;
}

int auction_time_active(char *aid) {
    int active_value;
    char *value = malloc((DUR_SIZE + 1) * sizeof(char));
    auction_parse_info(aid, NULL, NULL, NULL, NULL, value, NULL, NULL);
    value[DUR_SIZE] = '\0';

    // TODO: atoi can throw error
    active_value = atoi(value);
    free(value);
    return active_value;
}

char *auction_start_datetime(char *aid) {
    char *datetime = malloc((DATE_TIME_SIZE + 1) * sizeof(char));
    auction_parse_info(aid, NULL, NULL, NULL, NULL, NULL, datetime, NULL);
    datetime[DATE_TIME_SIZE] = '\0';
    return datetime;
}

time_t auction_start_fulltime(char *aid) {
    time_t fulltime_value;
    char *fulltime = malloc((SECS_SIZE + 1) * sizeof(char));
    auction_parse_info(aid, NULL, NULL, NULL, NULL, NULL, NULL, fulltime);
    fulltime[DUR_SIZE] = '\0';

    // TODO: atoi can throw error
    fulltime_value = strtol(fulltime, NULL, 0);
    free(fulltime);
    return fulltime_value;
}

bool auction_is_owner(char *aid, char *uid) {
    char *right_uid = auction_uid(aid);
    if (right_uid == NULL)
        return false;

    bool result = (strcmp(right_uid, uid) == 0);
    free(right_uid);
    return result;
}

bool auction_closed(char *aid) {
    auction_update(aid);
    char *auction = auction_dir(aid);
    char *end = get_filename(auction, aid, END_SUFIX);
    if (path_exists(end)) {
        free(auction);
        free(end);
        return true;
    }
    free(auction);
    free(end);
    return false;
}

int auction_close(char *aid, char *datetime, int elapsed) {
    char *auction = auction_dir(aid);
    char *end = get_filename(auction, aid, END_SUFIX);
    int fd, written = 0;
    char elapsed_str[10];
    snprintf(elapsed_str, 10, "%d", elapsed);

    fd = open(end, O_CREAT | O_RDWR, DEFAULT_PERMS);
    if (fd == -1)
        return -1;

    written += write(fd, datetime, DATE_TIME_SIZE);
    written += write(fd, " ", 1);
    written += write(fd, elapsed_str, strlen(elapsed_str));

    free(auction);
    free(end);
    close(fd);
    return written;
}

int auction_close_now(char *aid) {
    int err;
    time_t now, start_time;
    char *str_final_time;

    now = time(NULL);
    str_final_time = time_to_str(now);
    start_time = auction_start_fulltime(aid);

    err = auction_close(aid, str_final_time, now - start_time);

    free(str_final_time);
    return err;
}

int auction_update(char *aid) {
    int err;
    time_t curr_time, start_time, time_active;
    char *str_final_time;

    curr_time = time(NULL);
    start_time = auction_start_fulltime(aid);
    if (curr_time - start_time < 0) {
        time_active = auction_time_active(aid);
        str_final_time = time_to_str(start_time + time_active);
        err = auction_close(aid, str_final_time, time_active);
        free(str_final_time);
        return err;
    }
    return 0;
}

char *auction_new_info(char *uid, char *name, char *asset_fname,
                       char *start_value, char *timeactive,
                       char *start_datetime, time_t start_fulltime) {

    char start_fulltime_str[20];
    snprintf(start_fulltime_str, 20, "%ld", start_fulltime);
    char *info = malloc((INFO_SIZE + 1) * sizeof(char));
    memset(info, 0, (INFO_SIZE + 1) * sizeof(char));

    sprintf(info, "%s %s %s %s %s %s %ld", uid, name, asset_fname, start_value,
            timeactive, start_datetime, start_fulltime);
    return info;
}

int auction_count() { return count_entries(AUCTIONS_DIR, DT_DIR); }

int add_to_hosted(char *uid, char *aid) {
    char *hosted_dir = user_hosted_dir(uid),
         *hosted_file = get_filename(hosted_dir, aid, TXT_SUFIX);
    int fd = open(hosted_file, O_CREAT, DEFAULT_PERMS);

    free(hosted_dir);
    free(hosted_file);
    if (fd == -1)
        return -1;

    close(fd);
    return 0;
}

char *auction_open(char *uid, char *name, char *start_value, char *timeactive,
                   char *fname) {
    int fd, auction_i;
    size_t n;

    auction_i = auction_count() + 1;
    if (auction_i > 999 || auction_i < 1)
        return NULL;

    char *aid = i_to_aid(auction_i);
    char *auction = auction_dir(aid);

    if (path_exists(auction)) {
        free(aid);
        free(auction);
        return NULL;
    }
    char *bids = auction_bids_dir(aid);
    char *asset = auction_asset_dir(aid);

    mkdir(auction, DEFAULT_PERMS);
    mkdir(bids, DEFAULT_PERMS);
    mkdir(asset, DEFAULT_PERMS);

    char *start = get_filename(auction, aid, START_SUFIX);
    fd = open(start, O_CREAT | O_WRONLY, DEFAULT_PERMS);
    if (fd == -1) {
        remove(auction);
        remove(bids);
        free(aid);
        free(auction);
        free(bids);
        free(asset);
        free(start);
        return NULL;
    }

    time_t now = time(NULL);
    char *start_datetime = time_to_str(now);
    char *info = auction_new_info(uid, name, fname, start_value, timeactive,
                                  start_datetime, now);
    n = write(fd, info, strlen(info));
    if (n == -1) {
        remove(start);
        remove(auction);
        remove(bids);
        free(aid);
        free(auction);
        free(bids);
        free(asset);
        free(start);
        free(start_datetime);
        free(info);
        close(fd);
        return NULL;
    }

    n = add_to_hosted(uid, aid);
    if (n == -1) {
        remove(start);
        remove(auction);
        remove(bids);
        free(aid);
        free(auction);
        free(bids);
        free(asset);
        free(start);
        free(start_datetime);
        free(info);
        close(fd);
        return NULL;
    }

    n = set_current_bid(aid, start_value);
    if (n == -1) {
        remove(start);
        remove(auction);
        remove(bids);
        free(aid);
        free(auction);
        free(bids);
        free(asset);
        free(start);
        free(start_datetime);
        free(info);
        close(fd);
        return NULL;
    }

    free(auction);
    free(bids);
    free(asset);
    free(start);
    free(start_datetime);
    free(info);
    close(fd);
    return aid;
}

int get_current_bid(char *aid) {
    int fd;
    ssize_t n;
    char *dir = auction_dir(aid);
    char *file = get_filename(dir, CURRENT_BID, TXT_SUFIX);
    char *value = (char *)malloc(sizeof(char) * (BID_VALUE_SIZE + 1));

    fd = open(file, O_RDONLY);
    free(dir);
    free(file);

    if (fd == -1) {
        free(value);
        return -1;
    }

    n = read(fd, value, BID_VALUE_SIZE);
    close(fd);

    if (n == -1) {
        free(value);
        return -1;
    }

    value[BID_VALUE_SIZE] = '\0';
    int parsed_value = atoi(value);
    free(value);

    if (parsed_value == 0)
        return -1;

    return parsed_value;
}

int bid_value_ok(char *aid, char *value) {
    int current_value = get_current_bid(aid);
    int bid_value = atoi(value);

    if (current_value == 0 || bid_value == 0)
        return -1;

    return bid_value > current_value;
}

char *create_bid_info(char *uid, char *value) {
    char *bid_info = (char *)malloc(sizeof(char) * (BID_INFO_LEN + 1));
    char *formatted_time = time_to_str(time(NULL));

    sprintf(bid_info, "%s %s %s", uid, value, formatted_time);

    free(formatted_time);

    return bid_info;
}

int add_to_bids(char *aid, char *uid, char *value) {
    int fd;
    ssize_t n;
    char *dir = auction_bids_dir(aid);
    char *file = get_filename(dir, value, TXT_SUFIX);
    free(dir);

    fd = open(file, O_CREAT | O_WRONLY, DEFAULT_PERMS);
    free(file);

    if (fd == -1)
        return -1;

    char *bid_info = create_bid_info(uid, value);
    n = write(fd, bid_info, BID_INFO_LEN);
    free(bid_info);
    close(fd);

    if (n == -1)
        return -1;

    return 0;
}

int make_bid(char *uid, char *aid, char *value) {
    int n;

    n = add_to_bids(aid, uid, value);
    if (n == -1)
        return -1;

    n = add_to_bidded(uid, aid);
    if (n == -1)
        return -1;

    n = set_current_bid(aid, value);
    if (n == -1)
        return -1;

    return 0;
}

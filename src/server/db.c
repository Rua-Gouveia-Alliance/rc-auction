#include "db.h"
#include "../util.h"
#include "protocol.h"
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
        return NULL;
    }
    free(user);
    free(pass_user);
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

char *auction_dir(char *aid) { return get_filename(AUCTIONS_DIR, aid, "/"); }

bool auction_exists(char *aid) {
    char *auction = auction_dir(aid);

    if (path_exists(auction)) {
        free(auction);
        return true;
    }
    free(auction);
    return false;
}

char *auction_info(char *aid) {
    char *auction = auction_dir(aid);
    char *start = get_filename(auction, aid, START_SUFIX);
    char *info = malloc((INFO_SIZE + 1) * sizeof(char));
    int fd, n;

    fd = open(start, INFO_SIZE + 1);
    if (fd == -1) {
        free(auction);
        free(start);
        free(info);
        return NULL;
    }

    n = read(fd, start, INFO_SIZE + 1);
    if (n == -1) {
        free(auction);
        free(start);
        free(info);
        return NULL;
    }
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
        if (pointers[i] != NULL) {
            strcpy(pointers[i], token);

            // datetime case
            if (i == 5) {
                token = strtok(NULL, " ");
                if (token != NULL) {
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
        return err;
    }
    return 0;
}

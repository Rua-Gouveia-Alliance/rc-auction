#include "db.h"
#include "../util.h"
#include "protocol.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int create_defaults() {
    struct stat st = {0};
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

bool auction_exits(char *aid) {
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
    char *info = malloc((INFO_SIZE) * sizeof(char));
    int fd, n;

    fd = open(start, INFO_SIZE);
    if (fd == -1) {
        free(auction);
        free(start);
        free(info);
        return NULL;
    }

    n = read(fd, start, INFO_SIZE);
    if (n == -1) {
        free(auction);
        free(start);
        free(info);
        return NULL;
    }
    return info;
}

char *auction_uid(char *aid) {
    char *info = auction_info(aid);
    if (info == NULL)
        return NULL;

    char *uid = malloc((UID_SIZE + 1) * sizeof(char));
    strncpy(uid, info, UID_SIZE);

    free(info);
    return uid;
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

int auction_close(char *aid) {
    char *auction = auction_dir(aid);
    char *end = get_filename(auction, aid, END_SUFIX);
    int err;

    err = open(end, O_CREAT | O_WRONLY, DEFAULT_PERMS);
    free(auction);
    free(end);
    return err;
}

#include "protocol.h"

#ifndef SERVER_H
#define SERVER_H

#define AS_DIR "ASDIR"

#define USERS_DIR_SIZE 12
#define USERS_DIR "ASDIR/USERS/"
#define HOSTED_DIR "HOSTED"
#define BIDDED_DIR "BIDDED"

#define AUCTIONS_DIR "ASDIR/AUCTIONS"

#define PASS_FILENAME_EXT_SIZE 9
#define PASS_FILENAME_EXT "_pass.txt"
#define LOGGED_IN_FILENAME_EXT_SIZE 10
#define LOGGED_IN_FILENAME_EXT "_login.txt"

#define PASS_FILENAME_SIZE (USERS_DIR_SIZE + UID_SIZE + PASS_FILENAME_EXT_SIZE)
#define LOGGED_IN_FILENAME_SIZE                                                \
    (USERS_DIR_SIZE + UID_SIZE + LOGGED_IN_FILENAME_EXT_SIZE)

#endif

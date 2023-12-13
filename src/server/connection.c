#include "connection.h"

#include <stdio.h>

static TCPInfo tcp_conns[MAX_TCP_COUNT];

int setup_tcp(char *port) {
    // TODO
    return -1;
}

int setup_udp(char *port) {
    // TODO
    return -1;
}

int accept_new_tcp(int tcp_main) {
    // TODO
    return false;
}

char *handle_udp(int udp_sock) {
    // TODO
    return NULL;
}

char *handle_tcp(int tcp_sock) {
    // TODO
    return NULL;
}

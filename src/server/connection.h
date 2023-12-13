#ifndef CONNECTION_H
#define CONNECTION_H

#include <stdbool.h>

#define MAX_TCP_COUNT 100

typedef struct {
    int socket_id;
    char *buffer;
} TCPInfo;

int setup_tcp(char *port);

int setup_udp(char *port);

int accept_new_tcp(int tcp_main);

char *handle_udp(int udp_sock);

char *handle_tcp(int tcp_sock);

#endif

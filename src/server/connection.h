#ifndef CONNECTION_H
#define CONNECTION_H

#include <stdbool.h>

#define MAX_TCP_COUNT 100
#define DEFAU

typedef struct {
    int socket_id;
    char *buffer;
    int buffer_i;
} TCPInfo;

int setup_tcp(char *port);

int setup_udp(char *port);

int accept_new_tcp(int tcp_main);

char *receive_udp(int udp_sock);

char *receive_tcp(int tcp_sock);

#endif

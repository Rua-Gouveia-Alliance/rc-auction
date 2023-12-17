#ifndef CONNECTION_H
#define CONNECTION_H

#include <netinet/in.h>
#include <stdbool.h>

#define MAX_TCP_COUNT 100

typedef struct {
    int socket_id;
    char *buffer;
    int buffer_i;
    bool in_file;
    long int remaining_size;
    char *fname;
    int excess;
    char *final_response;
} TCPInfo;

int setup_tcp(char *port);

int setup_udp(char *port);

int accept_new_tcp(int tcp_main, bool verbose);

char *receive_udp(int udp_sock, bool verbose);

char *receive_tcp(int tcp_sock, bool verbose);

void send_udp(int udp_sock, char *msg, struct sockaddr_in *addr, bool verbose);

void send_tcp(int tcp_sock, char *msg, bool verbose);

void send_tcp_file(int tcp_sock, char *msg, char *path, bool verbose);

bool prepare_freceive(int tcp_sock, bool keep, char *response, bool verbose);

#endif

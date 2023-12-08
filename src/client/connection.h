#include <stdbool.h>

#ifndef CONNECTION_H
#define CONNECTION_H

#define PACKET_SIZE 8

char *use_udp(char *ip_addr, char *port, char *msg, int msg_size, int receive_size);

char *use_tcp(char *ip_addr, char *port, char *msg, int msg_size, int receive_size);

bool transfer_file(char *ip_addr, char *port, char *msg, int msg_size);

#endif
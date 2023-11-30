#ifndef CONNECTION_H
#define CONNECTION_H

char *use_udp(char *ip_addr, char *port, char *msg, int size, int receive_size);

char *use_tcp(char *ip_addr, char *port, char *msg, int size);

#endif
#ifndef CONNECTION_H
#define CONNECTION_H

int setup_tcp(char *port);

int setup_udp(char *port);

bool accept_new_tcp(int tcp_main);

void handle_udp(int udp_sock);

bool handle_tcp(int tcp_sock);

#endif

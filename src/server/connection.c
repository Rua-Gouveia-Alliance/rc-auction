#include "connection.h"
#include "protocol.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

TCPInfo tcp_conns[MAX_TCP_COUNT] = {[0 ... MAX_TCP_COUNT - 1] = {-1, NULL, 0}};

static struct sockaddr_in last_addr;

int new_tcpconn(int fd) {
    for (int i = 0; i < MAX_TCP_COUNT; i++) {
        if (tcp_conns[i].socket_id == -1) {
            tcp_conns[i].socket_id = fd;
            tcp_conns[i].buffer = malloc(DEFAULT_SIZE);
            return i;
        }
    }
    return -1;
}

int rm_tcpconn(int fd) {
    for (int i = 0; i < MAX_TCP_COUNT; i++) {
        if (fd == tcp_conns[i].socket_id) {
            tcp_conns[i].socket_id = -1;
            free(tcp_conns[i].buffer);
            tcp_conns[i].buffer = NULL;
            tcp_conns[i].buffer_i = 0;

            return 0;
        }
    }
    return -1;
}

TCPInfo *get_tcpinfo(int fd) {
    int tcpconn;
    for (int i = 0; i < MAX_TCP_COUNT; i++) {
        if (fd == tcp_conns[i].socket_id)
            return tcp_conns + i;
    }

    tcpconn = new_tcpconn(fd);
    if (tcpconn != -1)
        return tcp_conns + new_tcpconn(fd);
    return NULL;
}

int setup_tcp(char *port) {
    int fd, errcode;
    ssize_t n;
    struct addrinfo hints, *res;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
        exit(1);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    errcode = getaddrinfo(NULL, port, &hints, &res);
    if ((errcode) != 0)
        exit(1);

    n = bind(fd, res->ai_addr, res->ai_addrlen);
    if (n == -1)
        exit(1);

    if (listen(fd, 5) == -1)
        exit(1);

    return fd;
}

int setup_udp(char *port) {
    int fd, errcode;
    ssize_t n;
    struct addrinfo hints, *res;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1)
        exit(1);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    errcode = getaddrinfo(NULL, port, &hints, &res);
    if (errcode != 0)
        exit(1);

    n = bind(fd, res->ai_addr, res->ai_addrlen);
    if (n == -1)
        exit(1);

    return fd;
}

int accept_new_tcp(int tcp_main) {
    TCPInfo *tcp_info;
    int newfd;
    struct sockaddr_in addr;
    socklen_t addrlen;

    if ((newfd = accept(tcp_main, (struct sockaddr *)&addr, &addrlen)) == -1) {
        printf("error: tcp connection not accepted\n");
        exit(1);
    }

    tcp_info = get_tcpinfo(tcp_main);
    if (tcp_info == NULL) {
        printf("error: too many clients\n");
        return -1;
    }
    return newfd;
}

char *receive_udp(int udp_sock) {
    ssize_t n;
    socklen_t addrlen;
    char *buffer = malloc(DEFAULT_SIZE * sizeof(char));

    addrlen = sizeof(last_addr);
    n = recvfrom(udp_sock, buffer, DEFAULT_SIZE, 0,
                 (struct sockaddr *)&last_addr, &addrlen);
    if (n == -1) {
        printf("error: udp receive error\n");
        exit(1);
    }

    return buffer;
}

char *receive_tcp(int tcp_sock) {
    TCPInfo *tcp_info;
    ssize_t n;
    socklen_t addrlen;
    struct sockaddr_in addr;

    tcp_info = get_tcpinfo(tcp_sock);
    addrlen = sizeof(addr);
    n = read(tcp_sock, tcp_info->buffer + tcp_info->buffer_i,
             DEFAULT_SIZE - tcp_info->buffer_i);
    if (n == -1) {
        printf("error: tcp read error");
        exit(1);
    }

    tcp_info->buffer_i += n;
    if (tcp_info->buffer[tcp_info->buffer_i - 1] == '\n')
        return tcp_info->buffer;

    return NULL;
}

void send_udp(int udp_sock, char *msg, struct sockaddr_in *addr) {
    socklen_t addrlen;
    ssize_t n;

    // if addr is NULL, use last received UDP addr
    if (addr != NULL)
        last_addr = *addr;

    addrlen = sizeof(last_addr);
    n = sendto(udp_sock, msg, strlen(msg), 0, (struct sockaddr *)&last_addr,
               addrlen);
    if (n == -1) {
        printf("error: failed to send udp response");
        exit(1);
    }
}

void send_tcp(int tcp_sock, char *msg) {
    ssize_t n;

    n = read(tcp_sock, msg, strlen(msg));
    if (n == -1) {
        printf("error: failed to send tcp response\n");
        exit(1);
    }

    close(tcp_sock);
    rm_tcpconn(tcp_sock);
}

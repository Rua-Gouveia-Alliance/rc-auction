#include "connection.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// TODO: test connection protocols
char *use_udp(char *ip_addr, char *port, char *msg, int size) {
    int fd, errcode;
    ssize_t n;
    socklen_t addrlen;
    struct sockaddr_in addr;
    struct addrinfo hints, *res;
    char *buffer = (char *)malloc(128 * sizeof(char));

    // setup socket
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1)
        exit(1);

    // setup hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    // dns
    errcode = getaddrinfo(ip_addr, port, &hints, &res);
    if (errcode != 0)
        exit(1);

    // send
    n = sendto(fd, msg, size, 0, res->ai_addr, res->ai_addrlen);
    if (n == -1)
        exit(1);

    // receive
    addrlen = sizeof(addr);
    n = recvfrom(fd, buffer, 128, 0, (struct sockaddr *)&addr, &addrlen);
    if (n == -1)
        exit(1);

    freeaddrinfo(res);
    close(fd);
    return buffer;
}

char *use_tcp(char *ip_addr, char *port, char *msg, int size) {
    int fd, errcode;
    ssize_t n;
    socklen_t addrlen;
    struct sockaddr_in addr;
    struct addrinfo hints, *res;
    char *buffer = (char *)malloc(128 * sizeof(char));

    fd = socket(AF_INET, SOCK_STREAM, 0); // TCP socket
    if (fd == -1)
        exit(1); // error

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP socket

    errcode = getaddrinfo(ip_addr, port, &hints, &res);
    if (errcode != 0) /*error*/
        exit(1);

    n = connect(fd, res->ai_addr, res->ai_addrlen);
    if (n == -1) /*error*/
        exit(1);

    n = write(fd, msg, size);
    if (n == -1) /*error*/
        exit(1);

    n = read(fd, buffer, 128);
    if (n == -1) /*error*/
        exit(1);

    freeaddrinfo(res);
    close(fd);

    return buffer;
}

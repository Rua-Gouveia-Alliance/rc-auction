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
#include <fcntl.h>

// TODO: test connection protocols
char *use_udp(char *ip_addr, char *port, char *msg, int msg_size,
              int receive_size) {
    int fd, errcode;
    ssize_t n;
    socklen_t addrlen;
    struct sockaddr_in addr;
    struct addrinfo hints, *res;
    char *buffer = (char *)malloc((receive_size + 1) * sizeof(char));

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
    n = sendto(fd, msg, msg_size, 0, res->ai_addr, res->ai_addrlen);
    if (n == -1)
        exit(1);

    // receive
    addrlen = sizeof(addr);
    n = recvfrom(fd, buffer, receive_size, 0, (struct sockaddr *)&addr,
                 &addrlen);
    if (n == -1)
        exit(1);
    buffer[n] = '\0';

    freeaddrinfo(res);
    close(fd);
    return buffer;
}

char *use_tcp(char *ip_addr, char *port, char *msg, int msg_size, int receive_size) {
    int fd, errcode;
    ssize_t n;
    socklen_t addrlen;
    struct sockaddr_in addr;
    struct addrinfo hints, *res;
    char *buffer = (char *)malloc((receive_size + 1) * sizeof(char));

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

    n = write(fd, msg, msg_size);
    if (n == -1) /*error*/
        exit(1);

    n = read(fd, buffer, receive_size);
    if (n == -1) /*error*/
        exit(1);

    freeaddrinfo(res);
    close(fd);

    return buffer;
}

void read_word(int fd, char* buffer) {
    char c;
    int i = 0;
    ssize_t n;

    read(fd, &c, sizeof(char));
    while (c != ' ') {
        buffer[i++] = c;
        n = read(fd, &c, sizeof(char));
        if (n == -1)
           exit(EXIT_FAILURE);
    }
}

bool transfer_file(char *ip_addr, char *port, char *msg, int msg_size) {
    int fd, errcode, read_bytes = 0;
    ssize_t n;
    socklen_t addrlen;
    struct sockaddr_in addr;
    struct addrinfo hints, *res;
    char *status = (char *)malloc(7 * sizeof(char));
    char *file_name = (char *)malloc(25 * sizeof(char));
    char *file_size = (char *)malloc(9 * sizeof(char));
    char *buffer = (char *)malloc(PACKET_SIZE);

    fd = socket(AF_INET, SOCK_STREAM, 0); // TCP socket
    if (fd == -1)
        exit(EXIT_FAILURE); // error

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP socket

    errcode = getaddrinfo(ip_addr, port, &hints, &res);
    if (errcode != 0) /*error*/
        exit(EXIT_FAILURE);

    n = connect(fd, res->ai_addr, res->ai_addrlen);
    if (n == -1) /*error*/
        exit(EXIT_FAILURE);

    n = write(fd, msg, msg_size);
    if (n == -1) /*error*/
        exit(EXIT_FAILURE);

    n = read(fd, status, 7);
    if (n == -1) /*error*/
        exit(EXIT_FAILURE);
    
    status[6] = '\0';

    if (strcmp(status + 4, "OK") != 0) {
        free(status);
        free(file_name);
        free(file_size);
        free(buffer);

        freeaddrinfo(res);
        close(fd);
        return false;
    }

    read_word(fd, file_name);
    read_word(fd, file_size);
    
    int file_bytes = atoi(file_size);
    int file = open(file_name, O_CREAT | O_WRONLY, 0666);
    
    while (read_bytes < file_bytes) {
        n = read(fd, buffer, PACKET_SIZE);
        if (n == -1) /*error*/
            exit(EXIT_FAILURE);

        n = write(file, buffer, PACKET_SIZE);
        if (n == -1) /*error*/
            exit(EXIT_FAILURE);

        read_bytes += PACKET_SIZE;
    }

    free(status);
    free(file_name);
    free(file_size);
    free(buffer);

    freeaddrinfo(res);
    close(fd);
    close(file);

    return true;
}

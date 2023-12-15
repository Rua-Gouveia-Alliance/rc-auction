#include "connection.h"
#include "db.h"
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

TCPInfo tcp_conns[MAX_TCP_COUNT] = {
    [0 ... MAX_TCP_COUNT - 1] = {-1, NULL, 0, false, 0, NULL, 0}};

static struct sockaddr_in last_addr;

int new_tcpconn(int fd) {
    for (int i = 0; i < MAX_TCP_COUNT; i++) {
        if (tcp_conns[i].socket_id == -1) {
            tcp_conns[i].socket_id = fd;

            tcp_conns[i].buffer = malloc((DEFAULT_SIZE + 1) * sizeof(char));
            memset(tcp_conns[i].buffer, 0, (DEFAULT_SIZE + 1) * sizeof(char));

            tcp_conns[i].fname = malloc(DEFAULT_SIZE * sizeof(char));
            memset(tcp_conns[i].fname, 0, DEFAULT_SIZE * sizeof(char));

            return i;
        }
    }
    return -1;
}

int rm_tcpconn(int fd) {
    for (int i = 0; i < MAX_TCP_COUNT; i++) {
        if (fd == tcp_conns[i].socket_id) {
            tcp_conns[i].socket_id = -1;

            tcp_conns[i].buffer = NULL;

            tcp_conns[i].buffer_i = 0;

            tcp_conns[i].in_file = false;

            tcp_conns[i].remaining_size = 0;

            free(tcp_conns[i].fname);
            tcp_conns[i].fname = NULL;

            tcp_conns[i].excess = 0;

            free(tcp_conns[i].final_response);
            tcp_conns[i].final_response = NULL;

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
        return tcp_conns + tcpconn;
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
    memset(buffer, 0, DEFAULT_SIZE * sizeof(char));

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
    char *str, *path;
    TCPInfo *tcp_info;
    ssize_t n;
    int file;
    socklen_t addrlen;
    struct sockaddr_in addr;

    tcp_info = get_tcpinfo(tcp_sock);
    addrlen = sizeof(addr);
    if (tcp_info->in_file) {
        path = fpath_from_roa(tcp_info->final_response, tcp_info->fname);
        int file = open(path, O_CREAT | O_WRONLY | O_APPEND, DEFAULT_PERMS);
        free(path);
        if (file == -1) {
            printf("error: create file failed\n");
            exit(1);
        }

        n = read(tcp_sock, tcp_info->buffer, DEFAULT_SIZE);
        if (n == -1) {
            printf("error: tcp read error");
            exit(1);
        }

        n = write(file, tcp_info->buffer, n);
        if (n == -1) {
            printf("error: write to file error\n");
            exit(1);
        }

        tcp_info->remaining_size -= n;
        if (tcp_info->remaining_size < 1) {
            if (tcp_info->remaining_size == -1) {
                // remove \n from file
                long new_pos = lseek(file, -1, SEEK_END);
                ftruncate(file, new_pos);
            }

            close(file);
            send_tcp(tcp_sock, tcp_info->final_response);
            rm_tcpconn(tcp_sock);
            str = malloc(2 * sizeof(char));
            str[0] = '\n';
            str[1] = '\0';
            return str;
        }
        close(file);
        return NULL;
    }

    n = read(tcp_sock, tcp_info->buffer + tcp_info->buffer_i,
             DEFAULT_SIZE - tcp_info->buffer_i);
    if (n == -1) {
        printf("error: tcp read error");
        exit(1);
    }

    // OPA case
    int i;
    char fname[FNAME_SIZE + 1];
    char fsize[FSIZE_SIZE + 1];
    if ((i = parse_opa(tcp_info->buffer, NULL, NULL, NULL, NULL, NULL, fname,
                       fsize)) != -1) {
        tcp_info->in_file = true;
        strcpy(tcp_info->fname, fname);
        tcp_info->remaining_size = strtol(fsize, NULL, 0);
        tcp_info->excess = n - i;
        tcp_info->buffer_i = i;
        return tcp_info->buffer;
    }

    // normal case
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

    n = write(tcp_sock, msg, strlen(msg));
    if (n == -1) {
        printf("error: failed to send tcp response\n");
        exit(1);
    }

    close(tcp_sock);
    rm_tcpconn(tcp_sock);
}

bool prepare_freceive(int tcp_sock, bool keep, char *response) {
    char *path, *str;
    size_t n;

    if (!keep) {
        send_tcp(tcp_sock, response);
        rm_tcpconn(tcp_sock);
        return true;
    }

    TCPInfo *tcp_info = get_tcpinfo(tcp_sock);
    tcp_info->final_response = response;

    path = fpath_from_roa(tcp_info->final_response, tcp_info->fname);
    int file = open(path, O_CREAT | O_WRONLY | O_APPEND, DEFAULT_PERMS);
    free(path);

    if (file == -1) {
        printf("error: create file failed\n");
        exit(1);
    }

    if (tcp_info->excess) {
        // TODO: nao escrever \n
        n = write(file, tcp_info->buffer + tcp_info->buffer_i,
                  tcp_info->excess);
        if (n == -1) {
            printf("error: write to file error\n");
            exit(1);
        }

        tcp_info->remaining_size -= n;
        if (tcp_info->remaining_size < 1) {
            if (tcp_info->remaining_size == -1) {
                // remove \n from file
                long new_pos = lseek(file, -1, SEEK_END);
                ftruncate(file, new_pos);
            }

            close(file);
            send_tcp(tcp_sock, tcp_info->final_response);
            return true;
        }
    }
    return false;
}

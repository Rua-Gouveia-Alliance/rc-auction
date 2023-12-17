#include "connection.h"
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

char *use_udp(char *ip_addr, char *port, char *msg, int msg_size,
              int receive_size) {
    int fd, errcode;
    ssize_t n;
    socklen_t addrlen;
    struct sockaddr_in addr;
    struct addrinfo hints, *res;
    char *buffer = (char *)malloc((receive_size + 1) * sizeof(char));
    memset(buffer, 0, (receive_size + 1) * sizeof(char));

    // setup socket
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        printf("error: communication with server failed\n");
        return NULL;
    }

    // setup hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    // dns
    errcode = getaddrinfo(ip_addr, port, &hints, &res);
    if (errcode != 0) {
        printf("error: communication with server failed\n");
        close(fd);
        free(buffer);
        return NULL;
    }

    // send
    fd_set writefds;
    FD_ZERO(&writefds);
    FD_SET(fd, &writefds);

    struct timeval timeout;
    memset((void *)&timeout, 0, sizeof(timeout));
    timeout.tv_sec = TIMEOUT_SECS;

    int ready = select(fd + 1, NULL, &writefds, NULL, &timeout);
    if (ready == -1) {
        printf("error: communication with server failed\n");
        freeaddrinfo(res);
        close(fd);
        free(buffer);
        return NULL;
    } else if (ready == 0) {
        printf("error: connection with server timed out\n");
        freeaddrinfo(res);
        close(fd);
        free(buffer);
        return NULL;
    }

    n = sendto(fd, msg, msg_size, 0, res->ai_addr, res->ai_addrlen);
    if (n == -1) {
        printf("error: communication with server failed\n");
        freeaddrinfo(res);
        close(fd);
        free(buffer);
        return NULL;
    }

    // receive
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);

    ready = select(fd + 1, &readfds, NULL, NULL, &timeout);
    if (ready == -1) {
        printf("error: communication with server failed\n");
        freeaddrinfo(res);
        close(fd);
        free(buffer);
        return NULL;
    } else if (ready == 0) {
        printf("error: connection with server timed out\n");
        freeaddrinfo(res);
        close(fd);
        free(buffer);
        return NULL;
    }

    addrlen = sizeof(addr);
    n = recvfrom(fd, buffer, receive_size, 0, (struct sockaddr *)&addr,
                 &addrlen);

    if (n == -1) {
        printf("error: communication with server failed\n");
        freeaddrinfo(res);
        close(fd);
        free(buffer);
        return NULL;
    }
    buffer[n] = '\0';

    freeaddrinfo(res);
    close(fd);
    return buffer;
}

char *use_tcp(char *ip_addr, char *port, char *msg, int msg_size,
              int receive_size) {
    int fd, errcode;
    ssize_t n;
    socklen_t addrlen;
    struct sockaddr_in addr;
    struct addrinfo hints, *res;
    char *buffer = (char *)malloc((receive_size + 1) * sizeof(char));
    memset(buffer, 0, (receive_size + 1) * sizeof(char));

    fd = socket(AF_INET, SOCK_STREAM, 0); // TCP socket
    if (fd == -1) {
        printf("error: communication with server failed\n");
        free(buffer);
        return NULL;
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP socket

    errcode = getaddrinfo(ip_addr, port, &hints, &res);
    if (errcode != 0) {
        printf("error: communication with server failed\n");
        close(fd);
        free(buffer);
        return NULL;
    }

    n = connect(fd, res->ai_addr, res->ai_addrlen);
    if (n == -1) {
        printf("error: communication with server failed\n");
        freeaddrinfo(res);
        close(fd);
        free(buffer);
        return NULL;
    }

    // send
    fd_set writefds;
    FD_ZERO(&writefds);
    FD_SET(fd, &writefds);

    struct timeval timeout;
    memset((void *)&timeout, 0, sizeof(timeout));
    timeout.tv_sec = TIMEOUT_SECS;

    int ready = select(fd + 1, NULL, &writefds, NULL, &timeout);
    if (ready == -1) {
        printf("error: communication with server failed\n");
        freeaddrinfo(res);
        close(fd);
        free(buffer);
        return NULL;
    } else if (ready == 0) {
        printf("error: connection with server timed out\n");
        freeaddrinfo(res);
        close(fd);
        free(buffer);
        return NULL;
    }

    n = write(fd, msg, msg_size);
    if (n == -1) {
        printf("error: communication with server failed\n");
        freeaddrinfo(res);
        close(fd);
        free(buffer);
        return NULL;
    }

    // receive
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);

    ready = select(fd + 1, &readfds, NULL, NULL, &timeout);
    if (ready == -1) {
        printf("error: communication with server failed\n");
        freeaddrinfo(res);
        close(fd);
        free(buffer);
        return NULL;
    } else if (ready == 0) {
        printf("error: connection with server timed out\n");
        freeaddrinfo(res);
        close(fd);
        free(buffer);
        return NULL;
    }

    n = read(fd, buffer, receive_size);
    if (n == -1) {
        printf("error: communication with server failed\n");
        freeaddrinfo(res);
        close(fd);
        free(buffer);
        return NULL;
    }

    freeaddrinfo(res);
    close(fd);
    return buffer;
}

int read_word(int fd, char *buffer) {
    char c;
    int i = 0;
    ssize_t n;

    // receive
    fd_set readfds, copyfds;
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);

    struct timeval timeout;
    memset((void *)&timeout, 0, sizeof(timeout));
    timeout.tv_sec = TIMEOUT_SECS;

    copyfds = readfds;
    int ready = select(fd + 1, &readfds, NULL, NULL, &timeout);
    if (ready == -1) {
        printf("error: communication with server failed\n");
        return -1;
    } else if (ready == 0) {
        printf("error: connection with server timed out\n");
        return -1;
    }

    n = read(fd, &c, sizeof(char));
    if (n == -1) {
        printf("error: communication with server failed\n");
        return -1;
    }
    while (c != ' ') {
        buffer[i++] = c;

        // receive
        readfds = copyfds;
        ready = select(fd + 1, &readfds, NULL, NULL, &timeout);
        if (ready == -1) {
            printf("error: communication with server failed\n");
            return -1;
        } else if (ready == 0) {
            printf("error: connection with server timed out\n");
            return -1;
        }

        n = read(fd, &c, sizeof(char));
        if (n == -1) {
            printf("error: communication with server failed\n");
            return -1;
        }
    }
    return 0;
}

bool transfer_file(char *ip_addr, char *port, char *msg, int msg_size) {
    int fd, errcode, read_bytes = 0;
    ssize_t n;
    socklen_t addrlen;
    struct sockaddr_in addr;
    struct addrinfo hints, *res;
    char *status = (char *)malloc(7 * sizeof(char));
    memset(status, 0, 7 * sizeof(char));
    char *file_name = (char *)malloc(25 * sizeof(char));
    memset(file_name, 0, 25 * sizeof(char));
    char *file_size = (char *)malloc(9 * sizeof(char));
    memset(file_size, 0, 9 * sizeof(char));
    char *buffer = (char *)malloc(PACKET_SIZE);
    memset(buffer, 0, PACKET_SIZE);

    fd = socket(AF_INET, SOCK_STREAM, 0); // TCP socket
    if (fd == -1) {
        printf("error: communication with server failed\n");
        free(status);
        free(file_name);
        free(file_size);
        free(buffer);
        return false;
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP socket

    errcode = getaddrinfo(ip_addr, port, &hints, &res);
    if (errcode != 0) {
        printf("error: communication with server failed\n");
        free(status);
        free(file_name);
        free(file_size);
        free(buffer);
        close(fd);
        return false;
    }

    n = connect(fd, res->ai_addr, res->ai_addrlen);
    if (n == -1) {
        printf("error: communication with server failed\n");
        free(status);
        free(file_name);
        free(file_size);
        free(buffer);
        freeaddrinfo(res);
        close(fd);
        return false;
    }

    // send
    fd_set writefds;
    FD_ZERO(&writefds);
    FD_SET(fd, &writefds);

    struct timeval timeout;
    memset((void *)&timeout, 0, sizeof(timeout));
    timeout.tv_sec = TIMEOUT_SECS;

    int ready = select(fd + 1, NULL, &writefds, NULL, &timeout);
    if (ready == -1) {
        printf("error: communication with server failed\n");
        free(status);
        free(file_name);
        free(file_size);
        free(buffer);
        freeaddrinfo(res);
        close(fd);
        return NULL;
    } else if (ready == 0) {
        printf("error: connection with server timed out\n");
        free(status);
        free(file_name);
        free(file_size);
        free(buffer);
        freeaddrinfo(res);
        close(fd);
        return NULL;
    }

    n = write(fd, msg, msg_size);
    if (n == -1) {
        printf("error: communication with server failed\n");
        free(status);
        free(file_name);
        free(file_size);
        free(buffer);
        freeaddrinfo(res);
        close(fd);
        return false;
    }

    // receive
    fd_set readfds, copyfds;
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);

    copyfds = readfds;
    ready = select(fd + 1, &readfds, NULL, NULL, &timeout);
    if (ready == -1) {
        printf("error: communication with server failed\n");
        free(status);
        free(file_name);
        free(file_size);
        free(buffer);
        freeaddrinfo(res);
        close(fd);
        return -1;
    } else if (ready == 0) {
        printf("error: connection with server timed out\n");
        free(status);
        free(file_name);
        free(file_size);
        free(buffer);
        freeaddrinfo(res);
        close(fd);
        return -1;
    }

    n = read(fd, status, 7);
    if (n == -1) {
        printf("error: communication with server failed\n");
        free(status);
        free(file_name);
        free(file_size);
        free(buffer);
        freeaddrinfo(res);
        close(fd);
        return false;
    }

    status[6] = '\0';

    if (strcmp(status + 4, "OK") != 0 || strncmp(status, "RSA", 3) != 0) {
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
    if (file == -1) {
        printf("error: writing to file failed (%s)\n", file_name);
        free(status);
        free(file_name);
        free(file_size);
        free(buffer);
        freeaddrinfo(res);
        close(fd);
        return false;
    }

    while (read_bytes < file_bytes) {
        // receive
        readfds = copyfds;
        ready = select(fd + 1, &readfds, NULL, NULL, &timeout);
        if (ready == -1) {
            printf("error: communication with server failed\n");
            free(status);
            free(file_name);
            free(file_size);
            free(buffer);
            freeaddrinfo(res);
            close(fd);
            close(file);
            return -1;
        } else if (ready == 0) {
            printf("error: connection with server timed out\n");
            free(status);
            free(file_name);
            free(file_size);
            free(buffer);
            freeaddrinfo(res);
            close(fd);
            close(file);
            return -1;
        }

        n = read(fd, buffer, PACKET_SIZE);
        if (n == -1) {
            printf("error: communication with server failed\n");
            free(status);
            free(file_name);
            free(file_size);
            free(buffer);
            freeaddrinfo(res);
            close(fd);
            close(file);
            return false;
        }

        n = write(file, buffer, n);
        if (n == -1) {
            printf("error: writing to file failed (%s)\n", file_name);
            free(status);
            free(file_name);
            free(file_size);
            free(buffer);
            freeaddrinfo(res);
            close(fd);
            close(file);
            return false;
        }

        read_bytes += n;
    }
    if (file_bytes - read_bytes == -1) {
        // remove \n from file
        long new_pos = lseek(file, -1, SEEK_END);
        ftruncate(file, new_pos);
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

char *send_file(char *ip_addr, char *port, char *msg, int msg_size,
                char *filename, int receive_size) {
    int fd, errcode, written_bytes = 0;
    ssize_t n;
    socklen_t addrlen;
    struct sockaddr_in addr;
    struct addrinfo hints, *res;
    char *buffer = (char *)malloc(PACKET_SIZE);
    memset(buffer, 0, PACKET_SIZE);
    char *response = (char *)malloc((receive_size + 1) * sizeof(char));
    memset(response, 0, (receive_size + 1) * sizeof(char));

    fd = socket(AF_INET, SOCK_STREAM, 0); // TCP socket
    if (fd == -1) {
        printf("error: communication with server failed\n");
        free(buffer);
        return NULL;
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP socket

    errcode = getaddrinfo(ip_addr, port, &hints, &res);
    if (errcode != 0) {
        printf("error: communication with server failed\n");
        free(buffer);
        close(fd);
        return NULL;
    }

    n = connect(fd, res->ai_addr, res->ai_addrlen);
    if (n == -1) {
        printf("error: communication with server failed\n");
        free(buffer);
        freeaddrinfo(res);
        close(fd);
        return NULL;
    }

    // send
    fd_set writefds, copyfds;
    FD_ZERO(&writefds);
    FD_SET(fd, &writefds);

    struct timeval timeout;
    memset((void *)&timeout, 0, sizeof(timeout));
    timeout.tv_sec = TIMEOUT_SECS;

    copyfds = writefds;
    int ready = select(fd + 1, NULL, &writefds, NULL, &timeout);
    if (ready == -1) {
        printf("error: communication with server failed\n");
        free(buffer);
        freeaddrinfo(res);
        close(fd);
        return NULL;
    } else if (ready == 0) {
        printf("error: connection with server timed out\n");
        free(buffer);
        freeaddrinfo(res);
        close(fd);
        return NULL;
    }

    n = write(fd, msg, msg_size);
    if (n == -1) {
        printf("error: communication with server failed\n");
        free(buffer);
        freeaddrinfo(res);
        close(fd);
        return NULL;
    }

    int file = open(filename, O_RDONLY);
    if (file == -1) {
        printf("error: reading file failed (%s)\n", filename);
        free(buffer);
        freeaddrinfo(res);
        close(fd);
        return NULL;
    }
    struct stat st;
    int failed = fstat(file, &st);
    if (failed) {
        printf("error: reading file failed (%s)\n", filename);
        free(buffer);
        freeaddrinfo(res);
        close(fd);
        return NULL;
    }
    int file_bytes = st.st_size;
    while (written_bytes < file_bytes) {
        memset(buffer, 0, PACKET_SIZE);
        n = read(file, buffer, PACKET_SIZE);
        if (n == -1) {
            printf("error: reading file failed (%s)\n", filename);
            free(buffer);
            freeaddrinfo(res);
            close(fd);
            close(file);
            return NULL;
        }

        // receive
        writefds = copyfds;
        int ready = select(fd + 1, NULL, &writefds, NULL, &timeout);
        if (ready == -1) {
            printf("error: communication with server failed\n");
            free(buffer);
            freeaddrinfo(res);
            close(fd);
            close(file);
            return NULL;
        } else if (ready == 0) {
            printf("error: connection with server timed out\n");
            free(buffer);
            freeaddrinfo(res);
            close(fd);
            close(file);
            return NULL;
        }

        n = write(fd, buffer, n);
        if (n == -1) {
            printf("error: communication with server failed\n");
            free(buffer);
            freeaddrinfo(res);
            close(fd);
            close(file);
            return NULL;
        }

        written_bytes += n;
    }
    write(fd, "\n", 1);

    n = read(fd, response, receive_size);
    if (n == -1) {
        printf("error: communication with server failed\n");
        free(buffer);
        freeaddrinfo(res);
        close(fd);
        close(file);
        return NULL;
    }

    free(buffer);
    freeaddrinfo(res);
    close(fd);
    close(file);
    return response;
}

#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "sock.h"

#define SOCK_QUEUE_MAX 10

int setup_sock(u32 host, u16 port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        errno = ECREATESOCK;
        return -1;
    }

    int reuse = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        errno = ESETOPTSOCK;
        return -1;
    }

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr = { htonl(host) }
    };

    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        errno = EBINDSOCK;
        return -1;
    }

    if (listen(sock, SOCK_QUEUE_MAX) == -1) {
        errno = ELISTENSOCK;
        return -1;
    }

    return sock;
}
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>

#include "tcp.h"

int TCP_createlistener(const char *host, const char *port) {
    struct addrinfo hints, *addr;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host, port, &hints, &addr) != 0) {
        errno = ETCPADDRINFO;
        return -1;
    }

    int serverfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if (serverfd == -1) {
        errno = ECREATESOCK;
        return -1;
    }

    int reuse = 1;
    if (setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        errno = ESETOPTSOCK;
        return -1;
    }

    if (bind(serverfd, addr->ai_addr, addr->ai_addrlen) == -1) {
        errno = EBINDSOCK;
        return -1;
    }

    if (listen(serverfd, 10) == -1) {
        errno = ELISTENSOCK;
        return -1;
    }

    freeaddrinfo(addr);

    return serverfd;
}

int TCP_createclient(const char *host, const char *port) {
    struct addrinfo hints, *addr;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host, port, &hints, &addr) != 0) {
        errno = ETCPADDRINFO;
        return -1;
    }

    int serverfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if (serverfd == -1) {
        errno = ECREATESOCK;
        return -1;
    }

    if (connect(serverfd, addr->ai_addr, addr->ai_addrlen) == -1) {
        errno = ECONNECTSOCK;
        return -1;
    }

    freeaddrinfo(addr);

    return serverfd;
}

char *TCP_geterr(void) {
    switch (errno) {
    case ECREATESOCK:
        return "fail to create server socket";
    case ESETOPTSOCK:
        return "fail to set socket option";
    case EBINDSOCK:
        return "fail to bind address to socket";
    case ELISTENSOCK:
        return "fail to socket start listening for connections";
    case ECONNECTSOCK:
        return "fail to connect to server";
    default:
        return "something went wrong";
    }
}
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "sock.h"

int setup_sock(u32 host, u16 port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        errno = ECREATESOCK;
        return -1;
    }

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr = { htonl(host) }
    };

    if (connect(sock, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        errno = ECONNECTSOCK;
        return -1;
    }

    return sock;
}
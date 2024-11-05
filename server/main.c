#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>
#include <netinet/in.h>

#include "server.h"
#include "protocol.h"

int main(void) {
    struct Server server = { .host = INADDR_LOOPBACK, .port = 3000 };
    if (Server_init(&server) == -1) {
        return 1;
    }

    fprintf(stdout, "%s:%d INFO: server start running at :%d\n", __FILE__, __LINE__, server.port);

    struct epoll_event events[SOCK_QUEUE_MAX];
    while (1) {
        int nfds = epoll_wait(server.pollfd, events, SOCK_QUEUE_MAX, -1);
        if (nfds == -1) {
            fprintf(stderr, "%s:%d ERROR: fail to get ready events.\n", __FILE__, __LINE__);
            continue;
        }

        fprintf(stdout, "%s:%d INFO: receive server events: %d\n", __FILE__, __LINE__, nfds);

        for (int i = 0; i < nfds; i++) {
            struct epoll_event ev = events[i];
            if (!(ev.events & EPOLLIN)) continue;

            if (ev.data.fd == server.fd) {
                Server_handle_connection(&server);
            } else {
                Server_handle_event(&server, ev.data.fd);
            }
        }
    }

    close(server.fd);
    return 0;
}

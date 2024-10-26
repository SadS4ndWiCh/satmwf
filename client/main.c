#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>
#include <netinet/in.h>

#include "sock.h"
#include "client.h"
#include "protocol.h"

int main(int argc, char **argv) {
    struct Client client = {
        .nick = "guest1280",
        .host = INADDR_LOOPBACK,
        .port = 3000
    };

    if (argc == 2) {
        strcpy(client.nick, argv[1]);
    }

    if (Client_init(&client) == -1) {
        return 1;
    }

    struct epoll_event input_ev;
    input_ev.events = EPOLLIN;
    input_ev.data.fd = STDIN_FILENO;

    if (epoll_ctl(client.pollfd, EPOLL_CTL_ADD, STDIN_FILENO, &input_ev) == -1) {
        fprintf(stderr, "%s:%d ERROR: fail to add input event to poll.\n", __FILE__, __LINE__);
        return 2;
    }

    if (Client_join_chat(&client) == -1) {
        return 3;
    }

    struct epoll_event events[SOCK_QUEUE_MAX];
    while (1) {
        int nfds = epoll_wait(client.pollfd, events, SOCK_QUEUE_MAX, -1);
        if (nfds == -1) {
            fprintf(stderr, "%s:%d ERROR: fail to get ready events.\n", __FILE__, __LINE__);
            continue;
        }

        for (int i = 0; i < nfds; i++) {
            struct epoll_event ev = events[i];
            if (!(ev.events & EPOLLIN)) continue;

            if (ev.data.fd == client.fd) {
                Client_handle_message(&client);
            } else if (ev.data.fd == STDIN_FILENO) {
                Client_send_message(&client);
            }
        }
    }

    return 0;
}
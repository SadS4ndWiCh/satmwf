#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_CLIENTS 10
#define MAX_EVENTS (MAX_CLIENTS + 1)

struct array {
    int *items;
    size_t len;
    size_t cap;
};

struct array array_init(size_t cap) {
    struct array arr = { NULL, 0, cap };
    arr.items = (int *) calloc(cap, sizeof(int));

    return arr;
}

int array_append(struct array *arr, int item) {
    if (arr->len == arr->cap) { return -1; }
    arr->items[arr->len++] = item;

    return 0;
}

int array_remove(struct array *arr, int item) {
    for (size_t index = 0; index < arr->len; index++) {
        if (arr->items[index] == item) {
            for (size_t curr = index; curr < arr->len; curr++) {
                arr->items[curr] = arr->items[curr + 1];
            }

            arr->len--;
            break;
        }
    }

    return 0;
}

static int setup_server(uint32_t host, uint16_t port) {
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        fprintf(stderr, "ERROR: socket() failed to create server socket.\n");
        exit(EXIT_FAILURE);
    }

    int reuse = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        fprintf(stderr, "ERROR: setsockopt() failed to set `SO_REUSEADDR` option.\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr = { htonl(host) }
    };

    if (bind(server_sock, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        fprintf(stderr, "ERROR: bind() failed to bind address to the socket.\n");
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, MAX_CLIENTS) == -1) {
        fprintf(stderr, "ERROR: listen() failed to start listening to new connections.\n");
        exit(EXIT_FAILURE);
    }

    return server_sock;
}

int main(void) {
    int server_sock = setup_server(INADDR_LOOPBACK, 3000);

    int epoll_fd = epoll_create1(0);

    struct epoll_event server_ev;
    server_ev.events = EPOLLIN;
    server_ev.data.fd = server_sock;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_sock, &server_ev) == -1) {
        fprintf(stderr, "ERROR: epoll_ctl() failed to add server socket event.\n");
        exit(EXIT_FAILURE);
    }

    struct array clients = array_init(MAX_CLIENTS);
    if (clients.items == -1) {
        fprintf(stderr, "ERROR: array_init() failed to init dynamic array.\n");
        exit(EXIT_FAILURE);
    }

    struct epoll_event events[MAX_EVENTS];

    while (1) {
        fprintf(stdout, "INFO: connected clients: %lu\n", clients.len);

        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for (int i = 0; i < nfds; i++) {
            struct epoll_event ev = events[i];

            // I just want read operations, so, I don't care about others now
            if (!(ev.events & EPOLLIN)) continue;

            // When `server_sock` is ready to read, means that a new client 
            // wants to connect.
            if (ev.data.fd == server_sock) {
                int client_sock = accept(server_sock, NULL, NULL);
                if (client_sock == -1) {
                    continue;
                }

                struct epoll_event new_client_ev;
                new_client_ev.events = EPOLLIN;
                new_client_ev.data.fd = client_sock;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_sock, &new_client_ev) == -1) {
                    fprintf(stderr, "ERROR: epoll_ctl() failed to add client socket event.\n");
                    close(client_sock);
                }

                if (array_append(&clients, client_sock) == -1) {
                    fprintf(stderr, "ERROR: a new client attempt to connect but server is full.\n");
                    close(client_sock);
                }

                continue;
            }

            char buf[1024];
            int nbytes = recv(ev.data.fd, buf, 1024, 0);
            if (nbytes == -1) {
                fprintf(stdout, "WARN: recv() failed to receive message from fd: %d\n", ev.data.fd);
                continue;
            }

            for (int client_i = 0; client_i < clients.len; client_i++) {
                int client_fd = clients.items[client_i];
                if (client_fd == ev.data.fd) { continue; }

                if (send(client_fd, buf, nbytes, 0) == -1) {
                    fprintf(stderr, "ERROR: failed to broadcast message from <%d> to <%d>.\n", ev.data.fd, client_fd);
                }
            }
        }
    }

    return 0;
}
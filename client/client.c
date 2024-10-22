#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>

struct AppendBuffer {
    char *data;
    size_t len;
};

int AppendBuffer_append(struct AppendBuffer *ab, char c, size_t len) {
    ab->data = (char *) realloc(ab->data, ab->len + len);
    if (ab->data == -1) {
        return -1;
    }

    ab->data[ab->len] = c;
    ab->len += len;
    return 0;
}

void AppendBuffer_free(struct AppendBuffer *ab) {
    if (ab->data == NULL) { return; }

    free(ab->data);
    ab->len = 0;
}

static int setup_client(uint32_t host, uint16_t port) {
    int client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock == -1) {
        fprintf(stderr, "ERROR: socket() failed to client socket.\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr = { htonl(host) }
    };

    if (connect(client_sock, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        fprintf(stderr, "ERROR: connect() failed to connect client with server.\n");
        exit(EXIT_FAILURE);
    }

    return client_sock;
}

int kbhit(void) {
    int k;

    ioctl(STDIN_FILENO, FIONREAD, &k);

    return k;
}

int main(void) {
    int client_sock = setup_client(INADDR_LOOPBACK, 3000);

    int epoll_fd = epoll_create1(0);

    struct epoll_event client_ev;
    client_ev.events = EPOLLIN;
    client_ev.data.fd = client_sock;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_sock, &client_ev) == -1) {
        fprintf(stderr, "ERROR: epoll_ctl() failed to add client sock event.\n");
        exit(EXIT_FAILURE);
    }

    struct epoll_event stdin_ev;
    stdin_ev.events = EPOLLIN;
    stdin_ev.data.fd = STDIN_FILENO;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &stdin_ev) == -1) {
        fprintf(stderr, "ERROR: epoll_ctl() failed to add stdin event.\n");
        exit(EXIT_FAILURE);
    }

    struct epoll_event events[10];

    while (1) {
        int nfds = epoll_wait(epoll_fd, events, 10, -1);
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == client_sock) {
                char msg[1024];
                if (recv(events[i].data.fd, msg, 1024, 0) == -1) {
                    fprintf(stderr, "ERROR: recv() failed to receive message.\n");
                    continue;
                }

                printf("(server) ~> %s\n", msg);
            }

            if (events[i].data.fd == STDIN_FILENO) {
                char buf[1024];
                fgets(buf, 1024, stdin);

                printf("(you) ~> %s\n", buf);

                if (send(client_sock, buf, 1024, 0) == -1) {
                    fprintf(stderr, "ERROR: send() failed to send message to server.\n");
                }
            }
        }
    }

    return 0;
}
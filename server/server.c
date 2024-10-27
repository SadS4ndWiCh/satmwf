#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "poll.h"
#include "sock.h"
#include "server.h"
#include "protocol.h"

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

int Server_init(struct Server *server) {
    server->fd = setup_sock(server->host, server->port);
    if (server->fd == -1) {
        switch (errno) {
        case ECREATESOCK:
            fprintf(stderr, "%s:%d ERROR: fail to create server socket.\n", __FILE__, __LINE__);
            return -1;
        case ESETOPTSOCK:
            fprintf(stderr, "%s:%d ERROR: fail to set `SO_REUSEADDR` socket option.\n", __FILE__, __LINE__);
            return -1;
        case EBINDSOCK:
            fprintf(stderr, "%s:%d ERROR: fail to bind address to socket.\n", __FILE__, __LINE__);
            return -1;
        case ELISTENSOCK:
            fprintf(stderr, "%s:%d ERROR: fail to socket start listening for connections.\n", __FILE__, __LINE__);
            return -1;
        default:
            fprintf(stderr, "%s:%d ERROR: something went wrong: %d.\n", __FILE__, __LINE__, errno);
            return -1;
        }
    }

    server->pollfd = epoll_create1(0);
    if (server->pollfd == -1) {
        fprintf(stderr, "%s:%d ERROR: fail to create epoll.\n", __FILE__, __LINE__);
        return -1;
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = server->fd;

    if (epoll_ctl(server->pollfd, EPOLL_CTL_ADD, server->fd, &ev) == -1) {
        fprintf(stderr, "%s:%d ERROR: fail to add server sock to poll.\n", __FILE__, __LINE__);
        return -1;
    }
    
    return 0;
}

int Server_handle_connection(struct Server *server) {
    int client_fd = accept(server->fd, NULL, NULL);
    if (client_fd == -1) {
        fprintf(stderr, "%s:%d ERROR: fail to accept new connection.\n", __FILE__, __LINE__);
        return -1;
    }
    
    fprintf(stdout, "%s:%d INFO: server accept a new socket: %d\n", __FILE__, __LINE__, client_fd);

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = client_fd;

    if (epoll_ctl(server->pollfd, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
        fprintf(stderr, "%s:%d ERROR: fail to add client sock to poll.\n", __FILE__, __LINE__);
        return -1;
    }

    fprintf(stdout, "%s:%d INFO: socket added to the poll: %d\n", __FILE__, __LINE__, client_fd);

    return 0;
}

int Server_handle_message(struct Server *server, int fd) {
    struct Message msg;
    if (Message_recv(fd, &msg) == -1) {
        fprintf(stderr, "%s:%d ERROR: fail to receive message from fd: %d", __FILE__, __LINE__, fd);
        return -1;
    }

    fprintf(stdout, "%s:%d INFO: receives a message: ", __FILE__, __LINE__);

    switch (msg.type) {
    case MCON:
    {
        printf("CON\n");

        if (server->connected_count == SOCK_QUEUE_MAX) {
            struct FCNMessage fcn = { .reason = "server is full." };
            if (Message_send(fd, sizeof(fcn), MFCN, (u8 *) &fcn) == -1) {
                fprintf(stderr, "%s:%d ERROR: fail to send FCN reply to: %d\n", __FILE__, __LINE__, fd);
                return -1;
            }

            return 0;
        }

        struct CONMessage *con = (struct CONMessage *) msg.payload;
        fprintf(stdout, "%s:%d INFO: new client '%s' joined to the server.\n", __FILE__, __LINE__, con->nick);

        // Append client connection to room array
        struct Conn conn = { .id = fd };
        strcpy(conn.nick, con->nick);

        server->room[server->connected_count++] = conn;

        struct SCNMessage scn = { (u8) fd };
        if (Message_send(fd, sizeof(scn), MSCN, (u8 *) &scn) == -1) {
            fprintf(stderr, "%s:%d ERROR: fail to send SCN reply to: %d.\n", __FILE__, __LINE__, fd);
            return -1;
        }

        fprintf(stdout, "%s:%d INFO: sent SCN message to: %d.\n", __FILE__, __LINE__, fd);

        // Notify all clients that a new client joined
        for (int i = 0; i < server->connected_count; i++) {
            struct Conn client = server->room[i];

            if (Message_send(client.id, sizeof(struct CONMessage), MCON, (u8 *) con) == -1) {
                fprintf(stderr, "%s:%d WARN: fail to notify client %s that a new client joined.\n", __FILE__, __LINE__, client.nick);
            }
        }

        return 0;
    } break;
    case MDIS:
    {
        printf("DIS\n");

        struct DISMessage dis;
        for (int i = 0; i < server->connected_count; i++) {
            struct Conn client = server->room[i];

            if (client.id == fd) {
                strcpy(dis.nick, client.nick);

                for (; i < server->connected_count; i++) {
                    if (i == server->connected_count - 1) break;

                    server->room[i] = server->room[i + 1];
                }

                server->connected_count--;
                break;
            }
        }

        if (epoll_ctl(server->pollfd, EPOLL_CTL_DEL, fd, NULL) == -1) {
            fprintf(stderr, "%s:%d ERROR: fail to remove client from poll: %d\n", __FILE__, __LINE__, fd);
            return -1;
        }

        if (close(fd) == -1) {
            fprintf(stderr, "%s:%d ERROR: fail to close client connection\n", __FILE__, __LINE__);
            return -1;
        }

        fprintf(stdout, "%s:%d INFO: client was disconnected: %d\n", __FILE__, __LINE__, fd);

        for (int i = 0; i < server->connected_count; i++) {
            struct Conn client = server->room[i];

            if (Message_send(client.id, sizeof(struct DISMessage), MDIS, (u8 *) &dis) == -1) {
                fprintf(stderr, "%s:%d ERROR: fail to notify client '%s' that the client '%s' exit.\n", __FILE__, __LINE__, client.nick, dis.nick);
            }
        }

        return 0;
    } break;
    case MMSG:
    {
        printf("MSG\n");

        struct MSGMessage *chat_message = (struct MSGMessage *) msg.payload;
        fprintf(stdout, "%s:%d INFO: client %d sent: %s\n", __FILE__, __LINE__, chat_message->author_id, chat_message->message);

        struct Conn sender = { .id = 0 };
        for (int i = 0; i < server->connected_count; i++) {
            struct Conn conn = server->room[i];
            if (conn.id == fd) {
                sender = conn;
                break;
            }
        }

        // TODO: a better code
        if (sender.id == 0 && fd != 0) {
            fprintf(stderr, "%s:%d ERROR: sender not found: %d\n", __FILE__, __LINE__, fd);
            return -1;
        }

        strcpy(chat_message->nick, sender.nick);

        for (int i = 0; i < server->connected_count; i++) {
            struct Conn conn = server->room[i];
            if (Message_send(conn.id, sizeof(struct MSGMessage), MMSG, (u8 *) chat_message) == -1) {
                fprintf(stderr, "%s:%d ERROR: fail to broadcast chat message to: %d\n", __FILE__, __LINE__, conn.id);
            }
        }

        return 0;
    }
    default:
        fprintf(stderr, "%s:%d ERROR: unknow message was receive: %#02x\n", __FILE__, __LINE__, msg.type);
        return -1;
    }
}
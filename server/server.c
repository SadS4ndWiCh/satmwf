#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "tcp.h"
#include "room.h"
#include "server.h"
#include "protocol.h"

int Server_init(struct Server *server) {
    server->fd = TCP_createlistener(server->host, server->port);
    if (server->fd == -1) {
        fprintf(stderr, "%s:%d ERROR: %s.\n", __FILE__, __LINE__, TCP_geterr());
        return -1;
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

    Room_init(&server->room, ROOM_CAP);
    
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

int Server_handle_event(struct Server *server, int fd) {
    struct Event event;
    if (Event_recv(fd, &event) == -1) {
        if (errno == EPROTEMPT) {
            event.length  = 0;
            event.type    = DIS;
            event.payload = NULL;
        } else {
            fprintf(stderr, "%s:%d ERROR: fail to receive event from fd: %d", __FILE__, __LINE__, fd);
            return -1;
        }
    }

    fprintf(stdout, "%s:%d INFO: receives a event: ", __FILE__, __LINE__);

    switch (event.type) {
    case CON:
    {
        printf("CON\n");

        struct CONEvent *con = (struct CONEvent *) event.payload;

        // Check if nick is already taken
        for (size_t i = 0; i < server->room.len; i++) {
            struct Conn conn = server->room.clients[i];

            if (strcmp(conn.nick, con->nick) == 0) {
                struct FCNEvent fcn = { ECONSAMENAME };

                if (Event_send(fd, sizeof(fcn), FCN, (u8 *) &fcn) == -1) {
                    fprintf(stderr, "%s:%d ERROR: fail to send FCN reply to: %d.\n", __FILE__, __LINE__, fd);
                }

                return 0;
            }
        }

        if (Room_addconn(&server->room, fd, con->nick) == -1) {
            fprintf(stdout, "%s:%d ERROR: fail to add '%s' into room.\n", __FILE__, __LINE__, con->nick);
            return -1;
        }

        fprintf(stdout, "%s:%d INFO: new client '%s' joined to the server.\n", __FILE__, __LINE__, con->nick);

        struct SCNEvent scn = { (u8) fd };
        if (Event_send(fd, sizeof(scn), SCN, (u8 *) &scn) == -1) {
            fprintf(stderr, "%s:%d ERROR: fail to send SCN reply to: %d.\n", __FILE__, __LINE__, fd);
            return -1;
        }

        fprintf(stdout, "%s:%d INFO: sent SCN message to: %d.\n", __FILE__, __LINE__, fd);

        // Notify all clients that a new client joined
        for (size_t i = 0; i < server->room.len; i++) {
            struct Conn client = server->room.clients[i];

            if (Event_send(client.id, sizeof(struct CONEvent), CON, (u8 *) con) == -1) {
                fprintf(stderr, "%s:%d WARN: fail to notify client %s that a new client joined.\n", __FILE__, __LINE__, client.nick);
            }
        }

        return 0;
    } break;
    case DIS:
    {
        printf("DIS\n");

        if (epoll_ctl(server->pollfd, EPOLL_CTL_DEL, fd, NULL) == -1) {
            fprintf(stderr, "%s:%d ERROR: fail to remove client from poll: %d\n", __FILE__, __LINE__, fd);
            return -1;
        }

        if (close(fd) == -1) {
            fprintf(stderr, "%s:%d ERROR: fail to close client connection\n", __FILE__, __LINE__);
            return -1;
        }

        struct DISEvent dis;
        struct Conn disclient;

        // Get the disconnected client
        if (Room_getconn(&server->room, fd, &disclient) == -1) {
            // The client wasn't in chat, so don't need to notify that someone left
            return 0;
        }

        Room_delconn(&server->room, fd);

        fprintf(stdout, "%s:%d INFO: client was disconnected: %d\n", __FILE__, __LINE__, fd);

        strcpy(dis.nick, disclient.nick);

        for (size_t i = 0; i < server->room.len; i++) {
            struct Conn conn = server->room.clients[i];
            if (conn.id == fd) continue;

            if (Event_send(conn.id, sizeof(struct DISEvent), DIS, (u8 *) &dis) == -1) {
                fprintf(stderr, "%s:%d ERROR: fail to notify client '%s' that the client '%s' exit.\n", __FILE__, __LINE__, conn.nick, dis.nick);
            }
        }

        return 0;
    } break;
    case MSG:
    {
        printf("MSG\n");

        struct MSGEvent *chat_message = (struct MSGEvent *) event.payload;
        fprintf(stdout, "%s:%d INFO: client %d sent: %s\n", __FILE__, __LINE__, chat_message->authorid, chat_message->message);

        // Find out who is the sender
        struct Conn sender;
        for (size_t i = 0; i < server->room.len; i++) {
            struct Conn conn = server->room.clients[i];
            if (conn.id == fd) {
                sender = conn;
                break;
            }
        }

        if (sender.id != fd) {
            fprintf(stderr, "%s:%d ERROR: sender not found: %d\n", __FILE__, __LINE__, fd);
            return -1;
        }

        strcpy((char *) chat_message->authornick, sender.nick);

        // Broadcast message
        for (size_t i = 0; i < server->room.len; i++) {
            struct Conn conn = server->room.clients[i];
            if (Event_send(conn.id, sizeof(struct MSGEvent), MSG, (u8 *) chat_message) == -1) {
                fprintf(stderr, "%s:%d ERROR: fail to broadcast chat message to: %d\n", __FILE__, __LINE__, conn.id);
            }
        }

        return 0;
    }
    default:
        fprintf(stderr, "%s:%d ERROR: unknow event was receive: %#02x\n", __FILE__, __LINE__, event.type);
        return -1;
    }
}

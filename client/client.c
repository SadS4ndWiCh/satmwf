#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "sock.h"
#include "client.h"
#include "protocol.h"

int setup_sock(u32 host, u16 port) {
    errno = 0;

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

int Client_init(struct Client *client) {
    client->fd = setup_sock(client->host, client->port);
    if (client->fd == -1) {
        switch (errno) {
        case ECREATESOCK:
            fprintf(stderr, "%s:%d ERROR: fail to create client socket.\n", __FILE__, __LINE__);
            return -1;
        case ECONNECTSOCK:
            fprintf(stderr, "%s:%d ERROR: fail to connect to server.\n", __FILE__, __LINE__);
            return -1;
        }
    }

    client->pollfd = epoll_create1(0);
    if (client->pollfd == -1) {
        fprintf(stderr, "%s:%d ERROR: fail to create client poll.\n", __FILE__, __LINE__);
        return -1;
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = client->fd;

    if (epoll_ctl(client->pollfd, EPOLL_CTL_ADD, client->fd, &ev) == -1) {
        fprintf(stderr, "%s:%d ERROR: fail to add client sock to the poll.\n", __FILE__, __LINE__);
        return -1;
    }

    return 0;
}

int Client_join_chat(struct Client *client) {
    struct CONMessage con;
    strcpy(con.nick, client->nick);

    if (Message_send(client->fd, sizeof(con), MCON, (u8 *) &con) == -1) {
        switch (errno) {
        case EPROTOVRF:
            fprintf(stderr, "%s:%d ERROR: sent a too long message.\n", __FILE__, __LINE__);
            return -1;
        case EPROTSEND:
            fprintf(stderr, "%s:%d ERROR: fail to send message.\n", __FILE__, __LINE__);
            return -1;
        default:
            fprintf(stderr, "%s:%d ERROR: something went wrong: %d\n", __FILE__, __LINE__, errno);
            return -1;
        }
    }

    fprintf(stdout, "%s:%d INFO: joining the server...", __FILE__, __LINE__);

    struct Message replymsg;
    if (Message_recv(client->fd, &replymsg) == -1) {
        switch (errno) {
        case EPROTLEN:
            fprintf(stderr, "%s:%d ERROR: fail to receive message length.\n", __FILE__, __LINE__);
            return -1;
        case EPROTTYPE:
            fprintf(stderr, "%s:%d ERROR: fail to receive message type.\n", __FILE__, __LINE__);
            return -1;
        case EPROTOVRF:
            fprintf(stderr, "%s:%d ERROR: receive a too long message.\n", __FILE__, __LINE__);
            return -1;
        case EPROTPAYL:
            fprintf(stderr, "%s:%d ERROR: fail to receive message payload.\n", __FILE__, __LINE__);
            return -1;
        default:
            fprintf(stderr, "%s:%d ERROR: something went wrong: %d\n", __FILE__, __LINE__, errno);
            return -1;
        }       
    }

    if (replymsg.type != MSCN && replymsg.type != MFCN) {
        fprintf(stderr, "%s:%d ERROR: receive an unexpected message, receive '%#02x', but expected '%#02x' or '%#02x'", __FILE__, __LINE__, replymsg.type, MSCN, MFCN);
        return -1;
    }

    if (replymsg.type == MFCN) {
        struct FCNMessage *fcn = (struct FCNMessage *) replymsg.payload;

        switch (fcn->reason) {
        case RSNSERVERFULL:
            fprintf(stderr, "%s:%d ERROR: fail to join the server: server is full!\n", __FILE__, __LINE__);
            break;
        default:
            fprintf(stderr, "%s:%d ERROR: fail to join the server\n", __FILE__, __LINE__);
            break;
        }

        return -1;
    }

    struct SCNMessage *scn = (struct SCNMessage *) replymsg.payload;
    client->id = scn->id;

    fprintf(stdout, "%s:%d INFO: joined to the server.\n", __FILE__, __LINE__);

    return 0;
}

int Client_handle_message(struct Client *client) {
    struct Message msg;
    if (Message_recv(client->fd, &msg) == -1) {
        switch (errno) {
        case EPROTLEN:
            fprintf(stderr, "%s:%d ERROR: fail to receive message length.\n", __FILE__, __LINE__);
            return -1;
        case EPROTTYPE:
            fprintf(stderr, "%s:%d ERROR: fail to receive message type.\n", __FILE__, __LINE__);
            return -1;
        case EPROTOVRF:
            fprintf(stderr, "%s:%d ERROR: receive a too long message.\n", __FILE__, __LINE__);
            return -1;
        case EPROTPAYL:
            fprintf(stderr, "%s:%d ERROR: fail to receive message payload.\n", __FILE__, __LINE__);
            return -1;
        default:
            fprintf(stderr, "%s:%d ERROR: something went wrong: %d\n", __FILE__, __LINE__, errno);
            return -1;
        }
    }

    fprintf(stdout, "%s:%d INFO: receives a message: ", __FILE__, __LINE__);

    switch (msg.type) {
    case MMSG:
    {
        printf("MSG\n");

        struct MSGMessage *chat_message = (struct MSGMessage *) msg.payload;
        printf("(%s) ~> ", chat_message->nick);

        printf("%s\n", chat_message->message);
    } break;
    case MCON:
    {
        printf("CON\n");

        struct CONMessage *con = (struct CONMessage *) msg.payload;

        printf("%s:%d INFO: %s joined the server\n", __FILE__, __LINE__, con->nick);
    } break;
    case MDIS:
    {
        printf("DIS\n");

        struct DISMessage *dis = (struct DISMessage *) msg.payload;

        printf("%s:%d INFO: %s left the server\n", __FILE__, __LINE__, dis->nick);
    } break;
    }

    return 0;
}

int Client_send_message(struct Client *client) {
    struct MSGMessage chat_message = { .author_id = client->id };
    strcpy(chat_message.nick, client->nick);
    fgets(chat_message.message, MESSAGE_PAYLOAD_MAX - sizeof(u8) - sizeof(chat_message.nick), stdin);

    size_t mlen = strlen(chat_message.message);
    if (mlen == 0) {
        return 0;
    }

    // Remove the New Line (\n)
    if (chat_message.message[mlen - 1] == '\n') {
        chat_message.message[mlen - 1] = 0;
    }

    if (Message_send(client->fd, sizeof(chat_message), MMSG, (u8 *) &chat_message) == -1) {
        switch (errno) {
        case EPROTOVRF:
            fprintf(stderr, "%s:%d ERROR: send a too long message.\n", __FILE__, __LINE__);
            return -1;
        case EPROTSEND:
            fprintf(stderr, "%s:%d ERROR: fail to send chat message.\n", __FILE__, __LINE__);
            return -1;
        default:
            fprintf(stderr, "%s:%d ERROR: something went wrong.\n", __FILE__, __LINE__);
            return -1;
        }
    }

    return 0;
}
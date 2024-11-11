#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "tcp.h"
#include "client.h"
#include "protocol.h"

int Client_init(struct Client *client) {
    client->fd = TCP_createclient(client->host, client->port);
    if (client->fd == -1) {
        fprintf(stderr, "%s:%d ERROR: %s.\n", __FILE__, __LINE__, TCP_geterr());
        return -1;
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
    struct CONEvent con;
    strcpy(con.nick, client->nick);

    if (Event_send(client->fd, sizeof(con), CON, (u8 *) &con) == -1) {
        switch (errno) {
        case EPROTOVRF:
            fprintf(stderr, "%s:%d ERROR: sent a too long event.\n", __FILE__, __LINE__);
            return -1;
        case EPROTSEND:
            fprintf(stderr, "%s:%d ERROR: fail to send event.\n", __FILE__, __LINE__);
            return -1;
        default:
            fprintf(stderr, "%s:%d ERROR: something went wrong: %d\n", __FILE__, __LINE__, errno);
            return -1;
        }
    }

    struct Event status;
    if (Event_recv(client->fd, &status) == -1) {
        switch (errno) {
        case EPROTLENT:
            fprintf(stderr, "%s:%d ERROR: fail to receive event length.\n", __FILE__, __LINE__);
            return -1;
        case EPROTTYPE:
            fprintf(stderr, "%s:%d ERROR: fail to receive event type.\n", __FILE__, __LINE__);
            return -1;
        case EPROTOVRF:
            fprintf(stderr, "%s:%d ERROR: receive a too long event.\n", __FILE__, __LINE__);
            return -1;
        case EPROTPAYL:
            fprintf(stderr, "%s:%d ERROR: fail to receive event payload.\n", __FILE__, __LINE__);
            return -1;
        default:
            fprintf(stderr, "%s:%d ERROR: something went wrong: %d\n", __FILE__, __LINE__, errno);
            return -1;
        }       
    }

    if (status.type != SCN && status.type != FCN) {
        fprintf(stderr, "%s:%d ERROR: receive an unexpected status '%#02x', but expected '%#02x' or '%#02x'", __FILE__, __LINE__, status.type, SCN, FCN);
        return -1;
    }

    if (status.type == FCN) {
        struct FCNEvent *fcn = (struct FCNEvent *) status.payload;

        switch (fcn->reason) {
        case ECONSAMENAME:
            fprintf(stderr, "%s:%d ERROR: fail to join the server: nick is already taken!\n", __FILE__, __LINE__);
            break;
        default:
            fprintf(stderr, "%s:%d ERROR: fail to join the server\n", __FILE__, __LINE__);
            break;
        }

        return -1;
    }

    struct SCNEvent *scn = (struct SCNEvent *) status.payload;
    client->id = scn->id;

    fprintf(stdout, "%s:%d INFO: joined to the server.\n", __FILE__, __LINE__);

    return 0;
}

int Client_handle_event(struct Client *client) {
    struct Event event;
    if (Event_recv(client->fd, &event) == -1) {
        switch (errno) {
        case EPROTLENT:
            fprintf(stderr, "%s:%d ERROR: fail to receive event length.\n", __FILE__, __LINE__);
            return -1;
        case EPROTTYPE:
            fprintf(stderr, "%s:%d ERROR: fail to receive event type.\n", __FILE__, __LINE__);
            return -1;
        case EPROTOVRF:
            fprintf(stderr, "%s:%d ERROR: receive a too long event.\n", __FILE__, __LINE__);
            return -1;
        case EPROTPAYL:
            fprintf(stderr, "%s:%d ERROR: fail to receive event payload.\n", __FILE__, __LINE__);
            return -1;
        default:
            fprintf(stderr, "%s:%d ERROR: something went wrong: %d\n", __FILE__, __LINE__, errno);
            return -1;
        }
    }

    fprintf(stdout, "%s:%d INFO: receives an event: ", __FILE__, __LINE__);

    switch (event.type) {
    case MSG:
    {
        printf("MSG\n");

        struct MSGEvent *chat_message = (struct MSGEvent *) event.payload;
        printf("(%s) ~> %s\n", chat_message->authornick, chat_message->message);
    } break;
    case CON:
    {
        printf("CON\n");

        struct CONEvent *con = (struct CONEvent *) event.payload;

        printf("%s:%d INFO: %s joined the server\n", __FILE__, __LINE__, con->nick);
    } break;
    case DIS:
    {
        printf("DIS\n");

        struct DISEvent *dis = (struct DISEvent *) event.payload;

        printf("%s:%d INFO: %s left the server\n", __FILE__, __LINE__, dis->nick);
    } break;
    }

    return 0;
}

int Client_send_message(struct Client *client) {
    struct MSGEvent chat_message = { .authorid = client->id };
    strcpy(chat_message.authornick, client->nick);
    fgets(chat_message.message, MESSAGE_LENGTH, stdin);

    size_t mlen = strlen(chat_message.message);
    if (mlen == 0) {
        return 0;
    }

    // Remove the New Line (\n)
    if (chat_message.message[mlen - 1] == '\n') {
        chat_message.message[mlen - 1] = 0;
    }

    if (Event_send(client->fd, sizeof(chat_message), MSG, (u8 *) &chat_message) == -1) {
        switch (errno) {
        case EPROTOVRF:
            fprintf(stderr, "%s:%d ERROR: send a too long event.\n", __FILE__, __LINE__);
            return -1;
        case EPROTSEND:
            fprintf(stderr, "%s:%d ERROR: fail to send chat event.\n", __FILE__, __LINE__);
            return -1;
        default:
            fprintf(stderr, "%s:%d ERROR: something went wrong.\n", __FILE__, __LINE__);
            return -1;
        }
    }

    return 0;
}

#ifndef __CLIENT_H
#define __CLIENT_H

#include "types.h"

#define SOCK_QUEUE_MAX 10

#define DEFAULT_NICK "unknown"

struct Client {
    u8 id;
    char nick[20];

    char *host;
    char *port;
    int fd;

    int pollfd;
};

int Client_init(struct Client *client);
int Client_join_chat(struct Client *client);
int Client_exit_chat(struct Client *client);
int Client_send_message(struct Client *client);

int Client_handle_event(struct Client *client);

#endif

#ifndef __CLIENT_H
#define __CLIENT_H

#include "types.h"

#define SOCK_QUEUE_MAX 10

struct Client {
    u8 id;
    char nick[20];

    u32 host;
    u16 port;
    int fd;

    int pollfd;
};

int Client_init(struct Client *client);
int Client_join_chat(struct Client *client);
int Client_exit_chat(struct Client *client);
int Client_send_message(struct Client *client);

int Client_handle_message(struct Client *client);

#endif
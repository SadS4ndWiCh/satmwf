#ifndef __SERVER_H
#define __SERVER_H

#include "types.h"

#define SOCK_QUEUE_MAX 10

struct Conn {
    u8 id;
    char nick[20];
};

struct Server {
    u32 host;
    u16 port;
    int fd;

    int pollfd;

    u8 connected_count;
    struct Conn room[SOCK_QUEUE_MAX];
};

int Server_init(struct Server *server);
int Server_handle_connection(struct Server *server);
int Server_handle_message(struct Server *server, int fd);

#endif
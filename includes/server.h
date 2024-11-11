#ifndef __SERVER_H
#define __SERVER_H

#include "room.h"

#define SOCK_QUEUE_MAX 10

struct Server {
    char *host;
    char *port;
    int fd;

    int pollfd;

    struct Room room;
};

int Server_init(struct Server *server);
int Server_handle_connection(struct Server *server);
int Server_handle_event(struct Server *server, int fd);

#endif

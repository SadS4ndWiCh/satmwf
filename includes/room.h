#ifndef __ROOM_H
#define __ROOM_H

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"

#define ROOM_CAP 10

/* Erros */

#define EEXPANDROOM 0

struct Conn {
    u8 id;
    char nick[20];
};

struct Room {
    struct Conn *clients;
    size_t len;
    size_t cap;
};

void Room_init(struct Room *room, size_t cap);
int Room_addconn(struct Room *room, u8 id, char nick[20]);
void Room_delconn(struct Room *room, u8 id);
int Room_getconn(struct Room *room, u8 id, struct Conn *dest);

#endif
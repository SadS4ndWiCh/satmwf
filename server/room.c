#include <stdio.h>

#include "room.h"
#include "protocol.h"

void Room_init(struct Room *room, size_t cap) {
    room->cap = cap;
    room->len = 0;
    room->clients = (struct Conn *) malloc(sizeof(struct Conn) * cap);
}

int Room_addconn(struct Room *room, u8 id, char nick[20]) {
    if (room->len == room->cap) {
        room->clients = (struct Conn *) realloc(room->clients, room->cap + ROOM_CAP);
        if (room->clients == NULL) {
            errno = EEXPANDROOM;
            return -1;
        }
    }

    room->clients[room->len].id = id;
    strcpy(room->clients[room->len].nick, nick);

    room->len++;
    return 0;
}

void Room_delconn(struct Room *room, u8 id) {
    if (room->len == 0) return;

    for (size_t i = 0; i < room->len; i++) {
        struct Conn conn = room->clients[i];
        if (conn.id != id) continue;

        for (; i < room->len - 1; i++) {
            room->clients[i] = room->clients[i + 1];
        }

        room->len--;
        break;
    }
}

int Room_getconn(struct Room *room, u8 id, struct Conn *dest) {
    for (size_t i = 0; i < room->len; i++) {
        struct Conn conn = room->clients[i];

        if (conn.id == id) {
            *dest = conn;
            return 0;
        }
    }

    return -1;
}
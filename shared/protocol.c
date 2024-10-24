#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "protocol.h"

void Message_headerFromBytes(struct Message *dest, u8 *buf) {
    memcpy(&dest->length, buf, sizeof(u16));
    memcpy(&dest->type, &buf[sizeof(u16)], sizeof(u8));
}

void Message_payloadFromBytes(struct Message *dest, u8 *buf) {
    dest->payload = (u8 *) malloc(dest->length);
    memcpy(dest->payload, &buf[sizeof(u16) + sizeof(u8)], dest->length);
}

void Message_fromBytes(struct Message *dest, u8 *buf) {
    memcpy(&dest->length, buf, sizeof(u16));
    memcpy(&dest->type, &buf[sizeof(u16)], sizeof(u8));

    dest->payload = (u8 *) malloc(dest->length);
    memcpy(dest->payload, &buf[sizeof(u16) + sizeof(u8)], dest->length);
}

void Message_toBytes(struct Message *src, u8 *dest) {
    memcpy(dest, &src->length, sizeof(u16));
    memcpy(&dest[sizeof(u16)], &src->type, sizeof(u8));
    memcpy(&dest[sizeof(u16) + sizeof(u8)], src->payload, src->length);
}

int Message_recv(int fd, struct Message *dest) {
    u8 buffer[MESSAGE_HEADER_LENGTH + MESSAGE_PAYLOAD_MAX];
    if (recv(fd, buffer, sizeof(u16), 0) == -1) {
        errno = EPROTLEN;
        return -1;
    }

    if (recv(fd, &buffer[sizeof(u16)], sizeof(u8), 0) == -1) {
        errno = EPROTTYPE;
        return -1;
    }

    Message_headerFromBytes(dest, buffer);

    if (recv(fd, &buffer[sizeof(u16) + sizeof(u8)], dest->length, 0) == -1) {
        errno = EPROTPAYL;
        return -1;
    }

    Message_payloadFromBytes(dest, buffer);
    return 0;
}
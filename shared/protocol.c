#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "protocol.h"

void Event_headerFromBytes(struct Event *dest, u8 *buf) {
    memcpy(&dest->length, buf, sizeof(u16));
    memcpy(&dest->type, &buf[sizeof(u16)], sizeof(u8));
}

void Event_payloadFromBytes(struct Event *dest, u8 *buf) {
    if (dest->length == 0) return;

    dest->payload = (u8 *) malloc(dest->length);
    memcpy(dest->payload, &buf[sizeof(u16) + sizeof(u8)], dest->length);
}

void Event_toBytes(struct Event *src, u8 *dest) {
    memcpy(dest, &src->length, sizeof(u16));
    memcpy(&dest[sizeof(u16)], &src->type, sizeof(u8));

    if (src->length > 0) {
        memcpy(&dest[sizeof(u16) + sizeof(u8)], src->payload, src->length);
    }
}

int Event_recv(int fd, struct Event *dest) {
    errno = 0;

    u8 buffer[EVENT_HEADER_LENGTH + EVENT_PAYLOAD_LENGTH];
    int nbytes = recv(fd, buffer, sizeof(u16), 0);
    if (nbytes == 0) {
        errno = EPROTEMPT;
        return -1;
    }
    
    if (nbytes == -1) {
        errno = EPROTLENT;
        return -1;
    }

    if (recv(fd, &buffer[sizeof(u16)], sizeof(u8), 0) == -1) {
        errno = EPROTTYPE;
        return -1;
    }

    Event_headerFromBytes(dest, buffer);

    if (dest->length == 0) return 0;

    if (dest->length > EVENT_PAYLOAD_LENGTH) {
        errno = EPROTOVRF;
        return -1;
    }

    if (recv(fd, &buffer[sizeof(u16) + sizeof(u8)], dest->length, 0) == -1) {
        errno = EPROTPAYL;
        return -1;
    }

    Event_payloadFromBytes(dest, buffer);
    return 0;
}

int Event_send(int fd, u16 length, u8 type, u8 *payload) {
    errno = 0;
    
    struct Event msg = {
        .length  = length,
        .type    = type,
        .payload = payload 
    };

    if (msg.length > EVENT_PAYLOAD_LENGTH) {
        errno = EPROTOVRF;
        return -1;
    }

    size_t buffer_len = EVENT_HEADER_LENGTH + msg.length;
    u8 *buffer = (u8 *) malloc(buffer_len);

    Event_toBytes(&msg, buffer);

    if (send(fd, buffer, buffer_len, 0) == -1) {
        errno = EPROTSEND;
        return -1;
    }

    free(buffer);
    return 0;
}

char *Event_geterr(void) {
    switch (errno) {
    case EPROTLENT:
        return "fail to receive event length";
    case EPROTTYPE:
        return "fail to receive event type";
    case EPROTPAYL:
        return "fail to receive event payload";
    case EPROTOVRF:
        return "too long event";
    case EPROTSEND:
        return "fail to send event";
    default:
        return "something went wrong";
    }       
}
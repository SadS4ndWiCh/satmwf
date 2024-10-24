#include <stdlib.h>
#include <string.h>

#include "protocol.h"

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
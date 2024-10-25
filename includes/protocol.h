#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#include "types.h"

#define MESSAGE_HEADER_LENGTH 3
#define MESSAGE_PAYLOAD_MAX 256

/* Message Types */
#define MCON 0
#define MSCN 1
#define MFCN 2
#define MDIS 3
#define MTAI 4
#define MTOS 5
#define MMSG 6

/* Errors */
#define EPROTLEN  0
#define EPROTTYPE 1
#define EPROTPAYL 2

struct Message {
    u16 length;
    u8 type;
    u8 *payload;
};

struct CONMessage {
    char nick[20];
};

struct SCNMessage {
    u8 id;
};

struct FCNMessage {
    char reason[MESSAGE_PAYLOAD_MAX];
};

struct MSGMessage {
    u8 author_id;
    char message[255];
};

void Message_headerFromBytes(struct Message *dest, u8 *buf);
void Message_payloadFromBytes(struct Message *dest, u8 *buf);

void Message_fromBytes(struct Message *dest, u8 *buf);
void Message_toBytes(struct Message *src, u8 *dest);

int Message_recv(int fd, struct Message *dest);
int Message_send(int fd, struct Message *msg);

#endif
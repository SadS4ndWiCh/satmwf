#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#include "types.h"

#define MESSAGE_HEADER_LENGTH 3
#define MESSAGE_PAYLOAD_MAX 256

/* Message Types */

#define MCON 0 // Request to connect
#define MSCN 1 // Success to connect
#define MFCN 2 // Fail to connect
#define MDIS 3 // Request to disconnect
#define MTAI 4 // Tái?
#define MTOS 5 // Tô sim
#define MMSG 6 // Receive / Send chat message

/* Errors */

#define EPROTLEN  0 // Fail to parse message length
#define EPROTTYPE 1 // Fail to parse message type
#define EPROTPAYL 2 // Fail to parse message payload
#define EPROTOVRF 3 // Message payload overflow
#define EPROTSEND 4 // Fail to send message

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
    char nick[20];
    char message[235];
};

void Message_headerFromBytes(struct Message *dest, u8 *buf);
void Message_payloadFromBytes(struct Message *dest, u8 *buf);

void Message_toBytes(struct Message *src, u8 *dest);

int Message_recv(int fd, struct Message *dest);
int Message_send(int fd, struct Message *msg);

#endif
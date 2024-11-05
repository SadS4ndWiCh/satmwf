#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#include "types.h"

/* Lengths */

#define EVENT_HEADER_LENGTH 3
#define EVENT_PAYLOAD_LENGTH 256
#define NICK_LENGTH 20
#define MESSAGE_LENGTH (EVENT_PAYLOAD_LENGTH - NICK_LENGTH - 1)

/* Event Types */

#define CON 0x00 // Connect
#define SCN 0x01 // Success to Connect
#define FCN 0x02 // Fail to Connect
#define DIS 0x03 // Disconnection
#define MSG 0x04 // Message

/* Errors */

#define EPROTLENT 0x00 // Fail to parse event length
#define EPROTTYPE 0x01 // Fail to parse event type
#define EPROTPAYL 0x02 // Fail to parse event payload
#define EPROTOVRF 0x03 // Event payload overflow
#define EPROTSEND 0x04 // Fail to send event 
#define EPROTEMPT 0x05 // Receives an empty data (Possible disconnection)

/* FCN Reasons */

#define ECONSAMENAME 0x00 // The client chosen nick is already taken
#define ECONCHATFULL 0x01 // The chat is full
#define ECONUNEXPECT 0x02 // An unexpected error occour in server

struct Event {
    u16 length;
    u8 type;
    u8 *payload;
};

struct CONEvent {
    char nick[NICK_LENGTH];
};

struct SCNEvent {
    u8 id;
};

struct FCNEvent {
    u8 reason;
};

struct DISEvent {
    char nick[NICK_LENGTH];
};

struct MSGEvent {
    u8 authorid;
    char authornick[NICK_LENGTH];
    char message[MESSAGE_LENGTH];
};

/*
    Parse the Event Header from the buffer to `Event` struct.

    Parameters:
    - `dest` The destionation pointer.
    - `buf` Buffer which event is stored.
*/
void Event_headerFromBytes(struct Event *dest, u8 *buf);

/*
    Parse the Event Payload from the buffer to `Event` struct.

    To parse the payload, the header must be already parsed, because the payload 
    `length` must be know.

    Parameters:
    - `dest` The destionation pointer.
    - `buf` Buffer which event is stored.
*/
void Event_payloadFromBytes(struct Event *dest, u8 *buf);

/*
    Convert an Event to a bytes buffer.

    Parameters:
    - `src` The event which wants to convert.
    - `dest` The target buffer where event must be stored.
*/
void Event_toBytes(struct Event *src, u8 *dest);

/*
    Helper function to receive an Event from a socket.

    Parameters:
    - `fd` The target socket.
    - `dest` The destination to receive the Event.
*/
int Event_recv(int fd, struct Event *dest);

/*
    Helper function to send an Event to a socket.

    Parameters:
    - `fd` The target socket.
    - `length` The Event length.
    - `type` The Event type.
    - `payload` The Event payload.
*/
int Event_send(int fd, u16 length, u8 type, u8 *payload);

#endif

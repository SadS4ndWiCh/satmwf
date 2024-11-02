<img src=".github/typing.gif" width="300">

# 🏮 satmwf

The SATMWF (Super Application To Message With Friends) is the best software ever 
(wow, who thinks that?) to chat virtually.

## 🏑 How it works

Basically, it has the server that is the primary instance that manages all client 
connections and broadcasts the message sent by some client to others.

### Communication structure:

The entire communication between client <-> server is event based. Every event uses 
the following structure:

```
 0               1               2              
 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|            Length             |     Type      |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                   Payload                     |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

For a very simple approach, it only contains the `Length` (Payload length), `Type` 
(Event type) and the `Payload` from the event.

The `Length` field is a 2 bytes unsigned integer which range is `0-65535`. It means 
that the payload can have a limit of 65535 bytes. An event can have a `0` length 
payload, which means an _empty event_.

The `Type` field is a 1 byte unsigned integer. Each event type is represented by 
a code. All available events are:

- `CON`: 0x00;
- `SCN`: 0x01;
- `FCN`: 0x02;
- `MSG`: 0x03;
- `DIS`: 0x04;

The `Payload` field have a variable length defined in `Length` field.

### Event type payload structs

#### CON (Connect)

```c
struct CONEvent {
    nick_t nick;
};
```

The `CON` event can be sent both by client and server. 

The client sends it to request to join the chat and server sends to notify that 
someone joined the chat. In both cases, the `nick` payload is sent.

#### SCN (Success to Connect)

```c
struct SCNEvent {
    clientid_t id;
};
```

The `SCN` event can only be sent by the server.

When a client was able to join the chat, the server reply the `CON` event with 
given `id`. The client must store that `id` to know who he is.

#### FCN (Fail to Connect)

```c
struct SCNEvent {
    u8 reason;
};
```

The `FCN` event can only be sent by the server.

When a client wasn't able to join the chat, the server reply the `CON` event with 
the reason to don't be able to join. The reasons can be:

- `ECONSAMENAME`: The nick was already taken;
- `ECONUNEXPECT`: Some unexpected error occour;

#### MSG (Message)

```c
struct MSGEvent {
    clientid_t from;
    message_t message;
};
```

The `MSG` event can be sent both by the client and server.

The client sends it to send a new message and the sends to notify that a new message 
was received.

When the client sends the message, the `from` field is ignored by the server, so 
client don't to set it. But the server needs to fill it clients know what message 
they sent.

#### DIS (Disconnection)

```c
struct DISEvent {
    nick_t nick;
};
```

The `DIS` event can only be sent by the server.

When a client closes the connection with the server (or in other words, left from 
the chat), the server notify that someone left from the chat.

### Flow charts

#### Join chat flow

```
+ ------ +                     + ------ +                        + ------- +
| Client |                     | SERVER |                        | CLIENTS |
+ ------ +                     + ------ +                        + ------- +
    ||           CON               ||                                 ||
    || --------------------------> ||       + --------- +             ||
    ||  Request to join the chat   || ----> | Can join? |             ||
    ||                             ||       + --------- +             ||
    ||           FCN               ||    <NO> |       | <YES>         ||
    || <-------------------------- || ------- +       |               ||
    --    Fail to join the chat    --                 |               || 
    ||                             ||                 |               ||
    ||           SCN               ||                 |               ||
    || <-------------------------- || --------------- +               ||
    ||   Success to join the chat  ||                                 ||
    ||                             ||              CON                ||
    ||                             || ------------------------------> ||
    ||                             ||     Someone joined the chat     ||
    ||                             ||                                 ||
```

#### Messaging flow

```
+ ------ +                     + ------ +                        + ------- +
| Client |                     | SERVER |                        | CLIENTS |
+ ------ +                     + ------ +                        + ------- +
    ||           MSG               ||                                 ||
    || --------------------------> ||              MSG                ||
    ||     Send a new message      || ------------------------------> ||
    ||                             ||        New message arrive       ||
    ||                             ||                                 ||
```

#### Disconnection flow

```
+ ------ +                     + ------ +                        + ------- +
| Client |                     | SERVER |                        | CLIENTS |
+ ------ +                     + ------ +                        + ------- +
    ||                             ||                                 ||
    || <-----------//------------> ||              DIS                ||
    ||      Connection closed      || ------------------------------> ||
    ||                             ||       Client left the chat      ||
    ||                             ||                                 ||
```

## 🎳 Building

First, make sure you have the make command installed. Also, I'm using the `cc` compiler, 
in which many system already have, but if not, just install it or use any of your 
preference and update the `CC` variable in `Makefile`.

To build both server and client at once, just run the command:

```sh
$ make
```

Otherwise, if you want to build each one individually:

```sh
$ make server
$ make client
```
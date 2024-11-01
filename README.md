<img src=".github/typing.gif" width="300">

# 🏮 satmwf

The SATMWF (Super Application To Message With Friends) is the best software ever 
(wow, who thinks that?) to chat virtually.

## 🏑 How it works

Basically, it has the server that is the primary instance that manages all client 
connections and broadcasts the message from some client to the others.

### Communication structure:

All communication between client <-> server uses the same following simple structure:

```
 0               1               2              
 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|            Length             |     Type      |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                   Payload                     |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

The message must start with the `length` (2 bytes) of the `payload` following by 
the message `type`. A message can have a empty `payload`, so `length` can be 0 in 
that case.

All message types:

- [x] `CON`: Request to connect;
- [x] `SCN`: Success to connect;
- [x] `FCN`: Fail to connect;
- [x] `DIS`: Request to disconnect;
- [ ] `TAI`: _Taí?_;
- [ ] `TOS`: _Tô sim_;
- [x] `MSG`: Chat message;

#### CON Message

A `CON` message can be sent both by client and server. 

In client POV, if he send the message, mean the client want to join the server. 
Just after the `CON` message was successfuly replyed with `SCN`, the client is added 
to room list and can start sending `MSG` messages to the server broadcast to other 
clients. 

In server POV, he send the `CON` message to notify the clients that a new client 
joined the server.

```c
struct CONMessage {
    char nick[20];
};
```

The `CON` message allow send a payload with a nick name as an alias to their id. The 
nick name must have until 20 bytes (20 characters).

#### SCN Message

A `SCN` can only be sent by the server as an reply to `CON`.

The server reply the `CON` message with `SCN` if the client successfuly joined the 
chat.

```c
struct SCNMessage {
    u8 id;
};
```

The message payload contains the `id` defined to the client.

#### FCN Message

A `FCN` can only be sent by the server as an reply to `CON`.

The server reply the `CON` message with `FCN` if the client was't able to join the 
chat.

```c
struct FCNMessage {
    u8 reason;
};
```

The message payload contains the `reason` to client don't be able to join the chat. 
The reasons are:
 - `RSNSERVERFULL` The server is already full;
 - `RSNINTERNAL` An unexpected error occour in server;

#### DIS Message

A `DIS` message can be sent only by the server. 

When a client disconnect, the server sends the `DIS` message to all other clients 
to notify that a client was left from the chat.

```c
struct DISMessage {
    char nick[20];
};
```

The message payload contains the nick from client that left.

#### TAI and TOS Message

I'm an idiot, so I don't want PING and PONG. `TAI` and `TOS` mean something like 
`Are there?` and `I am` respectively in Portuguese.

The server sends the `TAI` message and the client must reply with `TOS` in at least 
10 seconds, otherwise, the server consider the client disconnected.

#### MSG Message

A `MSG` message can be sent both by client and server. 

In client POV, the message is sent when the client sends a new chat message. In 
other hand, when the server send the `MSG` message, is to tell the client that a 
new message arrive.

```c
struct MSGMessage {
    u8 author_id;
    char nick[20];
    char message[235]
};
```

In `MSG` message is require specifies the `author_id` and `nick` to validate if 
client exists in the room and to help UI be able to shows who send the message.

### Connection flow:

```
+--------+                 +--------+                             +---------+
| CLIENT |                 | SERVER |                             | CLIENTS |
+--------+                 +--------+                             +---------+
    ||           CON           ||                                      ||
    || ----------------------> ||      +--------------+                ||
    ||                         || ---> | Check if can |                ||
    ||                         ||      +--------------+                ||
    ||                         ||  YES OR NOT |                        ||
    ||                         || <-----------|                        ||
    ||       [NOT] FCN         ||                                      ||
    || <---------------------- || [YES]  +--------------------+  CON   ||
    ||                         || -----> | Notify all clients | -----> ||
    ||       [YES] SCN         ||        +--------------------+        ||
    || <---------------------- ||                                      ||
    ||                         ||                                      ||
    ||  Connection stablished  ||                                      ||
    || <---------------------> ||                                      ||
```

First the client asks if he can connect. In this basic chat, the only validation 
is if server is already full. 

If client can't connect, just fail and show some message to know that the connection 
cannot be stablished. Otherwise, the server notify all clients that a new client 
connected to the chat.

After that, the connection has been stablished.

### Messaging flow:

```
+--------+                 +--------+                   +---------+
| CLIENT |                 | SERVER |                   | CLIENTS |
+--------+                 +--------+                   +---------+
    ||           MSG           ||                           ||
    || ----------------------> ||      +-----------+  MSG   ||
    ||                         || ---> | Broadcast | -----> ||
    ||                         ||      +-----------+        ||
    ||                         ||                           ||
```

Nothing more simple that, send a message to server e the server broadcast the 
same message to all connected clients.

### Disconnection flow:

```
+--------+                 +--------+                             +---------+
| CLIENT |                 | SERVER |                             | CLIENTS |
+--------+                 +--------+                             +---------+
    ||       Disconnected      ||                                      ||
    || <---------/ /---------> ||                                      ||
    ||                         ||        +--------------------+  DIS   ||
    ||                         || -----> | Notify all clients | -----> ||
    ||                         ||        +--------------------+        ||
    ||                         ||                                      ||
```

The server knows when a client closes the connection and notify all other clients 
that a client left the server.

## 🎳 Building

First, make sure you have the `make` command installed. Also, I'm using the `cc` 
compiler, in which many system already have, but if your not, just install it or 
use any of your preference and update the `CC` variable in `Makefile`.

To build both server and client at once, just run the command:
```sh
$ make
```

Otherwise, if you want to build each one individually:
```sh
$ make server
$ make client
```
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

- [ ] `CON`: Request to connect;
- [ ] `SCN`: Success to connect;
- [ ] `FCN`: Fail to connect;
- [ ] `DIS`: Request to disconnect;
- [ ] `TAI`: _Taí?_;
- [ ] `TOS`: _Tô sim_;
- [ ] `MSG`: Chat message;

### Connection flow:

```
+--------+                 +--------+
| CLIENT |                 | SERVER |
+--------+                 +--------+
    ||           CON           ||
    || ----------------------> ||      +--------------+
    ||                         || ---> | CHECK IF CAN |
    ||                         ||      +--------------+
    ||                         ||  YES OR NOT |
    ||                         || <-----------|
    ||       [NOT] FCN         ||
    || <---------------------- || [YES]  +--------------------+
    ||                         || -----> | NOTIFY ALL CLIENTS |
    ||       [YES] SCN         ||        +--------------------+
    || <---------------------- ||
    ||                         ||
    ||  CONNECTION STABLISHED  ||
    || <---------------------> ||
```

First the client asks if he can connect. In this basic chat, the only validation 
is if server is already full. 

If client can't connect, just fail and show some message to know that the connection 
cannot be stablished. Otherwise, the server notify all clients that a new client 
connected to the chat.

After that, the connection has been stablished.

### Messaging flow:

```
+--------+                 +--------+                 
| CLIENT |                 | SERVER |                 
+--------+                 +--------+                 
    ||           MSG           ||                           
    || ----------------------> ||      +-----------+        
    ||                         || ---> | BROADCAST |        
    ||                         ||      +-----------+
    ||                         ||                           
```

Nothing more simple that, send a message to server e the server broadcast the 
same message to all connected clients.
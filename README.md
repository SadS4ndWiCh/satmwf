<img src=".github/typing.gif" width="300">

# 🏮 satmwf

The SATMWF (Super Application To Message With Friends) is the best software ever 
(wow, who thinks that?) to chat virtually.

## 🏑 How it works

Basically, it has the server that is the primary instance that manages all client 
connections and broadcasts the message from some client to the others.

### Connection flow:

```
+--------+                 +--------+
| CLIENT |                 | SERVER |
+--------+                 +--------+
    ||     CAN I CONNECT?      ||
    || ----------------------> ||      +--------------+
    ||                         || ---> | CHECK IF CAN |
    ||                         ||      +--------------+
    ||                         ||  YES OR NOT ||
    ||                         || <---------- ||
    ||    [NOT] YOU CAN'T      ||
    || <---------------------- || [YES]  +--------------------+
    ||                         || -----> | NOTIFY ALL CLIENTS |
    ||    [YES] SUCCESS        ||        +--------------------+
    || <---------------------- ||
    ||  CONNECTION STABLISHED  ||
    || <---------------------> ||
```

First the client asks if he can connect. In this basic chat, the only validation 
is if server is already full. 

If client can't connect, just fail and show some message to know that the connection 
cannot be stablished. Otherwise, the server notify all clients that a new client 
connected to the chat.

After that, a successfuly connection has been stablished.

### Messaging flow:

```
+--------+                 +--------+                 
| CLIENT |                 | SERVER |                 
+--------+                 +--------+                 
    ||         MESSAGE         ||                           
    || ----------------------> ||      +-----------+        
    ||                         || ---> | BROADCAST |        
    ||          SENDED         ||      +-----------+
    || <---------------------- ||                           
```

Nothing more simple that, send a message to server e the server broadcast the 
same message to all connected clients and reply back to who sent that the message 
are successfuly sent.
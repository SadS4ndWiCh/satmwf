CC := cc
CFLAGS := -Wall -Wextra

SERVERSRCS := $(wildcard ./server/*.c)
CLIENTSRCS := $(wildcard ./client/*.c)

.PHONY: bin server client

all: server client

server: bin $(SERVERSRCS)
	$(CC) $(SERVERSRCS) $(CFLAGS) -o bin/server

client: bin $(CLIENTSRCS)
	$(CC) $(CLIENTSRCS) $(CFLAGS) -o bin/client

bin:
	mkdir -p bin
CC := cc
CFLAGS := -Wall -Wextra
CINCLUDES := -I./includes

SHAREDSRCS := $(wildcard ./shared/*.c)
SERVERSRCS := $(wildcard ./server/*.c)
CLIENTSRCS := $(wildcard ./client/*.c)

.PHONY: bin server client

all: server client

server: bin $(SERVERSRCS)
	$(CC) $(SERVERSRCS) $(SHAREDSRCS) $(CFLAGS) $(CINCLUDES) -o bin/server

client: bin $(CLIENTSRCS)
	$(CC) $(CLIENTSRCS) $(SHAREDSRCS) $(CFLAGS) $(CINCLUDES) -o bin/client

bin:
	mkdir -p bin
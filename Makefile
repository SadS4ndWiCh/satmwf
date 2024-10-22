CC := cc
CFLAGS := -Wall -Wextra

SRCS := $(wildcard ./src/*.c)

build: $(SRCS)
	$(CC) $(SRCS) $(CFLAGS) -o satmwf
CC = gcc
CFLAGS = -Wall -Wextra -Wno-unused-parameter -Werror
LIBS = -lcrypto -luv

all: server client

server: server.c
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

client: async_client.c
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f server client

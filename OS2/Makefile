CC = gcc
CFLAGS = -Wall -Wextra -Wno-unused-parameter -Werror
LIBS = -lcrypto -luv

all: server client

server: server.c
	$(CC) $(CFLAGS) -o server server.c $(LIBS)

client: async_client.c
	$(CC) $(CFLAGS) -o client async_client.c $(LIBS)

clean:
	rm -f server client


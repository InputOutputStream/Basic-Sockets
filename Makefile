cc = gcc
CFLAGS = -Wall -Wextra -pedantic -g

all: client server

server: server.c
	cc server.c -o server $(CFLAGS)

client:
	cc client.c -o client $(CFLAGS)

clean:
	rm client server

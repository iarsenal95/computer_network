# Makefile for PA 1

CC = g++
CFLAGS = -g -Wall -Werror -Wno-write-strings

all: client server

client: client.c
	$(CC) $(CFLAGS) -o client.o -c client.c
	$(CC) $(CFLAGS) -o client client.o

server: server.c
	$(CC) $(CFLAGS) -o server.o -c server.c
	$(CC) $(CFLAGS) -o server server.o

clean:
	rm -f client server *.o

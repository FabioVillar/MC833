CCFLAGS=-Wall -g

all: client server

client: src/client.c
	$(CC) -o $@ $< $(CCFLAGS)

server: src/server.c
	$(CC) -o $@ $< $(CCFLAGS)

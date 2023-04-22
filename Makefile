CCFLAGS=-Wall

all: client server

client: client.c
	$(CC) -o $@ $< $(CCFLAGS)

server: server.c
	$(CC) -o $@ $< $(CCFLAGS)

CCFLAGS=-Wall -g

CLIENT_SRC=$(wildcard src/client/**.c)
CLIENT_OBJ=$(CLIENT_SRC:src/%.c=build/%.o)
CLIENT_DEP=$(CLIENT_OBJ:%.o=%.d)

SERVER_SRC=$(wildcard src/server/**.c)
SERVER_OBJ=$(SERVER_SRC:src/%.c=build/%.o)
SERVER_DEP=$(SERVER_OBJ:%.o=%.d)

all: client server

client: $(CLIENT_OBJ)
	$(CC) -o $@ $< $(CCFLAGS)

server: $(SERVER_OBJ)
	$(CC) -o $@ $< $(CCFLAGS)

clean:
	rm -rf build

-include $(CLIENT_DEP) $(SERVER_DEP)

build/%.o: src/%.c
	mkdir -p $(@D)
	$(CC) -o $@ $< $(CCFLAGS) -MMD -c

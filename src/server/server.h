#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>

typedef struct Server Server;

typedef struct {
    struct sockaddr_in address;
    char *cmd;
    void *param;
    int paramSize;
} Message;

Message *message_new(const struct sockaddr_in *address, const char *cmd,
                     const void *param, int paramSize);

void message_free(Message *message);

Server *server_new(int port);

void server_free(Server *client);

/// Send a message and waits for acknowledgement.
/// Error: returns -1
int server_sendMessage(Server *server, const Message *message);

/// Receives a message. cmd must have size at least 8.
/// Error: returns null
Message *server_recvMessage(Server *server);

#endif

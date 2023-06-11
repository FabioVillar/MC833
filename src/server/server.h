#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>

typedef struct Server Server;

Server *server_new(int port);

void server_free(Server *client);

/// Send a message and waits for acknowledgement.
/// Error: returns -1
int server_sendMessage(Server *server, const struct sockaddr_in *address,
                       const char *cmd, const void *param, int paramSize);

/// Receives a message. cmd must have size at least 8.
/// Error: returns -1
int server_recvMessage(Server *server, struct sockaddr_in *address, char *cmd,
                       void *param, int *paramSize);

#endif

#ifndef CLIENT_H
#define CLIENT_H

typedef struct Client Client;

Client *client_new(const char *ip, int port);

void client_free(Client *client);

/// Send a message and waits for acknowledgement.
/// Error: returns -1
int client_sendMessage(Client *client, const char *cmd, const void *param,
                       int paramSize);

/// Receives a message. cmd must have size at least 8.
/// Error: returns -1
int client_recvMessage(Client *client, char *cmd, void *param, int paramSize);

#endif

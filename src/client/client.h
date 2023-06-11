#ifndef CLIENT_H
#define CLIENT_H

typedef struct Client Client;

typedef struct {
    char *cmd;
    void *param;
    int paramSize;
} Message;

Message *message_new(const char *cmd, const void *param, int paramSize);

void message_free(Message *message);

Client *client_new(const char *ip, int port);

void client_free(Client *client);

/// Send a message and waits for acknowledgement.
/// Error: returns -1
int client_sendMessage(Client *client, const Message *message);

/// Receives a message. cmd must have size at least 8.
/// Error: returns null
Message *client_recvMessage(Client *client);

#endif

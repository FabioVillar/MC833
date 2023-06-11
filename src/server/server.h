#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>

typedef struct Server Server;

typedef struct {
    struct sockaddr_in address;
    int msgId;
    char *cmd;
    char *data;
} Request;

void request_free(Request *request);

/// Get the command from a request.

Server *server_new(int port);

void server_free(Server *client);

/// Receive a request from a client.
/// Error: returns null
Request *server_recvRequest(Server *server);

/// Send a response to a client.
/// Error: returns -1
int server_sendResponse(Server *server, const Request *request,
                        const void *data, int dataSize);

int server_sendResponse_str(Server *server, const Request *request,
                           const char *data);

#endif

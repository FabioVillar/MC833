#ifndef CLIENT_SOCKET_H
#define CLIENT_SOCKET_H

typedef struct ClientSocket ClientSocket;

typedef struct Response Response;

void response_free(Response *response);

/// Get the string from the response.
const char *response_getString(Response *response);

ClientSocket *clientsocket_new(const char *ip, int port);

void clientsocket_free(ClientSocket *client);

/// Sends a request and waits for response.
/// Error: returns null
Response *clientsocket_sendRequest(ClientSocket *client, const char *cmd, const char *data);

#endif

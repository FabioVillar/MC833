#ifndef CLIENT_H
#define CLIENT_H

typedef struct Client Client;

typedef struct Response Response;

void response_free(Response *response);

/// Get the string from the response.
const char *response_getString(Response *response);

Client *client_new(const char *ip, int port);

void client_free(Client *client);

/// Sends a request and waits for response.
/// Error: returns null
Response *client_sendRequest(Client *client, const char *cmd, const char *data);

#endif

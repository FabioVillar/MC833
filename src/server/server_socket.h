#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

typedef struct ServerSocket ServerSocket;

typedef struct Request Request;

void request_free(Request *request);

const char *request_getCmd(const Request *request);

/// Get the string from the request.
const char *request_getString(const Request *request);

/// Get the bytes from the request.
void request_getData(const Request *request, const void **bytes, int *size);

ServerSocket *serversocket_new(int port);

void serversocket_free(ServerSocket *client);

/// Receive a request from a client.
/// Error: returns null
Request *serversocket_recvRequest(ServerSocket *server);

/// Send a response to a client.
/// Error: returns -1
int serversocket_sendResponse(ServerSocket *server, const Request *request,
                        const void *data, int dataSize);

int serversocket_sendResponse_str(ServerSocket *server, const Request *request,
                            const char *data);

#endif

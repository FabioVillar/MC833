#define _GNU_SOURCE  // enables strdup

#include "server_socket.h"

#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define BUFFER_SIZE 64000
#define CMD_MAX 32
#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

struct ServerSocket {
    int fd;
    char buffer[BUFFER_SIZE];
};

struct Request {
    struct sockaddr_in address;
    int msgId;
    char cmd[CMD_MAX];
    int dataSize;
    char data[];
};

void request_free(Request *request) { free(request); }

const char *request_getCmd(const Request *request) { return request->cmd; }

const char *request_getString(const Request *request) {
    if (memchr(request->data, '\0', request->dataSize)) {
        return (const char *)request->data;
    } else {
        return NULL;
    }
}

void request_getData(const Request *request, const void **bytes, int *size) {
    *bytes = request->data;
    *size = request->dataSize;
}

ServerSocket *serversocket_new(int port) {
    // assign IP, PORT
    struct sockaddr_in servaddr = {};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = 0;
    servaddr.sin_port = htons(port);

    // socket create and verification
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) exit(-1);
    if (bind(fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
        printf("Bind failed.\n");
        exit(-1);
    }

    struct timeval tv;
    tv.tv_sec = 1;  // Timeout is less than client timeout
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    ServerSocket *server = calloc(1, sizeof(ServerSocket));
    if (!server) exit(-1);

    server->fd = fd;

    return server;
}

void serversocket_free(ServerSocket *server) {
    close(server->fd);
    free(server);
}

Request *serversocket_recvRequest(ServerSocket *server) {
    struct sockaddr_in address;

    for (;;) {
        socklen_t addressSize = sizeof(struct sockaddr_in);
        int r = recvfrom(server->fd, server->buffer, BUFFER_SIZE, 0,
                         (struct sockaddr *)&address, &addressSize);
        if (r < 0) {
            switch (errno) {
            case EAGAIN:
            case ETIMEDOUT:
                continue;
            default:
                printf("%s\n", strerror(errno));
                return NULL;
            }
        }

        // Not a string: ignore
        if (!memchr(server->buffer, '\0', r)) continue;
        DEBUG_PRINT("server_recvMessage: recv(%s ...)\n", server->buffer);

        const char *header = server->buffer;
        int headerlen = strlen(header) + 1;

        const void *data = header + headerlen;
        int dataSize = r - headerlen;

        int msgId;
        char cmd[CMD_MAX];
        sscanf(header, "%x %31s", &msgId, cmd);

        Request *request = malloc(sizeof(Request) + dataSize);
        if (!request) exit(-1);
        memcpy(&request->address, &address, sizeof(struct sockaddr_in));
        request->msgId = msgId;
        strcpy(request->cmd, cmd);
        memcpy(request->data, data, dataSize);
        request->dataSize = dataSize;
        return request;
    }
}

int serversocket_sendResponse(ServerSocket *server, const Request *request,
                        const void *data, int dataSize) {
    int headerlen = sprintf(server->buffer, "%08x", request->msgId) + 1;
    if (headerlen + dataSize > BUFFER_SIZE) return -1;

    memcpy(&server->buffer[headerlen], data, dataSize);

    DEBUG_PRINT("serversocket_sendResponse: send(%s ...)\n", server->buffer);
    return sendto(server->fd, server->buffer, headerlen + dataSize, 0,
                  (const struct sockaddr *)&request->address,
                  sizeof(struct sockaddr_in));
}

int serversocket_sendResponse_str(ServerSocket *server, const Request *request,
                            const char *data) {
    return serversocket_sendResponse(server, request, data, strlen(data) + 1);
}

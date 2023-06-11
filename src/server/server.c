#define _GNU_SOURCE  // enables strdup

#include "server.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define BUFFER_SIZE 32000
#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

struct Server {
    int fd;
    char buffer[BUFFER_SIZE];
};

void request_free(Request *request) {
    free(request->cmd);
    free(request->data);
    free(request);
}

Server *server_new(int port) {
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

    Server *server = calloc(1, sizeof(Server));
    if (!server) exit(-1);

    server->fd = fd;

    return server;
}

void server_free(Server *server) {
    close(server->fd);
    free(server);
}

Request *server_recvRequest(Server *server) {
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

        const char *data = header + headerlen;
        // Not a string: ignore
        if (!memchr(data, '\0', r - headerlen)) continue;

        int msgId;
        char cmd[32];
        sscanf(header, "%x %31s", &msgId, cmd);

        Request *request = calloc(sizeof(Request), 1);
        if (!request) exit(-1);
        memcpy(&request->address, &address, sizeof(struct sockaddr_in));
        request->msgId = msgId;
        request->cmd = strdup(cmd);
        if (!request->cmd) exit(-1);
        request->data = strdup(data);
        if (!request->data) exit(-1);
        return request;
    }
}

int server_sendResponse(Server *server, const Request *request,
                        const void *data, int dataSize) {
    int headerlen = sprintf(server->buffer, "%08x", request->msgId) + 1;
    if (headerlen + dataSize > BUFFER_SIZE) return -1;

    memcpy(&server->buffer[headerlen], data, dataSize);

    DEBUG_PRINT("server_sendResponse: send(%s ...)\n", server->buffer);
    return sendto(server->fd, server->buffer, headerlen + dataSize, 0,
                  (const struct sockaddr *)&request->address,
                  sizeof(struct sockaddr_in));
}

int server_sendResponse_str(Server *server, const Request *request,
                        const char *data) {
    return server_sendResponse(server, request, data, strlen(data) + 1);
}

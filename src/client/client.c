#define _GNU_SOURCE  // enables strdup

#include "client.h"

#include <arpa/inet.h>
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

struct Client {
    int fd;
    int nextMsgId;
    char sendBuffer[BUFFER_SIZE];
    char recvBuffer[BUFFER_SIZE];
};

struct Response {
    int dataSize;
    char data[];
};

void response_free(Response *response) { free(response); }

const char *response_getString(Response *response) {
    if (response->data[response->dataSize - 1] == '\0') {
        return response->data;
    } else {
        return "INVALID\n";
    }
}

Client *client_new(const char *ip, int port) {
    // assign IP, PORT
    struct sockaddr_in servaddr = {};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port = htons(port);

    // socket create and verification
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) exit(-1);
    if (connect(fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
        exit(-1);
    }

    struct timeval tv;
    tv.tv_sec = 30;
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    Client *client = calloc(1, sizeof(Client));
    if (!client) exit(-1);

    client->fd = fd;

    return client;
}

void client_free(Client *client) {
    close(client->fd);
    free(client);
}

Response *client_sendRequest(Client *client, const char *cmd,
                             const char *data) {
    int msgId = (client->nextMsgId++) & 0xFFFFFFFF;

    int requestHeaderSize =
        sprintf(client->sendBuffer, "%08x %s", msgId, cmd) + 1;
    if (requestHeaderSize > BUFFER_SIZE) return NULL;

    int size = requestHeaderSize;
    if (data) {
        strncpy(&client->sendBuffer[requestHeaderSize], data,
                BUFFER_SIZE - requestHeaderSize);
        size += strlen(data) + 1;
    }

    for (;;) {
        DEBUG_PRINT("client_sendMessage: send(%s ...)\n", client->sendBuffer);
        int r = send(client->fd, client->sendBuffer, size, 0);
        if (r < 0) return NULL;

        r = recv(client->fd, client->recvBuffer, BUFFER_SIZE, 0);
        if (r < 0) {
            DEBUG_PRINT("client_sendMessage: %s\n", strerror(errno));
            if (errno == ETIMEDOUT) {
                continue;
            } else {
                return NULL;
            }
        }

        // Not a string: ignore
        if (!memchr(client->recvBuffer, '\0', r)) {
            DEBUG_PRINT("client_sendMessage: recv not a string\n");
            continue;
        }
        DEBUG_PRINT("client_sendMessage: recv(%s ...)\n", client->recvBuffer);

        int receivedMsgId;
        sscanf(client->recvBuffer, "%x", &receivedMsgId);

        int responseHeaderSize = strlen(client->recvBuffer) + 1;
        int dataSize = r - responseHeaderSize;

        Response *response = malloc(sizeof(Response) + dataSize);
        if (!response) exit(-1);

        response->dataSize = dataSize;
        memcpy(response->data, &client->recvBuffer[responseHeaderSize],
               dataSize);
        return response;
    }
}

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
//#define DEBUG

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

int client_sendMessage(Client *client, const char *cmd, const void *param,
                       int paramSize) {
    int r;
    char receivedCmd[8];

    int msgId = (client->nextMsgId++) & 0xFFFFFFFF;

    r = sprintf(client->sendBuffer, "%08x %s", msgId, cmd);

    int size = r + 1 + paramSize;
    if (size > BUFFER_SIZE) return -1;

    memcpy(&client->sendBuffer[r + 1], param, paramSize);

    for (;;) {
        DEBUG_PRINT("client_sendMessage: send(%s ...)\n", client->sendBuffer);
        r = send(client->fd, client->sendBuffer, size, 0);
        if (r < 0) return -1;

        r = recv(client->fd, client->recvBuffer, BUFFER_SIZE, 0);
        if (r < 0) {
            DEBUG_PRINT("client_sendMessage: %s\n", strerror(errno));
            if (errno == ETIMEDOUT) {
                continue;
            } else {
                return -1;
            }
        }

        // Not a string: ignore
        if (!memchr(client->recvBuffer, '\0', r)) {
            DEBUG_PRINT("client_sendMessage: recv not a string\n");
            continue;
        }
        DEBUG_PRINT("client_sendMessage: recv(%s ...)\n", client->recvBuffer);

        int receivedMsgId;
        sscanf(client->recvBuffer, "%x %7s", &receivedMsgId, receivedCmd);

        if (strcmp(receivedCmd, "ack") == 0) {
            if (receivedMsgId == msgId) {
                return 0;
            }
        }
    }
}

int client_recvMessage(Client *client, char *cmd, void *param, int paramSize) {
    int r;

    // Retry at most 8 times
    for (int i = 0; i < 8; i++) {
        r = recv(client->fd, client->recvBuffer, BUFFER_SIZE, 0);
        if (r < 0) {
            if (errno == ETIMEDOUT) {
                continue;
            } else {
                return -1;
            }
        }

        // Not a string: ignore
        if (!memchr(client->recvBuffer, '\0', r)) continue;

        int msgId;
        sscanf(client->recvBuffer, "%x %7s", &msgId, cmd);

        if (strcmp(cmd, "ack") == 0) {
            continue;
        }

        int dataSize = r - 13;
        if (dataSize > paramSize) return -1;
        memcpy(param, &client->recvBuffer[13], dataSize);

        int sendSize = sprintf(client->sendBuffer, "%08x ack", msgId) + 1;
        r = send(client->fd, client->sendBuffer, sendSize, 0);
        if (r < 0) return -1;

        return 0;
    }

    return -1;
}

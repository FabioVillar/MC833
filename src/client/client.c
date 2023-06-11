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

Message *message_new(const char *cmd, const void *param, int paramSize) {
    char *cmd2 = strdup(cmd);
    void *param2 = malloc(paramSize);
    if (!cmd2 || !param2) exit(-1);

    memcpy(param2, param, paramSize);

    Message *message = calloc(sizeof(Message), 1);
    if (!message) exit(-1);

    message->cmd = cmd2;
    message->param = param2;
    message->paramSize = paramSize;
    
    return message;
}

void message_free(Message *message) {
    free(message->cmd);
    free(message->param);
    free(message);
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

int client_sendMessage(Client *client, const Message *message) {
    int r;
    char receivedCmd[32];

    int msgId = (client->nextMsgId++) & 0xFFFFFFFF;

    r = sprintf(client->sendBuffer, "%08x %s", msgId, message->cmd);

    int size = r + 1 + message->paramSize;
    if (size > BUFFER_SIZE) return -1;

    memcpy(&client->sendBuffer[r + 1], message->param, message->paramSize);

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
        sscanf(client->recvBuffer, "%x %31s", &receivedMsgId, receivedCmd);

        if (strcmp(receivedCmd, "ack") == 0) {
            if (receivedMsgId == msgId) {
                return 0;
            }
        }
    }
}

Message *client_recvMessage(Client *client) {
    int r;
    char cmd[32];

    // Retry at most 8 times
    for (int i = 0; i < 8; i++) {
        r = recv(client->fd, client->recvBuffer, BUFFER_SIZE, 0);
        if (r < 0) {
            if (errno == ETIMEDOUT) {
                continue;
            } else {
                return NULL;
            }
        }

        // Not a string: ignore
        if (!memchr(client->recvBuffer, '\0', r)) continue;

        const char *header = client->recvBuffer;
        int headerlen = strlen(header) + 1;

        int msgId;
        sscanf(header, "%x %31s", &msgId, cmd);

        if (strcmp(cmd, "ack") == 0) {
            continue;
        }

        int dataSize = r - headerlen;
        if (dataSize < 0) return NULL;

        int sendSize = sprintf(client->sendBuffer, "%08x ack", msgId) + 1;
        r = send(client->fd, client->sendBuffer, sendSize, 0);
        if (r < 0) return NULL;

        return message_new(cmd, header + headerlen, dataSize);
    }

    return NULL;
}

#include "server.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define BUFFER_SIZE 32000

struct Server {
    int fd;
    int nextMsgId;
    char sendBuffer[BUFFER_SIZE];
    char recvBuffer[BUFFER_SIZE];
};

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

int server_sendMessage(Server *server, const struct sockaddr_in *address,
                       const char *cmd, const void *param, int paramSize) {
    int r;
    char receivedCmd[8];

    int msgId = (server->nextMsgId++) & 0xFFFFFFFF;

    r = sprintf(server->sendBuffer, "%08x %s", msgId, cmd);

    int size = r + 1 + paramSize;
    if (size > BUFFER_SIZE) return -1;

    memcpy(&server->sendBuffer[r + 1], param, paramSize);

    for (int i = 0; i < 8; i++) {
        r = sendto(server->fd, server->sendBuffer, size, 0,
                   (const struct sockaddr *)address,
                   sizeof(struct sockaddr_in));
        if (r < 0) return -1;

        struct sockaddr_in recvAddress;
        socklen_t recvAddressSize = sizeof(struct sockaddr_in);
        r = recvfrom(server->fd, server->recvBuffer, BUFFER_SIZE, 0,
                     (struct sockaddr *)&recvAddress, &recvAddressSize);
        if (r < 0) {
            if (errno == ETIMEDOUT) {
                continue;
            } else {
                return -1;
            }
        }

        // Not a string: ignore
        if (!memchr(server->recvBuffer, '\0', r)) continue;

        int receivedMsgId;
        sscanf(server->recvBuffer, "%x %7s", &receivedMsgId, receivedCmd);

        if (strcmp(receivedCmd, "ack") == 0) {
            if (receivedMsgId == msgId) {
                return 0;
            }
        }
    }

    return -1;
}

int server_recvMessage(Server *server, struct sockaddr_in *address, char *cmd,
                       void *param, int paramSize) {
    int r = 0;

    for (int i = 0; i < 8; i++) {
        socklen_t addressSize = sizeof(struct sockaddr_in);
        r = recvfrom(server->fd, server->recvBuffer, BUFFER_SIZE, 0, address,
                     &addressSize);
        if (r < 0) {
            if (errno == ETIMEDOUT) {
                continue;
            } else {
                return -1;
            }
        }

        // Not a string: ignore
        if (!memchr(server->recvBuffer, '\0', r)) continue;

        int msgId;
        sscanf(server->recvBuffer, "%x %7s", &msgId, cmd);

        if (strcmp(cmd, "ack") == 0) {
            continue;
        }

        int dataSize = r - 13;
        if (dataSize > paramSize) return -1;
        memcpy(param, &server->recvBuffer[13], dataSize);

        int sendSize = sprintf(server->sendBuffer, "%08x ack", msgId);
        r = send(server->fd, server->sendBuffer, sendSize, 0);
        if (r < 0) return -1;

        return 0;
    }

    return -1;
}

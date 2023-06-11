#include "server.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define BUFFER_SIZE 32000
// #define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

struct Server {
    int fd;
    int nextMsgId;
    char sendBuffer[BUFFER_SIZE];
    char recvBuffer[BUFFER_SIZE];
};

Message *message_new(const struct sockaddr_in *address, const char *cmd,
                     const void *param, int paramSize) {
    char *cmd2 = strdup(cmd);
    void *param2 = malloc(paramSize);
    if (!cmd2 || !param2) exit(-1);

    memcpy(param2, param, paramSize);

    Message *message = calloc(sizeof(Message), 1);
    if (!message) exit(-1);

    message->cmd = cmd2;
    message->param = param2;
    message->paramSize = paramSize;

    memcpy(&message->address, address, sizeof(struct sockaddr_in));

    return message;
}

void message_free(Message *message) {
    free(message->cmd);
    free(message->param);
    free(message);
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

int server_sendMessage(Server *server, const Message *message) {
    int r;
    char receivedCmd[32];

    int msgId = (server->nextMsgId++) & 0xFFFFFFFF;

    r = sprintf(server->sendBuffer, "%08x %s", msgId, message->cmd);

    int size = r + 1 + message->paramSize;
    if (size > BUFFER_SIZE) return -1;

    memcpy(&server->sendBuffer[r + 1], message->param, message->paramSize);

    for (int i = 0; i < 8; i++) {
        DEBUG_PRINT("server_sendMessage: send(%s ...)\n", server->sendBuffer);
        r = sendto(server->fd, server->sendBuffer, size, 0,
                   (const struct sockaddr *)&message->address,
                   sizeof(struct sockaddr_in));
        if (r < 0) return -1;

        for (;;) {
            struct sockaddr_in recvAddress;
            socklen_t recvAddressSize = sizeof(struct sockaddr_in);

            r = recvfrom(server->fd, server->recvBuffer, BUFFER_SIZE, 0,
                         (struct sockaddr *)&recvAddress, &recvAddressSize);

            if (r < 0) break;

            if (recvAddress.sin_addr.s_addr !=
                    message->address.sin_addr.s_addr ||
                recvAddress.sin_port != message->address.sin_port) {
                // Message was from other address
                // Keep listening
                continue;
            }
        }

        if (r < 0) {
            if (errno == ETIMEDOUT) {
                continue;
            } else {
                return -1;
            }
        }

        // Not a string: ignore
        if (!memchr(server->recvBuffer, '\0', r)) continue;
        DEBUG_PRINT("server_sendMessage: recv(%s ...)\n", server->recvBuffer);

        int receivedMsgId;
        sscanf(server->recvBuffer, "%x %31s", &receivedMsgId, receivedCmd);

        if (strcmp(receivedCmd, "ack") == 0) {
            if (receivedMsgId == msgId) {
                return 0;
            }
        }
    }

    return -1;
}

Message *server_recvMessage(Server *server) {
    struct sockaddr_in address;
    int r = 0;
    char cmd[32];

    for (int i = 0; i < 8; i++) {
        socklen_t addressSize = sizeof(struct sockaddr_in);
        r = recvfrom(server->fd, server->recvBuffer, BUFFER_SIZE, 0,
                     (struct sockaddr *)&address, &addressSize);
        if (r < 0) {
            switch (errno) {
            case EAGAIN:
            case ETIMEDOUT:
                continue;
            default:
                return NULL;
            }
        }

        // Not a string: ignore
        if (!memchr(server->recvBuffer, '\0', r)) continue;
        DEBUG_PRINT("server_recvMessage: recv(%s ...)\n", server->recvBuffer);

        const char *header = server->recvBuffer;
        int headerlen = strlen(header) + 1;

        int msgId;
        sscanf(header, "%x %31s", &msgId, cmd);

        if (strcmp(cmd, "ack") == 0) {
            continue;
        }

        int dataSize = r - headerlen;
        if (dataSize < 0) return NULL;

        int sendSize = sprintf(server->sendBuffer, "%08x ack", msgId) + 1;
        r = sendto(server->fd, server->sendBuffer, sendSize, 0,
                   (struct sockaddr *)&address, addressSize);
        DEBUG_PRINT("server_recvMessage: send(%s ...)\n", server->sendBuffer);
        if (r < 0) return NULL;

        return message_new(&address, cmd, header + headerlen, dataSize);
    }

    return NULL;
}

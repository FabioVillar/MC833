#include <arpa/inet.h>  // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>  // bzero()
#include <sys/socket.h>
#include <unistd.h>  // read(), write(), close()

// #define DEBUG
#define BUFFER_SIZE 1024
#define DEFAULT_PORT 8082
#define SA struct sockaddr

#define CMD_PRINT "print"
#define CMD_INPUT "input"

#ifdef DEBUG
#define DEBUG_PRINT(...) printf("** DEBUG ** " __VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

typedef struct {
    int fd;
    // Buffer circular
    char receiveBuffer[BUFFER_SIZE];
    int receiveBufferFilled;
} Client;

/// Sends a string to the server.
///
/// Returns less than zero in case of error.
static int sendString(int fd, const char *buf) {
    DEBUG_PRINT("Sending: %s\n", buf);

    // send strlen(buf) plus the null terminator
    int size = strlen(buf) + 1;
    int sent = 0;
    while (sent < size) {
        int r = write(fd, &buf[sent], size - sent);
        if (r <= 0) return r;
        // we sent "r" bytes
        sent += r;
    }

    return size;
}

/// Receives a string from the server.
///
/// Returns less than zero in case of error.
static int receiveString(Client *client, char *buf, int bufSize) {
    for (;;) {
        if (client->receiveBufferFilled != 0) {
            // check if we received a null terminator
            char *terminator = memchr(client->receiveBuffer, '\0', BUFFER_SIZE);
            if (terminator != NULL) {
                int terminatorIndex = terminator - client->receiveBuffer;
                strncpy(buf, client->receiveBuffer, bufSize);
                memmove(client->receiveBuffer,
                        client->receiveBuffer + terminatorIndex + 1,
                        BUFFER_SIZE - terminatorIndex);
                client->receiveBufferFilled -= terminatorIndex + 1;

                DEBUG_PRINT("Received: %s\n", buf);

                return terminatorIndex - 1;
            }
        }

        DEBUG_PRINT("Waiting for server\n");

        int r = read(client->fd,
                &client->receiveBuffer[client->receiveBufferFilled],
                BUFFER_SIZE - client->receiveBufferFilled);
        if (r <= 0) return r;

        // we read "r" bytes
        client->receiveBufferFilled += r;
    }
}

/// Read user input until newline.
static void stdinLine(char *buf, int size) {
    int i = 0;

    for (;;) {
        char c = getchar();

        if (c == '\n' || c == EOF) {
            buf[i] = '\0';
            return;
        }

        if (i < size - 1) {
            buf[i] = c;
            i++;
        }
    }
}

/// Executes the client. Returns non-zero if an error occurred.
static int runClient(int sockfd) {
    Client client;
    char buf[BUFFER_SIZE];
    char cmd[BUFFER_SIZE];
    char param[BUFFER_SIZE];
    int r;

    client.fd = sockfd;
    client.receiveBufferFilled = 0;

    for (;;) {
        if ((r = receiveString(&client, buf, BUFFER_SIZE)) <= 0) {
            return r;
        }

        // Separate first word in cmd and rest in
        // param
        char *firstSpace = strchr(buf, ' ');
        if (firstSpace != NULL) {
            int firstSpaceIndex = firstSpace - buf;
            strncpy(cmd, buf, firstSpaceIndex);
            strncpy(param, firstSpace + 1, BUFFER_SIZE);
        } else {
            strncpy(cmd, buf, BUFFER_SIZE);
            param[0] = '\0';
        }

        // Decide next action based on cmd
        if (strcmp(cmd, CMD_PRINT) == 0) {
            printf("%s", param);
        } else if (strcmp(cmd, CMD_INPUT) == 0) {
            printf("> ");
            stdinLine(buf, BUFFER_SIZE);
            if ((r = sendString(sockfd, buf)) <= 0) return r;
        } else {
            printf("Unknown command: %s", cmd);
        }
    }
}

int main(int argc, char **argv) {
    int port;
    switch (argc) {
    case 1:
        port = DEFAULT_PORT;
        break;
    case 2:
        port = atoi(argv[1]);
        break;
    default:
        printf("Usage: %s [port]\n", argv[0]);
        return 1;
    }

    // assign IP, PORT
    struct sockaddr_in servaddr = {};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(port);

    for (;;) {
        // socket create and verification
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            printf("Socket creation failed.\n");
            exit(0);
        }

        // try to connect in a loop
        while (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0) {
            printf("Connection with the server failed.\n");
            sleep(1);
        }

        printf("Connected to the server.\n");

        // execute client code
        int r = runClient(sockfd);

        // close the socket
        close(sockfd);

        if (r == 0) {
            // successful exit
            printf("Connection closed\n");
            return 0;
        } else {
            // connection error
            printf("Connection error\n");
        }
    }
}

#include <arpa/inet.h>  // inet_addr()
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>  // read(), write(), close()

// #define DEBUG
#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT 8082

#define BUFFER_SIZE 1024
#define SA struct sockaddr

#define CMD_PRINT "print"
#define CMD_INPUT "input"

#ifdef DEBUG
#define DEBUG_PRINT(...) printf("** DEBUG ** " __VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

/// Reads with a timeout.
static int recvWithTimeout(int fd, int timeout, void *buffer, int size) {
    struct timeval tv;
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    return recv(fd, buffer, size, 0);
}

/// Sends bytes to the server and waits for acknowledgement.
///
/// Returns less than zero in case of error.
static int sendData(int fd, void *buf, int size) {
    char recvBuffer[1];
    int r;

    for (;;) {
        r = send(fd, buf, size, 0);
        if (r <= 0) return r;

        r = recvWithTimeout(fd, 5, recvBuffer, 1);
        if (r == 0) return 1;  // datagram size 0 means it's an ack

        if (errno == ETIMEDOUT) {
            DEBUG_PRINT("Timeout: trying again\n");
        } else {
            printf("Error: %d\n", errno);
            return r;
        }
    }
}

/// Read user input until newline.
static int stdinLine(char *buf, int size) {
    int i = 0;

    for (;;) {
        char c = getchar();

        if (c == '\n' || c == EOF) {
            buf[i] = '\0';
            return i;
        }

        if (i < size - 1) {
            buf[i] = c;
            i++;
        }
    }
}

/// Executes the client. Returns non-zero if an error occurred.
static int runClient(int fd) {
    char buf[BUFFER_SIZE];
    int r;

    printf("Connecting to server\n");

    r = sendData(fd, "connect", 8);
    if (r <= 0) return r;

    printf("Connected\n");

    for (;;) {
        r = recvWithTimeout(fd, 5, buf, BUFFER_SIZE);
        if (r <= 0) return r;

        // buf must contain a nul terminator
        if (!memchr(buf, '\0', r)) {
            return -1;
        }

        const char *cmd = buf;
        int sizeCmd = strlen(cmd);
        const void *param = &buf[sizeCmd + 1];
        int sizeParam = r - sizeCmd - 1;

        // Decide next action based on cmd
        if (strcmp(cmd, CMD_PRINT) == 0) {
            // param must contain a nul terminator
            if (!memchr(param, '\0', sizeParam)) {
                return -1;
            }

            printf("%s", (const char *)param);
        } else if (strcmp(cmd, CMD_INPUT) == 0) {
            if (sizeParam != 0) {
                // param must contain a nul terminator
                if (!memchr(param, '\0', sizeParam)) {
                    return -1;
                }

                printf("%s", (const char *)param);
            }

            printf("> ");
            strcpy(buf, "data");
            int lineSize = stdinLine(&buf[5], BUFFER_SIZE);
            if ((r = sendData(fd, buf, lineSize + 6)) <= 0) return r;
        } else {
            printf("Unknown command: %s", cmd);
        }
    }
}

int main(int argc, char **argv) {
    const char *ip;
    int port;
    switch (argc) {
    case 1:
        ip = DEFAULT_IP;
        port = DEFAULT_PORT;
        break;
    case 3:
        ip = argv[1];
        port = atoi(argv[2]);
        break;
    default:
        printf("Usage: %s [ip] [port]\n", argv[0]);
        return 1;
    }

    // assign IP, PORT
    struct sockaddr_in servaddr = {};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port = htons(port);

    // socket create and verification
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        printf("Socket creation failed.\n");
        exit(0);
    }

    for (;;) {
        // try to connect in a loop
        if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0) {
            printf("Connection with the server failed.\n");
            exit(0);
        }

        // execute client code
        int r = runClient(sockfd);

        if (r < 0) {
            send(sockfd, "close", 6, 0);
            printf("Connection error\n");
            sleep(1);
        } else {
            printf("Connection closed\n");
            break;
        }
    }

    // close the socket
    close(sockfd);

    return 0;
}

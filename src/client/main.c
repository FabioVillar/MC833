#include <arpa/inet.h>  // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>  // bzero()
#include <sys/socket.h>
#include <unistd.h>  // read(), write(), close()

// #define DEBUG
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8082

#define BUFFER_SIZE 1024
#define SA struct sockaddr

#define CMD_PRINT "print"
#define CMD_INPUT "input"

#ifdef DEBUG
#define DEBUG_PRINT(...) printf("** DEBUG ** " __VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

/// Sends bytes to the server and waits for acknowledgement.
///
/// Returns less than zero in case of error.
static int sendData(int fd, void *buf, int size) {
    char recvBuffer[1];
    int r;

    r = send(fd, buf, size, 0);
    if (r <= 0) return r;

    r = recv(fd, recvBuffer, 1, 0);
    if (r == 0) return 1; // datagram size 0 means it's an ack

    return -1;
}

/// Receives bytes from the server.
static int receiveData(int fd, void *buf, int bufSize) {
    int readSize;
    int r;

    r = recv(fd, buf, bufSize, 0);
    if (r <= 0) return r;
    readSize = r;
    
    // send ack
    // datagram size 0 means it's an ack
    r = send(fd, NULL, 0, 0);
    if (r < 0) return r;

    return readSize;
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
    char cmd[BUFFER_SIZE];
    char param[BUFFER_SIZE];
    int r;

    r = sendData(fd, "connect", 8);
    if (r <= 0) return r;

    for (;;) {
        if ((r = receiveData(fd, buf, BUFFER_SIZE)) <= 0) {
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
            int lineSize = stdinLine(buf, BUFFER_SIZE);
            if ((r = sendData(fd, buf, lineSize + 1)) <= 0) return r;
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
        ip = SERVER_IP;
        port = SERVER_PORT;
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
            printf("Connection error\n");
            sleep(1);
        }
    }

    // close the socket
    close(sockfd);

    // successful exit
    printf("Connection closed\n");
    return 0;
}

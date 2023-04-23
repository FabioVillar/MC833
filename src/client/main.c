#include <arpa/inet.h>  // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>  // bzero()
#include <sys/socket.h>
#include <unistd.h>  // read(), write(), close()

#define BUFFER_SIZE 80
#define PORT 8080
#define SA struct sockaddr

/// Sends a string to the server.
///
/// Returns less than zero in case of error.
static int sendString(int fd, const char *buf) {
    printf("Sending: %s\n", buf);

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
static int receiveString(int fd, char *buf, int bufSize) {
    int received = 0;
    for (;;) {
        if (received >= bufSize - 1) {
            // string too large
            return -1;
        }

        int r = read(fd, &buf[received], bufSize - received);
        if (r <= 0) return r;
        // we read "r" bytes
        received += r;

        // check if we received a null terminator
        if (buf[received - 1] == 0) {
            printf("Received: %s\n", buf);
            return received;
        }
    }
}

/// Shows the menu
static void showMenu() {
    char menu[500] =
            "\nMENU\n"
            "Write a number accordingly to what you want:\n"
            "1 - Insert a new profile in the system\n"
            "2 - List all people graduated in a specific course\n"
            "3 - List all people graduated in a specific year\n"
            "4 - Liss all informations of all profiles\n"
            "5 - Given an email, list all information of it\n"
            "6 - Given an email, remove a profile\n";
    printf("%s", menu);
}

/// Executes the client. Returns non-zero if an error occurred.
static int runClient(int sockfd) {
    char buf[BUFFER_SIZE];
    int r;

    for (;;) {
        if ((r = receiveString(sockfd, buf, BUFFER_SIZE)) <= 0) return r;
        if ((strcmp(buf, "exit")) == 0) {
            printf("Client Exit...\n");
            return 0;
        } else if ((strcmp(buf, "menu")) == 0) {
            showMenu();
        }
        
        scanf("%79s", buf);
        if ((r = sendString(sockfd, buf)) <= 0) return r;
    }
}

int main() {
    // assign IP, PORT
    struct sockaddr_in servaddr = {};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);

    for (;;) {
        // socket create and verification
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            printf("socket creation failed...\n");
            exit(0);
        }

        // try to connect in a loop
        while (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0) {
            printf("connection with the server failed...\n");
            sleep(1);
        }

        printf("connected to the server..\n");

        // execute client code
        int r = runClient(sockfd);

        // close the socket
        close(sockfd);

        if (r == 0) {
            // successful exit
            printf("connection closed\n");
            return 0;
        } else {
            // connection error
            printf("connection error\n");
        }
    }
}

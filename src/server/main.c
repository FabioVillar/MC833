#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>  // read(), write(), close()

#define BUFFER_SIZE 80
#define PORT 8080
#define SA struct sockaddr

/// Sends a string to the client.
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

/// Receives a string from the client.
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

static int insertProfile(int fd) {
    int r;
    
    sendString(fd, "Insert email");
    char email[50];
    r = receiveString(fd, email, sizeof(email));
    if (r <= 0) return r;

    sendString(fd, "Insert name");
    char name[50];
    r = receiveString(fd, name, sizeof(name));
    if (r <= 0) return r;

    sendString(fd, "Insert last name");
    char lastName[50];
    r = receiveString(fd, lastName, sizeof(lastName));
    if (r <= 0) return r;

    sendString(fd, "Insert your city");
    char city[50];
    r = receiveString(fd, city, sizeof(city));
    if (r <= 0) return r;

    sendString(fd, "Insert your graduation field");
    char graduationField[50];
    r = receiveString(fd, graduationField, sizeof(graduationField));
    if (r <= 0) return r;

    sendString(fd, "Insert graduation year");
    char year[5];
    r = receiveString(fd, year, sizeof(year));
    if (r <= 0) return r;

    sendString(fd, "Insert your skills (Skill1/Skill2/Skill3/etc)");
    char skills[100];
    r = receiveString(fd, skills, sizeof(skills));
    if (r <= 0) return r;

    FILE *fp;
    fp = fopen("profile.txt", "w");
    fprintf(fp, "Profile:\n");
    fprintf(fp, "%s\n", email);
    fprintf(fp, "%s\n", name);
    fprintf(fp, "%s\n", lastName);
    fprintf(fp, "%s\n", city);
    fprintf(fp, "%s\n", graduationField);
    fprintf(fp, "%s\n", year);
    fprintf(fp, "%s\n", skills);
    fclose(fp);
    return 1;
}

static int handleMessage(int fd, const char *message) {
    if ((strcmp(message, "1")) == 0) {
        return insertProfile(fd);
    } else {
        return sendString(fd, "Unknown message");
    }
}

// Executes the client. Returns non-zero if an error occurred.
static int runServer(int connfd) {
    char buf[BUFFER_SIZE];
    int r;

    // infinite loop for chat
    for (;;) {
        if ((r = sendString(connfd, "menu")) <= 0) return r;
        if ((r = receiveString(connfd, buf, BUFFER_SIZE)) <= 0) return r;
        if ((r = handleMessage(connfd, buf)) <= 0) return r;
    }
}

// Thread accepts incoming connections.
static void *serverThread(void *params_) {
    
}

// Driver function
int main() {
    int r;

    // assign IP, PORT
    struct sockaddr_in servaddr = {};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    // socket create and verification
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }

    // Binding newly created socket to given IP and verification
    if (bind(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0) {
        printf("socket bind failed... (%d)\n", errno);
        exit(0);
    }

    // Now server is ready to listen and verification
    if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    } else {
        printf("Server listening..\n");
    }

    for (;;) {
        // Accept the data packet from client and verification
        struct sockaddr_in cli;
        socklen_t len = sizeof(cli);
        int connfd = accept(sockfd, (SA *)&cli, &len);
        if (connfd < 0) {
            printf("server accept failed...\n");
            exit(0);
        } else {
            printf("server accept the client...\n");
        }

        // execute sever chat
        r = runServer(connfd);

        // close the connection
        close(connfd);

        if (r != 0) {
            // connection error
            printf("connection error\n");
        }
    }

    // never reached
    close(sockfd);
    return 0;
}

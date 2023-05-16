#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "chat.h"
#include "chatlist.h"
#include "database.h"

#define BUFFER_SIZE 1024

#define DEFAULT_PORT 8082

static int acceptData(int sockfd, ChatList *chatlist) {
    struct sockaddr_in clientaddr;
    char buf[BUFFER_SIZE];
    int r;
    socklen_t clientaddrSize = sizeof(clientaddr);

    r = recvfrom(sockfd, buf, BUFFER_SIZE, 0, (struct sockaddr *)&clientaddr,
                 &clientaddrSize);
    if (r <= 0) return r;

    Chat *chat =
        chatlist_get(chatlist, clientaddr.sin_addr.s_addr, clientaddr.sin_port);
    chat_handleData(chat, buf, r);

    return 0;
}

// Driver function
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
    servaddr.sin_addr.s_addr = 0;
    servaddr.sin_port = htons(port);

    // socket create and verification
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        printf("Socket creation failed.\n");
        return 1;
    }

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
        printf("Connection with the server failed.\n");
        return 1;
    }

    Database *database = database_new();
    if (!database) return 1;
    database_load(database, DATABASE_FILE);

    ChatList *chatlist = chatlist_new(database);
    acceptData(sockfd, chatlist);
    chatlist_free(chatlist);

    database_free(database);
    close(sockfd);

    return 0;
}

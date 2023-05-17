#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "chat.h"
#include "chatlist.h"
#include "database.h"

#define BUFFER_SIZE 1024

#define DEFAULT_PORT 8082

static void sendAck(int sockfd, struct sockaddr_in *clientaddr) {
    sendto(sockfd, NULL, 0, 0, (struct sockaddr *)&clientaddr,
           sizeof(struct sockaddr_in));
}

static void acceptData(int sockfd, ChatList *chatlist) {
    struct sockaddr_in clientaddr;
    char buf[BUFFER_SIZE];
    int r;
    socklen_t clientaddrSize = sizeof(clientaddr);

    for (;;) {
        r = recvfrom(sockfd, buf, BUFFER_SIZE, 0,
                     (struct sockaddr *)&clientaddr, &clientaddrSize);
        if (r <= 0) return;

        int readSize = r;
        char address[64];
        inet_ntop(clientaddr.sin_family, &clientaddr.sin_addr, address,
                  sizeof(address));
        int addressSize = strlen(address);
        if (addressSize >= 64) return;
        snprintf(&address[addressSize], 64 - addressSize, ":%d",
                 clientaddr.sin_port);

        if (readSize == 8 && memcmp(buf, "connect", 8) == 0) {
            printf("[%s] New connection\n", address);
            chatlist_createChat(chatlist, address);
            sendAck(sockfd, &clientaddr);
        } else if (readSize == 6 && memcmp(buf, "close", 6) == 0) {
            printf("[%s] Closed\n", address);
            chatlist_removeChat(chatlist, address);
        } else if (readSize >= 5 && memcmp(buf, "data", 5)) {
            printf("[%s] Received data\n", address);
            Chat *chat = chatlist_findChat(chatlist, address);
            if (chat) {
                chat_handleData(chat, &buf[5], readSize - 5);
                sendAck(sockfd, &clientaddr);
            }
        } else {
            printf("[%s] Received unknown message\n", address);
        }
    }
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

    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
        printf("Bind failed.\n");
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

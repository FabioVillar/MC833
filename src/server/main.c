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

static void acceptData(int sockfd, ChatList *chatlist, Database *database) {
    struct sockaddr_in clientaddr;
    char buf[BUFFER_SIZE];
    int r;
    socklen_t clientaddrSize = sizeof(clientaddr);

    for (;;) {
        r = recvfrom(sockfd, buf, BUFFER_SIZE, 0,
                     (struct sockaddr *)&clientaddr, &clientaddrSize);
        if (r < 0) return;

        int readSize = r;
        char addressString[64];
        inet_ntop(clientaddr.sin_family, &clientaddr.sin_addr, addressString,
                  sizeof(addressString));
        int addressSize = strlen(addressString);
        if (addressSize >= 64) return;
        snprintf(&addressString[addressSize], 64 - addressSize, ":%d",
                 clientaddr.sin_port);

        if (readSize == 8 && memcmp(buf, "connect", 8) == 0) {
            printf("[%s] New connection\n", addressString);

            Chat *chat = chat_new(sockfd, &clientaddr, addressString, database);
            if (!chat) return;

            chatlist_insertChat(chatlist, chat);
        } else if (readSize == 6 && memcmp(buf, "close", 6) == 0) {
            printf("[%s] Closed\n", addressString);
            chatlist_removeChat(chatlist, addressString);
        } else if (readSize >= 5 && memcmp(buf, "data", 5) == 0) {
            printf("[%s] Received data\n", addressString);
            Chat *chat = chatlist_findChat(chatlist, addressString);
            if (chat) {
                chat_handleData(chat, &buf[5], readSize - 5);
            }
        } else if (readSize == 4 && memcmp(buf, "ack", 4) == 0) {
            printf("[%s] Received acknowledgment\n", addressString);
            Chat *chat = chatlist_findChat(chatlist, addressString);
            if (chat) {
                chat_handleAck(chat);
            }
        } else {
            printf("[%s] Received unknown message\n", addressString);
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

    ChatList *chatlist = chatlist_new();
    acceptData(sockfd, chatlist, database);
    chatlist_free(chatlist);

    database_free(database);
    close(sockfd);

    return 0;
}

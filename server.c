#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>  // read(), write(), close()

#define MAX 80
#define PORT 8080
#define SA struct sockaddr

char *readFromClient(char *buff, int connfd) {
    bzero(buff, MAX);
    // read the message from client and copy it in buffer
    read(connfd, buff, sizeof(buff));
    printf("Client send this message: %s", buff);
    return buff;
}

void sendToClient(char *buff, int connfd) {
    printf("Is gonna send something\n");
    printf("%s\n", buff);
    int n = 0;
    // copy server message in the buffer
    while ((buff[n++] = getchar()) != '\n') {
    }
    // and send that buffer to client
    printf("To client: %s", buff);
    write(connfd, buff, sizeof(buff));
    // if msg contains "Exit" then server exit and chat ended.
    if (strncmp("exit", buff, 4) == 0) {
        printf("Server Exit...\n");
    }
}
void insertProfile(char *buff, int connfd) {
    FILE *fp;
    fp = fopen("profile.txt", "w");
    fprintf(fp, "Profile:");
    strcpy(buff, "Insert-email");
    sendToClient(buff, connfd);
    strcpy(buff, readFromClient(buff, connfd));
    char email[50] = "";
    strcpy(email, buff);

    strcpy(buff, "Insert name");
    sendToClient(buff, connfd);
    strcpy(buff, readFromClient(buff, connfd));
    char name[50] = "";
    strcpy(name, buff);

    strcpy(buff, "Insert last name");
    sendToClient(buff, connfd);
    strcpy(buff, readFromClient(buff, connfd));
    char lastName[50] = "";
    strcpy(lastName, buff);

    strcpy(buff, "Insert your city");
    sendToClient(buff, connfd);
    strcpy(buff, readFromClient(buff, connfd));
    char city[50] = "";
    strcpy(city, buff);

    strcpy(buff, "Insert your graduation field");
    sendToClient(buff, connfd);
    strcpy(buff, readFromClient(buff, connfd));
    char graduationField[50] = "";
    strcpy(graduationField, buff);

    strcpy(buff, "Insert graduation year");
    sendToClient(buff, connfd);
    strcpy(buff, readFromClient(buff, connfd));
    char year[5] = "";
    strcpy(year, buff);

    strcpy(buff, "Insert your skills");
    sendToClient(buff, connfd);
    strcpy(buff, readFromClient(buff, connfd));
    char skills[100] = "";
    strcpy(skills, buff);

    fprintf(fp, "%s", email);

    fclose(fp);
}
void manageBuff(char *buff, int connfd) {
    if (buff[0] == '1') {
        insertProfile(buff, connfd);
    }
}
// Function designed for chat between client and server.
void func(int connfd) {
    char buff[MAX];
    int n;
    // infinite loop for chat
    for (;;) {
        strcpy(readFromClient(buff, connfd), buff);
        manageBuff(buff, connfd);
    }
}

// Driver function
int main() {
    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    } else {
        printf("Socket successfully created..\n");
    }
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    } else {
        printf("Socket successfully binded..\n");
    }

    // Now server is ready to listen and verification
    if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    } else {
        printf("Server listening..\n");
    }
    len = sizeof(cli);

    // Accept the data packet from client and verification
    connfd = accept(sockfd, (SA *)&cli, &len);
    if (connfd < 0) {
        printf("server accept failed...\n");
        exit(0);
    } else {
        printf("server accept the client...\n");
    }

    // Function for chatting between client and server
    func(connfd);

    // After chatting close the socket
    close(sockfd);
}


#include "accept_thread.h"

#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "chat_thread.h"

struct AcceptThread {
    pthread_t thread;
    int fd;
    int port;
};

static void runAccept(AcceptThread *thread) {
    // assign IP, PORT
    struct sockaddr_in servaddr = {};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(thread->port);

    int fd = thread->fd;

    // Binding newly created socket to given IP and verification
    if (bind(fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
        printf("socket bind failed... (%s)\n", strerror(errno));
        return;
    }

    // Now server is ready to listen and verification
    if ((listen(fd, 5)) != 0) {
        printf("Listen failed...\n");
        return;
    }

    printf("Server listening on port %d\n", thread->port);

    for (;;) {
        // Accept the data packet from client and verification
        struct sockaddr_in cli;
        socklen_t len = sizeof(cli);
        int connfd = accept(fd, (struct sockaddr *)&cli, &len);
        if (connfd < 0) {
            printf("server accept failed...\n");
            break;
        }

        printf("[%d] new connection\n", connfd);

        pthread_t chatThread;
        if (startChatThread(&chatThread, connfd) < 0) {
            printf("startChatThread failed...\n");
            close(connfd);
            break;
        }
    }

    // close the socket
    close(fd);
}

static void *acceptThread(void *x) {
    runAccept((AcceptThread *)x);
    free(x);
    return NULL;
}

AcceptThread *startAcceptThread(int port) {
    AcceptThread *thread = calloc(1, sizeof(AcceptThread));
    thread->port = port;

    // socket create and verification
    thread->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (thread->fd == -1) {
        printf("socket creation failed...\n");
        free(thread);
        return NULL;
    }

    pthread_attr_t attr;
    if (pthread_attr_init(&attr) < 0) {
        free(thread);
        return NULL;
    }

    if (pthread_create(&thread->thread, &attr, acceptThread, thread) < 0) {
        free(thread);
        return NULL;
    }

    return thread;
}

void stopAcceptThread(AcceptThread *thread) {
    printf("stopping...\n");

    // Shutdown the socket
    shutdown(thread->fd, SHUT_RDWR);

    pthread_join(thread->thread, NULL);

    printf("stopped\n");
}

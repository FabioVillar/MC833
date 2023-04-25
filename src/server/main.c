#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "accept_thread.h"

#define DEFAULT_PORT 8080

static int shouldStop = 0;

static void signalHandler(int signal) { shouldStop = 1; }

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

    AcceptThread *acceptThread = startAcceptThread(port);
    if (!acceptThread) return 1;

    signal(SIGINT, signalHandler);
    while (!shouldStop) {
        pause();
    }

    stopAcceptThread(acceptThread);

    return 0;
}

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "accept_thread.h"

#define DEFAULT_PORT 8082

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

    Database *database = database_new();
    if (!database) return 1;
    database_load(database, DATABASE_FILE);

    AcceptThread *acceptThread = startAcceptThread(port, database);
    if (!acceptThread) return 1;

    signal(SIGINT, signalHandler);
    while (!shouldStop) {
        pause();
    }

    stopAcceptThread(acceptThread);
    database_free(database);

    return 0;
}

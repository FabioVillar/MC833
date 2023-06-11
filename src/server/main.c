#define _GNU_SOURCE  // enables some functions

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "server.h"

#define DEFAULT_PORT 8082
#define DEFAULT_DIRECTORY "serverdata"

#define BUFFER_SIZE_ 32000

int main(int argc, char **argv) {
    int port = DEFAULT_PORT;
    const char *directory = DEFAULT_DIRECTORY;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 && i < argc - 1) {
            port = atoi(argv[i + 1]);
        } else if (strcmp(argv[i], "-D") == 0 && i < argc - 1) {
            directory = argv[i + 1];
        } else {
            printf("Usage: %s [-p port] [-D directory]\n", argv[0]);
            return 1;
        }
    }

    Server *server = server_new(port, directory);
    server_run(server);
    server_free(server);

    return 0;
}

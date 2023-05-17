#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "database.h"
#include "server.h"

#define DEFAULT_PORT 8082

static void runServer(Server *server, Database *database) {
    char cmd[8] = {};
    char param[1024];
    int r;

    struct sockaddr_in clientaddr;

    for (;;) {
        r = server_recvMessage(server, &clientaddr, cmd, param, sizeof(param));
        if (r < 0) {
            if (errno == ETIMEDOUT) {
                continue;
            } else {
                return;
            }
        }

        if (strcmp(cmd, "insert") == 0) {
        } else if (strcmp(cmd, "listByCourse") == 0) {
        } else if (strcmp(cmd, "listBySkill") == 0) {
        } else if (strcmp(cmd, "listByYear") == 0) {
        } else if (strcmp(cmd, "listAll") == 0) {
        } else if (strcmp(cmd, "listByEmail") == 0) {
        } else if (strcmp(cmd, "removeByEmail") == 0) {
        } else {
            printf("Received unknown message\n");
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

    Server *server = server_new(port);
    Database *database = database_new();
    database_load(database, DATABASE_FILE);

    runServer(server, database);

    database_free(database);
    server_free(server);

    return 0;
}

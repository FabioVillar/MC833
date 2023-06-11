#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "database.h"
#include "server.h"

#define DEFAULT_PORT 8082

/// Iterator is formatted as "<string1>\0<string2>..."
/// - If <string1> exists, return it.
/// - Advance the iterator.
static const char *advanceString(const char **iterator, int *iteratorSize) {
    const char *string = *iterator;
    if (!string) {
        return NULL;
    }

    const char *nextNul = memchr(string, '\0', *iteratorSize);
    if (nextNul) {
        *iterator = nextNul + 1;
        *iteratorSize -= nextNul + 1 - string;
        return string;
    } else {
        *iteratorSize = 0;
        *iterator = NULL;
        return NULL;
    }
}

static void insertProfile(Database *database, const char *param,
                          int paramSize) {
    const char *email = advanceString(&param, &paramSize);
    const char *firstName = advanceString(&param, &paramSize);
    const char *lastName = advanceString(&param, &paramSize);
    const char *city = advanceString(&param, &paramSize);
    const char *graduation = advanceString(&param, &paramSize);
    const char *gradYear = advanceString(&param, &paramSize);
    const char *skills = advanceString(&param, &paramSize);

    if (!email || !firstName || !lastName || !city || !graduation ||
        !gradYear || !skills) {
        return;
    }

    database_addRow(database, email, firstName, lastName, city, graduation,
                    gradYear, skills);
    database_save(database, DATABASE_FILE);
}

static void runServer(Server *server, Database *database) {
    char cmd[8] = {};
    char param[1024];
    int r;

    struct sockaddr_in clientaddr;

    for (;;) {
        int paramSize = sizeof(param);
        r = server_recvMessage(server, &clientaddr, cmd, param, &paramSize);
        if (r < 0) {
            switch (errno) {
            case EAGAIN:
            case ETIMEDOUT:
                continue;
            default:
                printf("%s\n", strerror(errno));
                return;
            }
        }

        if (strcmp(cmd, "insert") == 0) {
            insertProfile(database, param, sizeof(param));
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

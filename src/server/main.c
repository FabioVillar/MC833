#define _GNU_SOURCE  // enables some functions

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "database.h"
#include "server.h"

#define DEFAULT_PORT 8082

#define BUFFER_SIZE 1024

/// Iterate over the lines of a string
static char *nextLine(const char **lineIterator) {
    const char *line = *lineIterator;

    if (!line) return NULL;

    char *newLine = strchr(line, '\n');
    if (newLine) {
        *lineIterator = newLine + 1;
        return strndup(line, newLine - line);
    } else {
        *lineIterator = NULL;
        return strdup(line);
    }
}

/// Print a row as a profile
static int printProfile(char *buffer, int bufferSize, Database *database,
                        struct sockaddr_in *clientaddr, int row) {
    char *email = database_get(database, row, COLUMN_EMAIL);
    char *firstName = database_get(database, row, COLUMN_FIRST_NAME);
    char *lastName = database_get(database, row, COLUMN_LAST_NAME);
    char *city = database_get(database, row, COLUMN_CITY);
    char *graduation = database_get(database, row, COLUMN_GRADUATION);
    char *gradYear = database_get(database, row, COLUMN_GRAD_YEAR);
    char *skills = database_get(database, row, COLUMN_SKILLS);

    int r = snprintf(buffer, bufferSize,
                     "------------------------\n"
                     "Email: %s\n"
                     "First Name: %s\n"
                     "Last Name: %s\n"
                     "City: %s\n"
                     "Graduation Field: %s\n"
                     "Graduation Year: %s\n"
                     "Skills: %s\n",
                     email, firstName, lastName, city, graduation, gradYear,
                     skills);

    free(email);
    free(firstName);
    free(lastName);
    free(city);
    free(graduation);
    free(gradYear);
    free(skills);

    return r;
}

/// Print only name and email
static int printNameAndEmail(char *buffer, int bufferSize, Database *database,
                             int row) {
    char *email = database_get(database, row, COLUMN_EMAIL);
    char *firstName = database_get(database, row, COLUMN_FIRST_NAME);
    char *lastName = database_get(database, row, COLUMN_LAST_NAME);

    int r = snprintf(buffer, bufferSize,
                     "------------------------\n"
                     "Email: %s\n"
                     "Name: %s %s\n",
                     email, firstName, lastName);

    free(email);
    free(firstName);
    free(lastName);

    return r;
}

/// Print graduation field, name and email
static int printCourseNameAndEmail(char *buffer, int bufferSize,
                                    Database *database, int row) {
    char *email = database_get(database, row, COLUMN_EMAIL);
    char *firstName = database_get(database, row, COLUMN_FIRST_NAME);
    char *lastName = database_get(database, row, COLUMN_LAST_NAME);
    char *graduation = database_get(database, row, COLUMN_GRADUATION);

    int r = snprintf(buffer, bufferSize,
                     "------------------------\n"
                     "Email: %s\n"
                     "Name: %s %s\n"
                     "Graduation Field: %s\n",
                     email, firstName, lastName, graduation);

    free(email);
    free(firstName);
    free(lastName);
    free(graduation);

    return r;
}

/// Print graduation field, name and email
static int printEndOfList(char *buffer, int bufferSize) {
    return snprintf(buffer, bufferSize, "------ END OF LIST -----\n");
}

static void insertProfile(Server *server, Database *database,
                          const Request *request) {
    const char *iterator = request->data;
    char *email = nextLine(&iterator);
    char *firstName = nextLine(&iterator);
    char *lastName = nextLine(&iterator);
    char *city = nextLine(&iterator);
    char *graduation = nextLine(&iterator);
    char *gradYear = nextLine(&iterator);
    char *skills = nextLine(&iterator);

    if (email && firstName && lastName && city && graduation && gradYear &&
        skills) {
        switch (database_addRow(database, email, firstName, lastName, city,
                                graduation, gradYear, skills)) {
        case DB_OK:
            database_save(database, DATABASE_FILE);
            server_sendResponse_str(server, request, "Success\n");
            break;
        case DB_FULL:
            server_sendResponse_str(server, request, "Database is full\n");
            break;
        case DB_ALREADY_EXISTS:
            server_sendResponse_str(server, request,
                                    "E-mail is already registered\n");
            break;
        default:
            server_sendResponse_str(server, request, "Failed\n");
            break;
        }
    }

    free(email);
    free(firstName);
    free(lastName);
    free(city);
    free(graduation);
    free(gradYear);
    free(skills);
}

static void listByCourse(Server *server, Database *database,
                         const Request *request) {
    char buf[BUFFER_SIZE];

    char *next = buf;
    int remaining = BUFFER_SIZE;

    int rows = database_countRows(database);
    for (int i = 0; i < rows; i++) {
        char *graduation = database_get(database, i, COLUMN_GRADUATION);
        if (strcmp(graduation, request->data) == 0) {
            int r = printNameAndEmail(next, remaining, database, i);
            next += r;
            remaining -= r;
        }
        free(graduation);
    }
    printEndOfList(next, remaining);

    server_sendResponse_str(server, request, buf);
}

static void runServer(Server *server, Database *database) {
    for (;;) {
        Request *request = server_recvRequest(server);
        if (!request) {
            switch (errno) {
            case EAGAIN:
            case ETIMEDOUT:
                continue;
            default:
                printf("%s\n", strerror(errno));
                return;
            }
        }

        const char *cmd = request->cmd;

        if (strcmp(cmd, "insert") == 0) {
            insertProfile(server, database, request);
        } else if (strcmp(cmd, "listByCourse") == 0) {
            listByCourse(server, database, request);
        } else if (strcmp(cmd, "listBySkill") == 0) {
        } else if (strcmp(cmd, "listByYear") == 0) {
        } else if (strcmp(cmd, "listAll") == 0) {
        } else if (strcmp(cmd, "listByEmail") == 0) {
        } else if (strcmp(cmd, "removeByEmail") == 0) {
        } else {
            printf("Received unknown message\n");
        }

        request_free(request);
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

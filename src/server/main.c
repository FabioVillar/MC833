#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "database.h"
#include "server.h"

#define DEFAULT_PORT 8082

#define BUFFER_SIZE 1024

static const char endOfList[] = "------ END OF LIST ------\n";

/// Iterator is formatted as "<string1>\0<string2>..."
/// - If <string1> exists, return it.
/// - Advance the iterator.
static const char *advanceString(const void **iterator, int *iteratorSize) {
    const void *string = *iterator;
    if (!string) {
        return NULL;
    }

    const void *nextNul = memchr(string, '\0', *iteratorSize);
    if (nextNul) {
        *iterator = nextNul + 1;
        *iteratorSize -= nextNul + 1 - string;
        return (const char *)string;
    } else {
        *iteratorSize = 0;
        *iterator = NULL;
        return NULL;
    }
}

/// Print a row as a profile
static void printProfile(Server *server, Database *database,
                         struct sockaddr_in *clientaddr, int row) {
    char *email = database_get(database, row, COLUMN_EMAIL);
    char *firstName = database_get(database, row, COLUMN_FIRST_NAME);
    char *lastName = database_get(database, row, COLUMN_LAST_NAME);
    char *city = database_get(database, row, COLUMN_CITY);
    char *graduation = database_get(database, row, COLUMN_GRADUATION);
    char *gradYear = database_get(database, row, COLUMN_GRAD_YEAR);
    char *skills = database_get(database, row, COLUMN_SKILLS);

    char buf[BUFFER_SIZE];
    int r = snprintf(buf, BUFFER_SIZE,
                     "-------------------------\n"
                     "Email: %s\n"
                     "First Name: %s\n"
                     "Last Name: %s\n"
                     "City: %s\n"
                     "Graduation Field: %s\n"
                     "Graduation Year: %s\n"
                     "Skills: %s\n",
                     email, firstName, lastName, city, graduation, gradYear,
                     skills);
    Message *message = message_new(clientaddr, "print", buf, r + 1);
    server_sendMessage(server, message);
    message_free(message);

    free(email);
    free(firstName);
    free(lastName);
    free(city);
    free(graduation);
    free(gradYear);
    free(skills);
}

/// Print only name and email
static void printNameAndEmail(Server *server, Database *database,
                              const struct sockaddr_in *clientaddr, int row) {
    char *email = database_get(database, row, COLUMN_EMAIL);
    char *firstName = database_get(database, row, COLUMN_FIRST_NAME);
    char *lastName = database_get(database, row, COLUMN_LAST_NAME);

    char buf[BUFFER_SIZE];
    int r = snprintf(buf, BUFFER_SIZE,
                     "-------------------------\n"
                     "Email: %s\n"
                     "Name: %s %s\n",
                     email, firstName, lastName);
    Message *message = message_new(clientaddr, "print", buf, r + 1);
    server_sendMessage(server, message);
    message_free(message);

    free(email);
    free(firstName);
    free(lastName);
}

/// Print graduation field, name and email
static void printCourseNameAndEmail(Server *server, Database *database,
                                    const struct sockaddr_in *clientaddr,
                                    int row) {
    char *email = database_get(database, row, COLUMN_EMAIL);
    char *firstName = database_get(database, row, COLUMN_FIRST_NAME);
    char *lastName = database_get(database, row, COLUMN_LAST_NAME);
    char *graduation = database_get(database, row, COLUMN_GRADUATION);

    char buf[BUFFER_SIZE];
    int r = snprintf(buf, BUFFER_SIZE,
                     "-------------------------\n"
                     "Email: %s\n"
                     "Name: %s %s\n"
                     "Graduation Field: %s\n",
                     email, firstName, lastName, graduation);
    Message *message = message_new(clientaddr, "print", buf, r + 1);
    server_sendMessage(server, message);
    message_free(message);

    free(email);
    free(firstName);
    free(lastName);
    free(graduation);
}

static void insertProfile(Database *database, const Message *message) {
    const void *param = message->param;
    int paramSize = message->paramSize;

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

static void listByCourse(Server *server, Database *database,
                         const Message *message) {
    const void *param = message->param;
    int paramSize = message->paramSize;

    const char *course = advanceString(&param, &paramSize);

    if (!course) return;

    int rows = database_countRows(database);
    for (int i = 0; i < rows; i++) {
        char *graduation = database_get(database, i, COLUMN_GRADUATION);
        if (strcmp(graduation, course) == 0) {
            printNameAndEmail(server, database, &message->address, i);
        }
        free(graduation);
    }

    Message *response =
        message_new(&message->address, "print", endOfList, sizeof(endOfList));
    server_sendMessage(server, response);
    message_free(response);
}

static void runServer(Server *server, Database *database) {
    for (;;) {
        Message *message = server_recvMessage(server);
        if (!message) {
            switch (errno) {
            case EAGAIN:
            case ETIMEDOUT:
                continue;
            default:
                printf("%s\n", strerror(errno));
                return;
            }
        }

        const char *cmd = message->cmd;

        if (strcmp(cmd, "insert") == 0) {
            insertProfile(database, message);
        } else if (strcmp(cmd, "listByCourse") == 0) {
            listByCourse(server, database, message);
        } else if (strcmp(cmd, "listBySkill") == 0) {
        } else if (strcmp(cmd, "listByYear") == 0) {
        } else if (strcmp(cmd, "listAll") == 0) {
        } else if (strcmp(cmd, "listByEmail") == 0) {
        } else if (strcmp(cmd, "removeByEmail") == 0) {
        } else {
            printf("Received unknown message\n");
        }

        message_free(message);
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

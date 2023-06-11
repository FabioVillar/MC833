#include "server.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "database.h"

#define BUFFER_SIZE 32000

struct Server {
    ServerSocket *socket;
    Database *database;
    char buffer[BUFFER_SIZE];
    int filledBuffer;
};

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

Server *server_new(int port, const char *directory) {
    Server *server = calloc(sizeof(Server), 1);
    if (!server) exit(-1);
    server->socket = serversocket_new(port);
    server->database = database_new(directory);
    return server;
}

void server_free(Server *server) {
    serversocket_free(server->socket);
    database_free(server->database);
    free(server);
}

void server_run(Server *server) {
    database_load(server->database);
    for (;;) {
        Request *request = serversocket_recvRequest(server->socket);
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

        const char *cmd = request_getCmd(request);

        if (strcmp(cmd, "insert") == 0) {
            server_insertProfile(server, request);
        } else if (strcmp(cmd, "listByCourse") == 0) {
            server_listByCourse(server, request);
        } else if (strcmp(cmd, "listBySkill") == 0) {
            server_listBySkill(server, request);
        } else if (strcmp(cmd, "listByYear") == 0) {
            server_listByYear(server, request);
        } else if (strcmp(cmd, "listAll") == 0) {
            server_listAll(server, request);
        } else if (strcmp(cmd, "listByEmail") == 0) {
            server_listByEmail(server, request);
        } else if (strcmp(cmd, "removeByEmail") == 0) {
            server_removeByEmail(server, request);
        } else {
            printf("Received unknown message\n");
        }

        request_free(request);
    }
}

void server_insertProfile(Server *server, const Request *request) {
    const char *iterator = request_getString(request);
    char *email = nextLine(&iterator);
    char *firstName = nextLine(&iterator);
    char *lastName = nextLine(&iterator);
    char *city = nextLine(&iterator);
    char *graduation = nextLine(&iterator);
    char *gradYear = nextLine(&iterator);
    char *skills = nextLine(&iterator);

    if (email && firstName && lastName && city && graduation && gradYear &&
        skills) {
        switch (database_addRow(server->database, email, firstName, lastName,
                                city, graduation, gradYear, skills)) {
        case DB_OK:
            database_save(server->database);
            serversocket_sendResponse_str(server->socket, request, "Success\n");
            break;
        case DB_FULL:
            serversocket_sendResponse_str(server->socket, request,
                                          "Database is full\n");
            break;
        case DB_ALREADY_EXISTS:
            serversocket_sendResponse_str(server->socket, request,
                                          "E-mail is already registered\n");
            break;
        default:
            serversocket_sendResponse_str(server->socket, request, "Failed\n");
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

void server_listByCourse(Server *server, const Request *request) {
    const char *data = request_getString(request);
    if (!data) return;

    server_clearBuffer(server);
    int rows = database_countRows(server->database);
    for (int i = 0; i < rows; i++) {
        char *skills = database_get(server->database, i, COLUMN_SKILLS);
        if (strstr(skills, data)) {
            server_addNameAndEmailToBuffer(server, i);
        }
        free(skills);
    }
    server_addEndOfListToBuffer(server);

    serversocket_sendResponse_str(server->socket, request, server->buffer);
}

void server_listBySkill(Server *server, const Request *request) {
    const char *data = request_getString(request);
    if (!data) return;

    server_clearBuffer(server);
    int rows = database_countRows(server->database);
    for (int i = 0; i < rows; i++) {
        char *skills = database_get(server->database, i, COLUMN_SKILLS);
        if (strstr(skills, data)) {
            server_addNameAndEmailToBuffer(server, i);
        }
        free(skills);
    }
    server_addEndOfListToBuffer(server);

    serversocket_sendResponse_str(server->socket, request, server->buffer);
}

void server_listByYear(Server *server, const Request *request) {
    const char *data = request_getString(request);
    if (!data) return;

    server_clearBuffer(server);
    int rows = database_countRows(server->database);
    for (int i = 0; i < rows; i++) {
        char *gradYear = database_get(server->database, i, COLUMN_GRAD_YEAR);
        if (strcmp(gradYear, data) == 0) {
            server_addNameAndEmailToBuffer(server, i);
        }
        free(gradYear);
    }
    server_addEndOfListToBuffer(server);

    serversocket_sendResponse_str(server->socket, request, server->buffer);
}

void server_listAll(Server *server, const Request *request) {
    server_clearBuffer(server);
    int rows = database_countRows(server->database);
    for (int i = 0; i < rows; i++) {
        server_addNameAndEmailToBuffer(server, i);
    }
    server_addEndOfListToBuffer(server);

    serversocket_sendResponse_str(server->socket, request, server->buffer);
}

void server_listByEmail(Server *server, const Request *request) {
    const char *data = request_getString(request);
    if (!data) return;

    server_clearBuffer(server);
    int rows = database_countRows(server->database);
    for (int i = 0; i < rows; i++) {
        char *email = database_get(server->database, i, COLUMN_EMAIL);
        if (strcmp(email, data) == 0) {
            server_addNameAndEmailToBuffer(server, i);
        }
        free(email);
    }
    server_addEndOfListToBuffer(server);

    serversocket_sendResponse_str(server->socket, request, server->buffer);
}

void server_removeByEmail(Server *server, const Request *request) {
    const char *data = request_getString(request);
    if (!data) return;

    switch (database_deleteRow(server->database, data)) {
    case DB_OK:
        serversocket_sendResponse_str(server->socket, request, "Success\n");
        break;
    case DB_EMAIL_DOES_NOT_EXIST:
        serversocket_sendResponse_str(server->socket, request,
                                      "E-mail is not registered\n");
        break;
    default:
        serversocket_sendResponse_str(server->socket, request, "Failed\n");
        break;
    }

    database_save(server->database);
}

void server_uploadImage(Server *server, const Request *request) {
    const void *data;
    int dataSize;

    request_getData(request, &data, &dataSize);

    const void *nulTerminator = memchr(data, '\0', dataSize);
    if (!nulTerminator) return;

    const char *email = (const char *)data;
    const void *image = nulTerminator + 1;
    int imageSize = dataSize - (image - data);

    switch (database_setImage(server->database, email, image, imageSize)) {
    case DB_OK:
        serversocket_sendResponse_str(server->socket, request, "Success\n");
        break;
    case DB_EMAIL_DOES_NOT_EXIST:
        serversocket_sendResponse_str(server->socket, request,
                                      "E-mail is not registered\n");
        break;
    default:
        serversocket_sendResponse_str(server->socket, request, "Failed\n");
        break;
    }
}

void server_downloadImage(Server *server, const Request *request) {
    static const char success[] = "Success\n";

    const char *data = request_getString(request);
    if (!data) return;

    strcpy(server->buffer, success);
    server->filledBuffer = sizeof(success);

    int imageSize = BUFFER_SIZE - server->filledBuffer;

    switch (database_getImage(server->database, data,
                              &server->buffer[server->filledBuffer],
                              &imageSize)) {
    case DB_OK:
        serversocket_sendResponse(server->socket, request, server->buffer,
                                  server->filledBuffer + imageSize);
        break;
    case DB_IMAGE_DOES_NOT_EXIST:
        serversocket_sendResponse_str(server->socket, request,
                                      "Image is not registered\n");
        break;
    case DB_EMAIL_DOES_NOT_EXIST:
        serversocket_sendResponse_str(server->socket, request,
                                      "E-mail is not registered\n");
        break;
    default:
        serversocket_sendResponse_str(server->socket, request, "Failed\n");
        break;
    }
}

void server_clearBuffer(Server *server) { server->filledBuffer = 0; }

void server_addProfileToBuffer(Server *server, int row) {
    Database *database = server->database;
    char *email = database_get(database, row, COLUMN_EMAIL);
    char *firstName = database_get(database, row, COLUMN_FIRST_NAME);
    char *lastName = database_get(database, row, COLUMN_LAST_NAME);
    char *city = database_get(database, row, COLUMN_CITY);
    char *graduation = database_get(database, row, COLUMN_GRADUATION);
    char *gradYear = database_get(database, row, COLUMN_GRAD_YEAR);
    char *skills = database_get(database, row, COLUMN_SKILLS);

    server->filledBuffer += snprintf(&server->buffer[server->filledBuffer],
                                     BUFFER_SIZE - server->filledBuffer,
                                     "------------------------\n"
                                     "Email: %s\n"
                                     "First Name: %s\n"
                                     "Last Name: %s\n"
                                     "City: %s\n"
                                     "Graduation Field: %s\n"
                                     "Graduation Year: %s\n"
                                     "Skills: %s\n",
                                     email, firstName, lastName, city,
                                     graduation, gradYear, skills);

    free(email);
    free(firstName);
    free(lastName);
    free(city);
    free(graduation);
    free(gradYear);
    free(skills);
}

void server_addNameAndEmailToBuffer(Server *server, int row) {
    Database *database = server->database;
    char *email = database_get(database, row, COLUMN_EMAIL);
    char *firstName = database_get(database, row, COLUMN_FIRST_NAME);
    char *lastName = database_get(database, row, COLUMN_LAST_NAME);

    server->filledBuffer += snprintf(&server->buffer[server->filledBuffer],
                                     BUFFER_SIZE - server->filledBuffer,
                                     "------------------------\n"
                                     "Email: %s\n"
                                     "Name: %s %s\n",
                                     email, firstName, lastName);

    free(email);
    free(firstName);
    free(lastName);
}

void server_addCourseNameAndEmailToBuffer(Server *server, int row) {
    Database *database = server->database;
    char *email = database_get(database, row, COLUMN_EMAIL);
    char *firstName = database_get(database, row, COLUMN_FIRST_NAME);
    char *lastName = database_get(database, row, COLUMN_LAST_NAME);
    char *graduation = database_get(database, row, COLUMN_GRADUATION);

    server->filledBuffer += snprintf(&server->buffer[server->filledBuffer],
                                     BUFFER_SIZE - server->filledBuffer,
                                     "------------------------\n"
                                     "Email: %s\n"
                                     "Name: %s %s\n"
                                     "Graduation Field: %s\n",
                                     email, firstName, lastName, graduation);

    free(email);
    free(firstName);
    free(lastName);
    free(graduation);
}

void server_addEndOfListToBuffer(Server *server) {
    server->filledBuffer += snprintf(&server->buffer[server->filledBuffer],
                                     BUFFER_SIZE - server->filledBuffer,
                                     "------ END OF LIST -----\n");
}

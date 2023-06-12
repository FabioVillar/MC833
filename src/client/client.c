#include "client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

#include "client_socket.h"

#define BUFFER_SIZE 64000
#define MAX_IMAGE_SIZE 32000

struct Client {
    ClientSocket *socket;
    char buffer[BUFFER_SIZE];
};

/// Ask user to press enter to continue.
static void waitForEnter() {
    for (;;) {
        char c = getchar();

        if (c == '\n' || c == EOF) {
            return;
        }
    }
}

/// Read user input until newline.
static int stdinLine(char *buf, int size) {
    int i = 0;

    printf("> ");
    for (;;) {
        char c = getchar();

        if (c == '\n' || c == EOF) {
            buf[i] = '\0';
            return i;
        }

        if (i < size - 1) {
            buf[i] = c;
            i++;
        }
    }
}

Client *client_new(const char *ip, int port) {
    Client *client = calloc(sizeof(Client), 1);
    if (!client) exit(-1);
    client->socket = clientsocket_new(ip, port);
    return client;
}

void client_free(Client *client) {
    clientsocket_free(client->socket);
    free(client);
}

void client_run(Client *client) {
    char buf[8];

    for (;;) {
        // Show the menu
        printf(
            "\nMENU\n"
            "Write a number accordingly to what you want:\n"
            "1 - Insert a new profile in the system\n"
            "2 - List all people graduated in a specific course\n"
            "3 - List all people with a given skill\n"
            "4 - List all people graduated in a specific year\n"
            "5 - List all informations of all profiles\n"
            "6 - Given an email, list all information of it\n"
            "7 - Given an email, remove a profile\n"
            "8 - Upload the image for a profile\n"
            "9 - Download the image for a profile\n"
            "0 - Exit\n");

        int size = stdinLine(buf, sizeof(buf));
        int error = 0;
        if (size == 0) {
            continue;
        } else if (size == 1) {
            switch (buf[0]) {
            case '1':
                client_insertProfile(client);
                break;
            case '2':
                client_listByCourse(client);
                break;
            case '3':
                client_listBySkill(client);
                break;
            case '4':
                client_listByYear(client);
                break;
            case '5':
                client_listAll(client);
                break;
            case '6':
                client_listByEmail(client);
                break;
            case '7':
                client_removeByEmail(client);
                break;
            case '8':
                client_uploadImage(client);
                break;
            case '9':
                client_downloadImage(client);
                break;
            case '0':
                return;
            default:
                error = 1;
                break;
            }
        } else {
            error = 1;
        }

        if (error) {
            printf("Invalid input. Press Enter to continue.\n");
            waitForEnter();
        }
    }
}

void client_insertProfile(Client *client) {
    char email[128];
    printf("Insert email\n");
    stdinLine(email, sizeof(email));

    char firstName[64];
    printf("Insert first name\n");
    stdinLine(firstName, sizeof(firstName));

    char lastName[64];
    printf("Insert last name\n");
    stdinLine(lastName, sizeof(lastName));

    char city[64];
    printf("Insert city\n");
    stdinLine(city, sizeof(city));

    char graduation[64];
    printf("Insert graduation course\n");
    stdinLine(graduation, sizeof(graduation));

    char gradYear[64];
    printf("Insert graduation year\n");
    stdinLine(gradYear, sizeof(gradYear));

    char skills[128];
    printf("Insert skills (Ex.: Skill1,Skill2,Skill3,etc)\n");
    stdinLine(skills, sizeof(skills));

    snprintf(client->buffer, BUFFER_SIZE, "%s\n%s\n%s\n%s\n%s\n%s\n%s", email,
             firstName, lastName, city, graduation, gradYear, skills);

    Response *response =
        clientsocket_sendRequest_str(client->socket, "insert", client->buffer);
    if (response) {
        printf("%s", response_getString(response));
        response_free(response);
    } else {
        printf("Response timed out\n");
    }

    printf("Press enter to continue.");
    waitForEnter();
}

void client_listByCourse(Client *client) {
    printf("Insert graduation course\n");
    stdinLine(client->buffer, BUFFER_SIZE);

    Response *response = clientsocket_sendRequest_str(
        client->socket, "listByCourse", client->buffer);
    if (response) {
        printf("%s", response_getString(response));
        response_free(response);
    } else {
        printf("Response timed out\n");
    }

    printf("Press enter to continue.");
    waitForEnter();
}

void client_listBySkill(Client *client) {
    printf("Insert skill\n");
    stdinLine(client->buffer, BUFFER_SIZE);

    Response *response = clientsocket_sendRequest_str(
        client->socket, "listBySkill", client->buffer);
    if (response) {
        printf("%s", response_getString(response));
        response_free(response);
    } else {
        printf("Response timed out\n");
    }

    printf("Press enter to continue.");
    waitForEnter();
}

void client_listByYear(Client *client) {
    printf("Insert graduation year\n");
    stdinLine(client->buffer, BUFFER_SIZE);

    Response *response = clientsocket_sendRequest_str(
        client->socket, "listByYear", client->buffer);
    if (response) {
        printf("%s", response_getString(response));
        response_free(response);
    } else {
        printf("Response timed out\n");
    }

    printf("Press enter to continue.");
    waitForEnter();
}

void client_listAll(Client *client) {
    Response *response =
        clientsocket_sendRequest_str(client->socket, "listAll", NULL);
    if (response) {
        printf("%s", response_getString(response));
        response_free(response);
    } else {
        printf("Response timed out\n");
    }

    printf("Press enter to continue.");
    waitForEnter();
}

void client_listByEmail(Client *client) {
    printf("Insert email\n");
    stdinLine(client->buffer, BUFFER_SIZE);

    Response *response = clientsocket_sendRequest_str(
        client->socket, "listByEmail", client->buffer);
    if (response) {
        printf("%s", response_getString(response));
        response_free(response);
    } else {
        printf("Response timed out\n");
    }

    printf("Press enter to continue.");
    waitForEnter();
}

void client_removeByEmail(Client *client) {
    printf("Insert email\n");
    stdinLine(client->buffer, BUFFER_SIZE);

    Response *response = clientsocket_sendRequest_str(
        client->socket, "removeByEmail", client->buffer);
    if (response) {
        printf("%s", response_getString(response));
        response_free(response);
    } else {
        printf("Response timed out\n");
    }

    printf("Press enter to continue.");
    waitForEnter();
}

void client_uploadImage(Client *client) {
    char email[128];
    printf("Insert email\n");
    stdinLine(email, sizeof(email));

    char path[1024];
    printf("Insert the path for the image on your computer\n");
    stdinLine(path, sizeof(path));

    FILE *f = fopen(path, "rb");
    if (!f) {
        printf("%s\n", strerror(errno));
        return;
    }

    fseek(f, 0L, SEEK_END);
    int fileSize = ftell(f);
    fseek(f, 0L, SEEK_SET);

    if (fileSize > MAX_IMAGE_SIZE) {
        printf("Maximum image size is %d B\n", MAX_IMAGE_SIZE);
    } else {
        int emailSize = strlen(email);
        strcpy(client->buffer, email);
        fread(&client->buffer[emailSize + 1], fileSize, 1, f);

        Response *response =
            clientsocket_sendRequest(client->socket, "uploadImage",
                                     client->buffer, emailSize + 1 + fileSize);
        if (response) {
            printf("%s", response_getString(response));
            response_free(response);
        } else {
            printf("Response timed out\n");
        }
    }

    fclose(f);

    printf("Press enter to continue.");
    waitForEnter();
}

void client_downloadImage(Client *client) {
    printf("Insert email\n");
    stdinLine(client->buffer, BUFFER_SIZE);

    char path[1024];
    printf("Insert the path for the image on your computer\n");
    stdinLine(path, sizeof(path));

    Response *response = clientsocket_sendRequest_str(
        client->socket, "downloadImage", client->buffer);
    if (response) {
        const void *data;
        int size;

        response_getData(response, &data, &size);

        // Check if is a string
        if (memchr(data, '\0', size)) {
            const char *string = (const char *)data;
            int stringSize = strlen(string);

            const void *image = data + (stringSize + 1);
            int imageSize = size - (stringSize + 1);

            printf("%s", string);
            if (imageSize != 0) {
                FILE *f = fopen(path, "wb");
                if (f) {
                    fwrite(image, imageSize, 1, f);
                    fclose(f);
                }
            }
        }

        response_free(response);
    } else {
        printf("Response timed out\n");
    }

    printf("Press enter to continue.");
    waitForEnter();
}

#include "client.h"

#include <stdio.h>
#include <stdlib.h>

#include "client_socket.h"

struct Client {
    ClientSocket *socket;
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
            "8 - Exit\n");

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

    char params[1024];
    snprintf(params, sizeof(params), "%s\n%s\n%s\n%s\n%s\n%s\n%s", email,
             firstName, lastName, city, graduation, gradYear, skills);

    Response *response =
        clientsocket_sendRequest(client->socket, "insert", params);
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
    char graduation[64];

    printf("Insert graduation course\n");
    stdinLine(graduation, sizeof(graduation));

    Response *response =
        clientsocket_sendRequest(client->socket, "listByCourse", graduation);
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
    char skill[128];

    printf("Insert skill\n");
    stdinLine(skill, sizeof(skill));

    Response *response =
        clientsocket_sendRequest(client->socket, "listBySkill", skill);
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
    char gradYear[64];

    printf("Insert graduation year\n");
    stdinLine(gradYear, sizeof(gradYear));

    Response *response =
        clientsocket_sendRequest(client->socket, "listByYear", gradYear);
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
        clientsocket_sendRequest(client->socket, "listAll", NULL);
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
    char email[128];

    printf("Insert email\n");
    stdinLine(email, sizeof(email));

    Response *response =
        clientsocket_sendRequest(client->socket, "listByEmail", email);
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
    char email[128];

    printf("Insert email\n");
    stdinLine(email, sizeof(email));

    Response *response =
        clientsocket_sendRequest(client->socket, "removeByEmail", email);
    if (response) {
        printf("%s", response_getString(response));
        response_free(response);
    } else {
        printf("Response timed out\n");
    }

    printf("Press enter to continue.");
    waitForEnter();
}

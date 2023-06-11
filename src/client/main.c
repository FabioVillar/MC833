#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client.h"

#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT 8082

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

static void insertProfile(Client *client) {
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

    Response *response = client_sendRequest(client, "insert", params);
    if (response) {
        printf("%s", response_getString(response));
        response_free(response);
    } else {
        printf("Response timed out\n");
    }

    printf("Press enter to continue.");
    waitForEnter();
}

static void listByCourse(Client *client) {
    char graduation[64];

    printf("Insert graduation course\n");
    stdinLine(graduation, sizeof(graduation));

    Response *response = client_sendRequest(client, "listByCourse", graduation);
    if (response) {
        printf("%s", response_getString(response));
        response_free(response);
    } else {
        printf("Response timed out\n");
    }

    printf("Press enter to continue.");
    waitForEnter();
}

static void listBySkill(Client *client) {
    char skill[128];

    printf("Insert skill\n");
    stdinLine(skill, sizeof(skill));

    Response *response = client_sendRequest(client, "listBySkill", skill);
    if (response) {
        printf("%s", response_getString(response));
        response_free(response);
    } else {
        printf("Response timed out\n");
    }

    printf("Press enter to continue.");
    waitForEnter();
}

static void listByYear(Client *client) {
    char gradYear[64];

    printf("Insert graduation year\n");
    stdinLine(gradYear, sizeof(gradYear));

    Response *response = client_sendRequest(client, "listByYear", gradYear);
    if (response) {
        printf("%s", response_getString(response));
        response_free(response);
    } else {
        printf("Response timed out\n");
    }

    printf("Press enter to continue.");
    waitForEnter();
}

static void listAll(Client *client) {
    Response *response = client_sendRequest(client, "listAll", NULL);
    if (response) {
        printf("%s", response_getString(response));
        response_free(response);
    } else {
        printf("Response timed out\n");
    }

    printf("Press enter to continue.");
    waitForEnter();
}

static void listByEmail(Client *client) {
    char email[128];

    printf("Insert email\n");
    stdinLine(email, sizeof(email));

    Response *response = client_sendRequest(client, "listByEmail", email);
    if (response) {
        printf("%s", response_getString(response));
        response_free(response);
    } else {
        printf("Response timed out\n");
    }

    printf("Press enter to continue.");
    waitForEnter();
}

static void removeByEmail(Client *client) {
    char email[128];

    printf("Insert email\n");
    stdinLine(email, sizeof(email));

    Response *response = client_sendRequest(client, "removeByEmail", email);
    if (response) {
        printf("%s", response_getString(response));
        response_free(response);
    } else {
        printf("Response timed out\n");
    }

    printf("Press enter to continue.");
    waitForEnter();
}

/// Executes the client
static void runClient(Client *client) {
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
                insertProfile(client);
                break;
            case '2':
                listByCourse(client);
                break;
            case '3':
                listBySkill(client);
                break;
            case '4':
                listByYear(client);
                break;
            case '5':
                listAll(client);
                break;
            case '6':
                listByEmail(client);
                break;
            case '7':
                removeByEmail(client);
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

int main(int argc, char **argv) {
    const char *ip;
    int port;
    switch (argc) {
    case 1:
        ip = DEFAULT_IP;
        port = DEFAULT_PORT;
        break;
    case 3:
        ip = argv[1];
        port = atoi(argv[2]);
        break;
    default:
        printf("Usage: %s [ip] [port]\n", argv[0]);
        return 1;
    }

    Client *client = client_new(ip, port);
    runClient(client);
    client_free(client);

    return 0;
}

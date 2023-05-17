#define _GNU_SOURCE  // enables strdup

#include "chat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024

#define CMD_PRINT "print"
#define CMD_INPUT "input"

static const char MENU[] =
    "\nMENU\n"
    "Write a number accordingly to what you want:\n"
    "1 - Insert a new profile in the system\n"
    "2 - List all people graduated in a specific course\n"
    "3 - List all people with a given skill\n"
    "4 - List all people graduated in a specific year\n"
    "5 - List all informations of all profiles\n"
    "6 - Given an email, list all information of it\n"
    "7 - Given an email, remove a profile\n";

static int sendCmd(Chat *chat, const char *cmd, const void *value,
                   int sizeValue) {
    char buf[BUFFER_SIZE];

    int sizeCmd = strlen(cmd) + 1;
    int size = sizeValue + sizeCmd;
    if (size > BUFFER_SIZE) return -1;

    strcpy(buf, cmd);
    memcpy(&buf[sizeCmd], value, sizeValue);

    printf("[%s] Sending %s ...\n", chat->addressString, cmd);

    return sendto(chat->fd, buf, size, 0, (struct sockaddr *)&chat->address,
                  sizeof(struct sockaddr_in));
}

static int handleMenuOption(Chat *chat, const char *message) {
    /* if (strcmp(message, "1") == 0) {
        return insertProfile(fd, database);
    } else if (strcmp(message, "2") == 0) {
        return listByCourse(fd, database);
    } else if (strcmp(message, "3") == 0) {
        return listBySkill(fd, database);
    }
    if (strcmp(message, "4") == 0) {
        return listByYear(fd, database);
    } else if (strcmp(message, "5") == 0) {
        return listEverything(fd, database);
    } else if (strcmp(message, "6") == 0) {
        return listByEmail(fd, database);
    } else if (strcmp(message, "7") == 0) {
        return removeByEmail(fd, database);
    } else {
        return sendCmd(fd, CMD_PRINT, "Unknown message\n");
    } */

    return -1;
}

Chat *chat_new(int fd, struct sockaddr_in *address, const char *addressString,
               Database *database) {
    Chat *chat = calloc(1, sizeof(Chat));
    if (!chat) return NULL;

    chat->state = UNINIT;
    memcpy(&chat->address, address, sizeof(struct sockaddr_in));
    chat->addressString = strdup(addressString);
    chat->fd = fd;
    chat->database = database;

    sendCmd(chat, CMD_PRINT, MENU, strlen(MENU) + 1);
    chat->state = WAITING_MENU_ACK;

    return chat;
}

void chat_free(Chat *chat) {
    free(chat->addressString);
    free(chat);
}

int chat_handleData(Chat *chat, void *data, int size) {
    switch (chat->state) {
    case WAITING_MENU:
        // data must contain a nul terminator
        if (!memchr(data, '\0', size)) {
            return -1;
        }
        return handleMenuOption(chat, (const char *)data);

    default:
        return -1;
    }
}

int chat_handleAck(Chat *chat) {
    switch (chat->state) {
    case WAITING_MENU_ACK:
        sendCmd(chat, CMD_INPUT, "menu", 5);
        chat->state = WAITING_MENU;
        return 1;

    default:
        return -1;
    }
}

/*
/// Print a row as a profile
static void printProfile(int fd, Database *database, int row) {
    char *email = database_get(database, row, COLUMN_EMAIL);
    char *firstName = database_get(database, row, COLUMN_FIRST_NAME);
    char *lastName = database_get(database, row, COLUMN_LAST_NAME);
    char *city = database_get(database, row, COLUMN_CITY);
    char *graduation = database_get(database, row, COLUMN_GRADUATION);
    char *gradYear = database_get(database, row, COLUMN_GRAD_YEAR);
    char *skills = database_get(database, row, COLUMN_SKILLS);

    char buf[BUFFER_SIZE];
    snprintf(buf, BUFFER_SIZE,
             "-------------------------\n"
             "Email: %s\n"
             "First Name: %s\n"
             "Last Name: %s\n"
             "City: %s\n"
             "Graduation Field: %s\n"
             "Graduation Year: %s\n"
             "Skills: %s\n",
             email, firstName, lastName, city, graduation, gradYear, skills);
    sendCmd(fd, CMD_PRINT, buf);

    free(email);
    free(firstName);
    free(lastName);
    free(city);
    free(graduation);
    free(gradYear);
    free(skills);
}

/// Print only name and email
static void printNameAndEmail(int fd, Database *database, int row) {
    char *email = database_get(database, row, COLUMN_EMAIL);
    char *firstName = database_get(database, row, COLUMN_FIRST_NAME);
    char *lastName = database_get(database, row, COLUMN_LAST_NAME);

    char buf[BUFFER_SIZE];
    snprintf(buf, BUFFER_SIZE,
             "-------------------------\n"
             "Email: %s\n"
             "Name: %s %s\n",
             email, firstName, lastName);
    sendCmd(fd, CMD_PRINT, buf);

    free(email);
    free(firstName);
    free(lastName);
}

/// Print graduation field, name and email
static void printCourseNameAndEmail(int fd, Database *database, int row) {
    char *email = database_get(database, row, COLUMN_EMAIL);
    char *firstName = database_get(database, row, COLUMN_FIRST_NAME);
    char *lastName = database_get(database, row, COLUMN_LAST_NAME);
    char *graduation = database_get(database, row, COLUMN_GRADUATION);

    char buf[BUFFER_SIZE];
    snprintf(buf, BUFFER_SIZE,
             "-------------------------\n"
             "Email: %s\n"
             "Name: %s %s\n"
             "Graduation Field: %s\n",
             email, firstName, lastName, graduation);
    sendCmd(fd, CMD_PRINT, buf);

    free(email);
    free(firstName);
    free(lastName);
    free(graduation);
}

static int listByCourse(int fd, Database *database) {
    int r;
    sendCmd(fd, CMD_PRINT, "Insert the course to list by:\n");
    char course[50];
    r = askInput(fd, course, sizeof(course));
    if (r <= 0) return r;

    int rows = database_countRows(database);
    for (int i = 0; i < rows; i++) {
        char *graduation = database_get(database, i, COLUMN_GRADUATION);
        if (strcmp(graduation, course) == 0) {
            printNameAndEmail(fd, database, i);
        }
        free(graduation);
    }
    sendCmd(fd, CMD_PRINT, "------ END OF LIST ------\n");

    return 1;
}

static int listBySkill(int fd, Database *database) {
    int r;
    sendCmd(fd, CMD_PRINT, "Insert the skill to list by:\n");
    char skill[50];
    r = askInput(fd, skill, sizeof(skill));
    if (r <= 0) return r;

    int rows = database_countRows(database);
    for (int i = 0; i < rows; i++) {
        char *skills = database_get(database, i, COLUMN_SKILLS);
        if (strstr(skills, skill)) {
            printNameAndEmail(fd, database, i);
        }
        free(skills);
    }
    sendCmd(fd, CMD_PRINT, "------ END OF LIST ------\n");

    return 1;
}

static int listByYear(int fd, Database *database) {
    int r;
    sendCmd(fd, CMD_PRINT, "Insert the year to list by:\n");
    char year[50];
    r = askInput(fd, year, sizeof(year));
    if (r <= 0) return r;

    int rows = database_countRows(database);
    for (int i = 0; i < rows; i++) {
        char *gradYear = database_get(database, i, COLUMN_GRAD_YEAR);
        if (strcmp(gradYear, year) == 0) {
            printCourseNameAndEmail(fd, database, i);
        }
        free(gradYear);
    }
    sendCmd(fd, CMD_PRINT, "------ END OF LIST ------\n");

    return 1;
}

static int listEverything(int fd, Database *database) {
    int rows = database_countRows(database);
    for (int i = 0; i < rows; i++) {
        printProfile(fd, database, i);
    }
    sendCmd(fd, CMD_PRINT, "------ END OF LIST ------\n");

    return 1;
}

static int listByEmail(int fd, Database *database) {
    int r;
    sendCmd(fd, CMD_PRINT, "Insert the email to list by:\n");
    char email[50];
    r = askInput(fd, email, sizeof(email));
    if (r <= 0) return r;

    int rows = database_countRows(database);
    for (int i = 0; i < rows; i++) {
        char *email2 = database_get(database, i, COLUMN_EMAIL);
        if (strcmp(email2, email) == 0) {
            printProfile(fd, database, i);
        }
        free(email2);
    }
    sendCmd(fd, CMD_PRINT, "------ END OF LIST ------\n");

    return 1;
}

static int removeByEmail(int fd, Database *database) {
    int r;
    sendCmd(fd, CMD_PRINT, "Insert the email to remove by:\n");
    char email[50];
    r = askInput(fd, email, sizeof(email));
    if (r <= 0) return r;

    int i = 0;
    while (i < database_countRows(database)) {
        const char *email2 = database_get(database, i, COLUMN_EMAIL);
        if (strcmp(email2, email) == 0) {
            database_deleteRow(database, i);
        } else {
            i++;
        }
    }

    database_save(database, DATABASE_FILE);

    return 1;
}

static int insertProfile(int fd, Database *database) {
    int r;

    sendCmd(fd, CMD_PRINT, "Insert email\n");
    char email[50];
    r = askInput(fd, email, sizeof(email));
    if (r <= 0) return r;

    sendCmd(fd, CMD_PRINT, "Insert name\n");
    char name[50];
    r = askInput(fd, name, sizeof(name));
    if (r <= 0) return r;

    sendCmd(fd, CMD_PRINT, "Insert last name\n");
    char lastName[50];
    r = askInput(fd, lastName, sizeof(lastName));
    if (r <= 0) return r;

    sendCmd(fd, CMD_PRINT, "Insert your city\n");
    char city[50];
    r = askInput(fd, city, sizeof(city));
    if (r <= 0) return r;

    sendCmd(fd, CMD_PRINT, "Insert your graduation field\n");
    char graduationField[50];
    r = askInput(fd, graduationField, sizeof(graduationField));
    if (r <= 0) return r;

    sendCmd(fd, CMD_PRINT, "Insert graduation year\n");
    char year[5];
    r = askInput(fd, year, sizeof(year));
    if (r <= 0) return r;

    sendCmd(fd, CMD_PRINT,
            "Insert your skills (Ex.: Skill1/Skill2/Skill3/etc)\n");
    char skills[100];
    r = askInput(fd, skills, sizeof(skills));
    if (r <= 0) return r;

    database_addRow(database, email, name, lastName, city, graduationField,
                    year, skills);
    database_save(database, DATABASE_FILE);

    return 1;
}



/// Lógica da thread
static void runChat(Params *params) {
    char buf[BUFFER_SIZE];
    int r;

    int fd = params->fd;

    for (;;) {
        if ((r = sendCmd(fd, CMD_PRINT, MENU)) <= 0) break;
        if ((r = askInput(fd, buf, BUFFER_SIZE)) <= 0) break;
        if ((r = handleMenuOption(fd, params->database, buf)) <= 0) break;
    }

    // close the connection
    close(fd);

    printf("[%d] connection closed\n", fd);
}
*/

#include "chat_thread.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

typedef struct {
    Database *database;
    int fd;
} Params;

/// Sends a command to the client.
///
/// Returns less than zero in case of error.
static int sendCmd(int fd, const char *cmd, const char *params) {
    char buf[BUFFER_SIZE];
    if (params) {
        snprintf(buf, BUFFER_SIZE, "%s %s", cmd, params);
        printf("[%d] sending: %s ...\n", fd, cmd);
    } else {
        strncpy(buf, cmd, BUFFER_SIZE);
        printf("[%d] sending: %s\n", fd, cmd);
    }

    // send strlen(buf) plus the null terminator
    int size = strlen(buf) + 1;
    int sent = 0;
    while (sent < size) {
        int r = write(fd, &buf[sent], size - sent);
        if (r <= 0) return r;
        // we sent "r" bytes
        sent += r;
    }

    return size;
}

/// Asks the client for input.
///
/// Returns less than zero in case of error.
static int askInput(int fd, char *buf, int bufSize) {
    int r;
    if ((r = sendCmd(fd, CMD_INPUT, NULL)) < 0) {
        return r;
    }

    int received = 0;
    for (;;) {
        if (received >= bufSize - 1) {
            // string too large
            return -1;
        }

        r = read(fd, &buf[received], bufSize - received);
        if (r <= 0) return r;
        // we read "r" bytes
        received += r;

        // check if we received a null terminator
        if (buf[received - 1] == 0) {
            break;
        }
    }

    // check if contains a tab
    if (strchr(buf, '\t')) {
        sendCmd(fd, CMD_PRINT, "Nao deve conter tab");
        return -1;
    }

    printf("[%d] received: %s\n", fd, buf);

    return received;
}

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
            printProfile(fd, database, i);
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
            printProfile(fd, database, i);
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
            printProfile(fd, database, i);
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

static int handleMenuOption(int fd, Database *database, const char *message) {
    if (strcmp(message, "1") == 0) {
        return insertProfile(fd, database);
    } else if (strcmp(message, "2") == 0) {
        return listByCourse(fd, database);
    } else if (strcmp(message, "3") == 0) {
        return listBySkill(fd, database);
    } if (strcmp(message, "4") == 0) {
        return listByYear(fd, database);
    } else if (strcmp(message, "5") == 0) {
        return listEverything(fd, database);
    } else if (strcmp(message, "6") == 0) {
        return listByEmail(fd, database);
    } else if (strcmp(message, "7") == 0) {
        return removeByEmail(fd, database);
    } else {
        return sendCmd(fd, CMD_PRINT, "Unknown message\n");
    }
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

/// Callback do pthread
static void *chatThread(void *x) {
    runChat((Params *)x);
    free(x);
    return NULL;
}

int startChatThread(pthread_t *thread, Database *database, int fd) {
    int r;
    pthread_attr_t attr;

    Params *params = malloc(sizeof(Params));
    params->fd = fd;
    params->database = database;

    if ((r = pthread_attr_init(&attr)) < 0) return r;

    return pthread_create(thread, &attr, chatThread, params);
}

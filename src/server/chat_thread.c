#include "chat_thread.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 80

typedef struct {
    int fd;
} Params;

/// Sends a string to the client.
///
/// Returns less than zero in case of error.
static int sendString(int fd, const char *buf) {
    printf("[%d] sending: %s\n", fd, buf);

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

/// Receives a string from the client.
///
/// Returns less than zero in case of error.
static int receiveString(int fd, char *buf, int bufSize) {
    int received = 0;
    for (;;) {
        if (received >= bufSize - 1) {
            // string too large
            return -1;
        }

        int r = read(fd, &buf[received], bufSize - received);
        if (r <= 0) return r;
        // we read "r" bytes
        received += r;

        // check if we received a null terminator
        if (buf[received - 1] == 0) {
            printf("[%d] received: %s\n", fd, buf);
            return received;
        }
    }
}

static int listByCourse(int fd) {
    int r;
    sendString(fd, "Insert the course to list by:");
    char course[50];
    r = receiveString(fd, course, sizeof(course));
    if (r <= 0) return r;
    printf("Listing all profiles with %s as graduation course:\n", course);
    FILE *fp;
    fp = fopen("profile.txt", "r");
    char file[1000];
    while (fgets(file, 1000, fp)) {
        char *token;
        char file2[1000];
        strcpy(file2, file);
        token = strtok(file2, " ");
        while (token != NULL) {
            if (strcmp(token, course) == 0) {
                sendString(fd, file);
                break;
            }
            token = strtok(NULL, " ");
        }
    }
    return 1;
}

static int listByYear(int fd) {
    int r;
    sendString(fd, "Insert the year to list by:");
    char year[50];
    r = receiveString(fd, year, sizeof(year));
    if (r <= 0) return r;
    printf("Listing all profiles with %s as graduation year:\n", year);
    FILE *fp;
    fp = fopen("profile.txt", "r");
    char file[1000];
    while (fgets(file, 1000, fp)) {
        char *token;
        char file2[1000];
        strcpy(file2, file);
        token = strtok(file2, " ");
        while (token != NULL) {
            if (strcmp(token, year) == 0) {
                sendString(fd, file);
                break;
            }
            token = strtok(NULL, " ");
        }
    }
    return 1;
}

static int listEverything(int fd) {
    printf("Listing all information stored:");
    FILE *fp;
    fp = fopen("profile.txt", "r");
    char file[1000];
    while (fgets(file, 1000, fp)) {
        sendString(fd, file);
    }
    return 1;
}

static int listByEmail(int fd) {
    int r;
    sendString(fd, "Insert the email to list by:");
    char email[50];
    r = receiveString(fd, email, sizeof(email));
    if (r <= 0) return r;
    printf("Listing all profiles with %s as email:\n", email);
    FILE *fp;
    fp = fopen("profile.txt", "r");
    char file[1000];
    while (fgets(file, 1000, fp)) {
        char *token;
        char file2[1000];
        strcpy(file2, file);
        token = strtok(file2, " ");
        while (token != NULL) {
            if (strcmp(token, email) == 0) {
                sendString(fd, file);
                break;
            }
            token = strtok(NULL, " ");
        }
    }
    return 1;
}

static int deleteEmail(FILE *fp, FILE *temp, char *email) {
    char file[1000];
    while (fgets(file, 1000, fp)) {
        char *token;
        char file2[1000];
        strcpy(file2, file);
        token = strtok(file2, " ");
        while (token != NULL) {
            if (strcmp(token, email) == 0) {
                fputs(file, temp);
                break;
            }
            token = strtok(NULL, " ");
        }
    }
    return 1;
}

static int removeByEmail(int fd) {
    int r;
    sendString(fd, "Insert the email to remove by:");
    char email[50];
    r = receiveString(fd, email, sizeof(email));
    if (r <= 0) return r;
    printf("Removing all profiles with %s as email:\n", email);
    FILE *fp, *temp;
    fp = fopen("profile.txt", "r");
    temp = fopen("delete.tmp", "w");
    if (fp == NULL || temp == NULL) {
        printf("Problem while opening file.\n");
    } else {
        deleteEmail(fp, temp, email);
        fclose(fp);
        fclose(temp);
        remove("profile.txt");
        rename("delete.tmp", "profile.txt");
    }
    return 1;
}

static int insertProfile(int fd) {
    int r;

    sendString(fd, "Insert email");
    char email[50];
    r = receiveString(fd, email, sizeof(email));
    if (r <= 0) return r;

    sendString(fd, "Insert name");
    char name[50];
    r = receiveString(fd, name, sizeof(name));
    if (r <= 0) return r;

    sendString(fd, "Insert last name");
    char lastName[50];
    r = receiveString(fd, lastName, sizeof(lastName));
    if (r <= 0) return r;

    sendString(fd, "Insert your city");
    char city[50];
    r = receiveString(fd, city, sizeof(city));
    if (r <= 0) return r;

    sendString(fd, "Insert your graduation field");
    char graduationField[50];
    r = receiveString(fd, graduationField, sizeof(graduationField));
    if (r <= 0) return r;

    sendString(fd, "Insert graduation year");
    char year[5];
    r = receiveString(fd, year, sizeof(year));
    if (r <= 0) return r;

    sendString(fd, "Insert your skills (Ex.: Skill1/Skill2/Skill3/etc)");
    char skills[100];
    r = receiveString(fd, skills, sizeof(skills));
    if (r <= 0) return r;

    FILE *fp;
    fp = fopen("profile.txt", "a");
    fprintf(fp, "%s %s %s %s %s %s %s %s\n", "Profile:", email, name, lastName,
            city, graduationField, year, skills);
    fclose(fp);

    return 1;
}

static int handleMessage(int fd, const char *message) {
    if ((strcmp(message, "1")) == 0) {
        return insertProfile(fd);
    } else if ((strcmp(message, "2")) == 0) {
        return listByCourse(fd);
    } else if ((strcmp(message, "3")) == 0) {
        return listByYear(fd);
    } else if ((strcmp(message, "4")) == 0) {
        return listEverything(fd);
    } else if ((strcmp(message, "5")) == 0) {
        return listByEmail(fd);
    } else if ((strcmp(message, "6")) == 0) {
        return removeByEmail(fd);
    } else {
        return sendString(fd, "Unknown message");
    }
}

static void runChat(Params *params) {
    char buf[BUFFER_SIZE];
    int r;

    int fd = params->fd;

    for (;;) {
        if ((r = sendString(fd, "menu")) <= 0) break;
        if ((r = receiveString(fd, buf, BUFFER_SIZE)) <= 0) break;
        if ((r = handleMessage(fd, buf)) <= 0) break;
    }

    // close the connection
    close(fd);

    printf("[%d] connection closed\n", fd);
}

static void *chatThread(void *x) {
    runChat((Params *)x);
    free(x);
    return NULL;
}

int startChatThread(pthread_t *thread, int fd) {
    int r;
    pthread_attr_t attr;

    Params *params = malloc(sizeof(Params));
    params->fd = fd;

    if ((r = pthread_attr_init(&attr)) < 0) return r;

    return pthread_create(thread, &attr, chatThread, params);
}

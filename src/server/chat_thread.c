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

static int listByCourse(int fd){
    int r;
    sendString(fd, "Insert the course to list by:");
    char course[50];
    r = receiveString(fd, course, sizeof(course));
    if (r <= 0) return r;
    printf("Listing all profiles with %s as graduation course:\n", course);
    FILE *fp;
    fp = fopen("profile.txt", "r");
    char file[100];
    char profile[6][100];
    int count = 0;
    int condition = -1;
    while(fgets(file, 100, fp)) {
        strcpy(profile[count], file);
        if (count == 5){
            if (strncmp(profile[count], course, strlen(course)) != 0){
                condition = 1;
            }
        }
        else if (count == 7){
            if (condition == -1){
                for(int i = 0; i <= count; i++){
                    sendString(fd, profile[i]);
                }
            }
            condition = -1;
            count = -1;
        }
        count ++;
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

    sendString(fd, "Insert your skills (Skill1/Skill2/Skill3/etc)");
    char skills[100];
    r = receiveString(fd, skills, sizeof(skills));
    if (r <= 0) return r;

    FILE *fp;
    fp = fopen("profile.txt", "a");
    fprintf(fp, "Profile:\n");
    fprintf(fp, "%s\n", email);
    fprintf(fp, "%s\n", name);
    fprintf(fp, "%s\n", lastName);
    fprintf(fp, "%s\n", city);
    fprintf(fp, "%s\n", graduationField);
    fprintf(fp, "%s\n", year);
    fprintf(fp, "%s\n", skills);
    fclose(fp);

    return 1;
}

static int handleMessage(int fd, const char *message) {
    if ((strcmp(message, "1")) == 0) {
        return insertProfile(fd);
    } 
    else if ((strcmp(message, "2")) == 0){
        return listByCourse(fd);
    }   
    else{
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

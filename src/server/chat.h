#ifndef CHAT_H
#define CHAT_H

#include <arpa/inet.h>

#include "database.h"

typedef enum {
    UNINIT,
    WAITING_MENU_ACK,
    WAITING_MENU,

    INSERTPROFILE_WAITING_EMAIL_ACK,
    INSERTPROFILE_WAITING_EMAIL,
    INSERTPROFILE_WAITING_NAME_ACK,
    INSERTPROFILE_WAITING_NAME,
    INSERTPROFILE_WAITING_LASTNAME_ACK,
    INSERTPROFILE_WAITING_LASTNAME,
    INSERTPROFILE_WAITING_CITY_ACK,
    INSERTPROFILE_WAITING_CITY,
    INSERTPROFILE_WAITING_GRADFIELD_ACK,
    INSERTPROFILE_WAITING_GRADFIELD,
    INSERTPROFILE_WAITING_GRADYEAR_ACK,
    INSERTPROFILE_WAITING_GRADYEAR,
    INSERTPROFILE_WAITING_SKILLS,

    LISTBYCOURSE_WAITING_GRADFIELD_ACK,
    LISTBYCOURSE_WAITING_GRADFIELD,

    LISTBYSKILL_WAITING_SKILL_ACK,
    LISTBYSKILL_WAITING_SKILL,

    LISTBYYEAR_WAITING_GRADYEAR_ACK,
    LISTBYYEAR_WAITING_GRADYEAR,

    LISTBYEMAIL_WAITING_EMAIL_ACK,
    LISTBYEMAIL_WAITING_EMAIL,

    REMOVEBYEMAIL_WAITING_EMAIL_ACK,
    REMOVEBYEMAIL_WAITING_EMAIL,
} ChatState;

typedef struct {
    ChatState state;
    struct sockaddr_in address;
    char *addressString;
    int fd;
    Database *database;
} Chat;

Chat *chat_new(int fd, struct sockaddr_in *address, const char *addressString,
               Database *database);

void chat_free(Chat *chat);

int chat_handleData(Chat *chat, void *data, int size);

int chat_handleAck(Chat *chat);

#endif

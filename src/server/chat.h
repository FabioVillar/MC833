#ifndef CHAT_H
#define CHAT_H

#include "database.h"

typedef struct Chat Chat;

Chat *chat_new(Database *database);

void chat_free(Chat *chat);

void chat_handleData(Chat *chat, void *data, int size);

#endif

#ifndef CHATLIST_H
#define CHATLIST_H

#include "chat.h"
#include "database.h"

typedef struct ChatList ChatList;

ChatList *chatlist_new(Database *database);

void chatlist_free(ChatList *chatlist);

void chatlist_createChat(ChatList *chatlist, const char *address);

void chatlist_removeChat(ChatList *chatlist, const char *address);

Chat *chatlist_findChat(ChatList *chatlist, const char *address);

#endif

#ifndef CHATLIST_H
#define CHATLIST_H

#include "chat.h"

typedef struct ChatList ChatList;

ChatList *chatlist_new();

void chatlist_free(ChatList *chatlist);

void chatlist_insertChat(ChatList *chatlist, Chat *chat);

void chatlist_removeChat(ChatList *chatlist, const char *address);

Chat *chatlist_findChat(ChatList *chatlist, const char *address);

#endif

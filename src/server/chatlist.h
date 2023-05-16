#ifndef CHATLIST_H
#define CHATLIST_H

#include <arpa/inet.h>

#include "chat.h"
#include "database.h"

typedef struct ChatList ChatList;

ChatList *chatlist_new(Database *database);

void chatlist_free(ChatList *chatlist);

Chat *chatlist_get(ChatList *chatlist, in_addr_t address, in_port_t port);

#endif

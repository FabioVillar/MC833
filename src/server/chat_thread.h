#ifndef CHAT_THREAD_H
#define CHAT_THREAD_H

#include <pthread.h>

#include "database.h"

// Thread que se comunica com o cliente.
int startChatThread(pthread_t *thread, Database *database, int fd);

#endif

#ifndef CHAT_THREAD_H
#define CHAT_THREAD_H

#include <pthread.h>

// Thread que se comunica com o cliente.
int startChatThread(pthread_t *thread, int fd);

#endif

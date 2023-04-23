#ifndef ACCEPT_THREAD_H
#define ACCEPT_THREAD_H

#include <pthread.h>

typedef struct AcceptThread AcceptThread;

// Thread que aceita conexões dos clientes.
AcceptThread *startAcceptThread(int port);

// Interrompe a thread.
void stopAcceptThread(AcceptThread *thread);

#endif

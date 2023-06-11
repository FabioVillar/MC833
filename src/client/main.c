#include <stdio.h>
#include <stdlib.h>

#include "client.h"

#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT 8082

int main(int argc, char **argv) {
    const char *ip;
    int port;
    switch (argc) {
    case 1:
        ip = DEFAULT_IP;
        port = DEFAULT_PORT;
        break;
    case 3:
        ip = argv[1];
        port = atoi(argv[2]);
        break;
    default:
        printf("Usage: %s [ip] [port]\n", argv[0]);
        return 1;
    }

    Client *client = client_new(ip, port);
    client_run(client);
    client_free(client);

    return 0;
}

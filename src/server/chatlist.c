#include "chatlist.h"

#include <stdlib.h>

#define HASH_TABLE_SIZE 127

typedef struct HashBucket {
    struct HashBucket *next;
    int hash;
    in_addr_t address;
    in_port_t port;
    Chat *chat;
} HashBucket;

static HashBucket *hashbucket_new(int hash, in_addr_t address, in_port_t port) {
    HashBucket *bucket = calloc(1, sizeof(HashBucket));
    bucket->hash = hash;
    bucket->address = address;
    bucket->port = port;
    return bucket;
}

static void hashbucket_free(HashBucket *bucket) {
    if (bucket->next) hashbucket_free(bucket->next);
    chat_free(bucket->chat);
    free(bucket);
}

struct ChatList {
    Database *database;
    HashBucket *hashTable[HASH_TABLE_SIZE];
};

// Hash function
// http://www.cse.yorku.ca/~oz/hash.html
static int djb2(char *data, int size) {
    unsigned long hash = 5381;
    for (int i = 0; i < size; i++) {
        hash = ((hash << 5) + hash) + data[i];
    }
    return hash % HASH_TABLE_SIZE;
}

ChatList *chatlist_new(Database *database) {
    ChatList *chatlist = calloc(1, sizeof(ChatList));
    chatlist->database = database;
    return chatlist;
}

void chatlist_free(ChatList *chatlist) {
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        HashBucket *bucket = chatlist->hashTable[i];
        if (bucket) hashbucket_free(bucket);
    }
    free(chatlist);
}

Chat *chatlist_get(ChatList *chatlist, in_addr_t address, in_port_t port) {
    int hash = djb2((char *)&address, sizeof(in_addr_t));
    hash ^= djb2((char *)&port, sizeof(port));

    HashBucket **bucketLoc = &chatlist->hashTable[hash];
    while (*bucketLoc) {
        HashBucket *bucket = *bucketLoc;

        if (bucket->address == address && bucket->port == port) {
            return bucket->chat;
        }

        *bucketLoc = bucket->next;
    }

    *bucketLoc = hashbucket_new(hash, address, port);

    Chat *chat = chat_new(chatlist->database);
    (*bucketLoc)->chat = chat;

    return chat;
}

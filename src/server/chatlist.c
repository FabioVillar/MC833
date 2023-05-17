#define _GNU_SOURCE  // enables strdup

#include "chatlist.h"

#include <stdlib.h>
#include <string.h>

#define HASH_TABLE_SIZE 127
#define ADDRESS_SIZE 64

typedef struct HashBucket {
    struct HashBucket *next;
    int hash;
    char *address;
    Chat *chat;
} HashBucket;

static HashBucket *hashbucket_new(int hash, const char *address) {
    HashBucket *bucket = calloc(1, sizeof(HashBucket));
    bucket->hash = hash;
    bucket->address = strdup(address);
    return bucket;
}

static void hashbucket_free(HashBucket *bucket) {
    if (bucket->next) hashbucket_free(bucket->next);
    chat_free(bucket->chat);
    free(bucket->address);
    free(bucket);
}

struct ChatList {
    Database *database;
    HashBucket *hashTable[HASH_TABLE_SIZE];
};

// Hash function
// http://www.cse.yorku.ca/~oz/hash.html
static int djb2(const char *data) {
    unsigned long hash = 5381;
    while (*data) {
        hash = ((hash << 5) + hash) + *data;
        data++;
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

void chatlist_createChat(ChatList *chatlist, const char *address) {
    int hash = djb2(address);

    HashBucket **bucketLoc = &chatlist->hashTable[hash];
    while (*bucketLoc) {
        HashBucket *bucket = *bucketLoc;

        if (strcmp(address, bucket->address) == 0) {
            hashbucket_free(bucket);
            *bucketLoc = NULL;
            break;
        }

        *bucketLoc = bucket->next;
    }

    *bucketLoc = hashbucket_new(hash, address);
}

void chatlist_removeChat(ChatList *chatlist, const char *address) {
    int hash = djb2(address);

    HashBucket **bucketLoc = &chatlist->hashTable[hash];
    while (*bucketLoc) {
        HashBucket *bucket = *bucketLoc;

        if (strcmp(address, bucket->address) == 0) {
            hashbucket_free(bucket);
            *bucketLoc = NULL;
            return;
        }

        *bucketLoc = bucket->next;
    }
}

Chat *chatlist_findChat(ChatList *chatlist, const char *address) {
    int hash = djb2(address);

    HashBucket **bucketLoc = &chatlist->hashTable[hash];
    while (*bucketLoc) {
        HashBucket *bucket = *bucketLoc;

        if (strcmp(address, bucket->address) == 0) {
            return bucket->chat;
        }

        *bucketLoc = bucket->next;
    }

    return NULL;
}

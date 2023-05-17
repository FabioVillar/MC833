#include "chatlist.h"

#include <stdlib.h>
#include <string.h>

#define HASH_TABLE_SIZE 127
#define ADDRESS_SIZE 64

typedef struct HashBucket {
    struct HashBucket *next;
    int hash;
    Chat *chat;
} HashBucket;

static HashBucket *hashbucket_new(int hash, Chat *chat) {
    HashBucket *bucket = calloc(1, sizeof(HashBucket));
    bucket->hash = hash;
    bucket->chat = chat;
    return bucket;
}

static void hashbucket_free(HashBucket *bucket) {
    if (bucket->next) hashbucket_free(bucket->next);
    chat_free(bucket->chat);
    free(bucket);
}

struct ChatList {
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

ChatList *chatlist_new() {
    return calloc(1, sizeof(ChatList));
}

void chatlist_free(ChatList *chatlist) {
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        HashBucket *bucket = chatlist->hashTable[i];
        if (bucket) hashbucket_free(bucket);
    }
    free(chatlist);
}

void chatlist_insertChat(ChatList *chatlist, Chat *chat) {
    int hash = djb2(chat->addressString);

    HashBucket **bucketLoc = &chatlist->hashTable[hash];
    while (*bucketLoc) {
        HashBucket *bucket = *bucketLoc;

        if (strcmp(chat->addressString, bucket->chat->addressString) == 0) {
            chat_free(bucket->chat);
            bucket->chat = chat;
            return;
        }

        *bucketLoc = bucket->next;
    }

    *bucketLoc = hashbucket_new(hash, chat);
}

void chatlist_removeChat(ChatList *chatlist, const char *address) {
    int hash = djb2(address);

    HashBucket **bucketLoc = &chatlist->hashTable[hash];
    while (*bucketLoc) {
        HashBucket *bucket = *bucketLoc;

        if (strcmp(address, bucket->chat->addressString) == 0) {
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

        if (strcmp(address, bucket->chat->addressString) == 0) {
            return bucket->chat;
        }

        *bucketLoc = bucket->next;
    }

    return NULL;
}

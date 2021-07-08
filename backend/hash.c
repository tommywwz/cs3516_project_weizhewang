#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "hash.h"

// initialize hash table
hashtab* ht_create () {
    hashtab* ht = malloc(sizeof(hashtab)); // allocate space for hashtable

    ht->entries = malloc(sizeof(entry_ht) * TABLE_SIZE); // allocate space for each data entry in hashtable

    for(int i = 0; i < TABLE_SIZE; ++i) {
        ht->entries[i] = NULL;
    }

    return ht;
}

// allocate mem for entry slot
entry_ht* ht_inst(const char* user, int sock_id) {
    // allocate memory for entry
    entry_ht* entry = malloc(sizeof*entry);

    entry->user = malloc(strlen(user)+1);
    //entry->sock_id = malloc(sizeof(sock_id));

    entry->next = malloc(sizeof*entry);

    // copy the username and sock_id in place
    strcpy((entry->user), user);
    entry->sock_id = sock_id;
    // initialize next to NULL
    entry->next = NULL;

    return entry;
}

// insert user to hash table, return 1 indicate add successfully, return 2 indicate a duplication in name
int ht_add(hashtab* ht, const char* user, int sock_id) {
    unsigned int slot = hash_user(user);
    // look up the slot in hashmap
    entry_ht* entry = ht->entries[slot];
    //printf("entries pointer before: %p\n", ht->entries[slot]); //DEBUG
    // if no entry in that slot, insert one
    if (entry == NULL) {
        ht->entries[slot] = ht_inst(user, sock_id);
        //printf("entries pointer after: %p\n", ht->entries[slot]); // DEBUG
        return 1;
    }
    // when slot is occupied
    entry_ht* prev;
    while (entry != NULL) {
        if (strcmp(entry->user, user) == 0) {
            return 2; // indicate the same username is exist
        }
        // go the the next
        prev = entry;
        entry = prev->next;
    }
    // while the entry is empty 
    prev->next = ht_inst(user, sock_id);
    printf("[DEBUG] user: %s is appended to %d\n", user, slot);
    return 1;
}

void ht_print(hashtab* ht) {
    unsigned int i;
    for(i = 0; i < TABLE_SIZE; ++i) {
        entry_ht* entry = ht->entries[i];

        if (entry == NULL) {
            continue;
        } else {
            for(;;) {
                printf("%d: %s, %d\n", i, entry->user, entry->sock_id);
                if (entry->next == NULL) {
                    printf("[DEBUG] next entry is empty\n");
                    break;
                }

                entry = entry->next;
                // //debug
                // printf("[DEBUG] %d: %s\n", i, entry->user);
            }
            
        }
    }
}

// find user's socket id, if not found return -1
int ht_find(hashtab* ht, const char* user) {
    // look up the slot of this user
    unsigned int slot = hash_user(user);

    // load this entry
    entry_ht* entry = ht->entries[slot];
    entry_ht* prev;
    int i = 0; 

    if (entry == NULL) {
        printf("[DEBUG] no such user\n");
        return -1; // user not found
    }

    while (entry != NULL) {
        if (strcmp(entry->user, user) == 0) {
            printf("[DEBUG] user found\n");
            return entry->sock_id;
        }

        prev = entry;
        entry = prev->next;
        i++;
    }

}

// remove user from hash table
void ht_rm(hashtab* ht, const char* user) {
    // look up the slot of this user
    unsigned int slot = hash_user(user);
        
    // load this entry
    entry_ht* entry = ht->entries[slot];
    entry_ht* prev;
    int i = 0; 

    if (entry == NULL) {
        printf("[DEBUG] nothing need to remove\n");
        return;
    }

    while (entry != NULL) {
        if (strcmp(entry->user, user) == 0) {
            printf("[DEBUG] find the user to be removed!\n");
            // no next entry and first item in the list
            if (entry->next == NULL && i == 0) {
                ht->entries[slot] = NULL;
            }
            // has next entry but first item in the list
            if (entry->next != NULL && i == 0) {
                ht->entries[slot] = entry->next;
            }
            // last item in the list
            if (entry->next == NULL && i != 0) {
                printf("[DEBUG] condition three\n");
                prev->next = NULL;
            }
            // item is in the middle of list
            if (entry->next != NULL && i != 0) {
                printf("[DEBUG] condition four\n");
                prev->next = entry->next;
            }
            free(entry->user);
            //free(entry->sock_id);
            free(entry); 
            return;
        }
           
        prev = entry;
        entry = prev->next;
        i++;
    }
}

// give username a hash code
unsigned int hash_user (const char* user) {
    unsigned long int hash = 0;
    unsigned int i;
    unsigned int user_len = strlen(user);

    // doing mutiple mutiplications to generate the hashcode
    for (i = 0; i < user_len; ++i) {
     hash = hash * 27 + user[i];
    }

    hash = hash % TABLE_SIZE;
    return hash;
}
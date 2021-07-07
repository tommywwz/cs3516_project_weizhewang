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
entry_ht* ht_inst(const char* user) {
    // allocate memory for entry
    entry_ht* entry = malloc(sizeof*entry);
    //entry_ht* entry = malloc(8);
    entry->user = malloc(strlen(user)+1);
    //entry->next = malloc(8);
    entry->next = malloc(sizeof*entry);

    // copy the username in place
    strcpy((entry->user), user);
    // initialize next to NULL
    entry->next = NULL;

    return entry;
}

// insert user to hash table, return 1 indicate add successfully, return 2 indicate a duplication in name
int ht_add(hashtab* ht, const char* user) {
    unsigned int slot = hash_user(user);
    // look up the slot in hashmap
    entry_ht* entry = ht->entries[slot];
    //printf("entries pointer before: %p\n", ht->entries[slot]); //DEBUG
    // if no entry in that slot, insert one
    if (entry == NULL) {
        ht->entries[slot] = ht_inst(user);
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
    prev->next = ht_inst(user);
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
                printf("%d: %s\n", i, entry->user);
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
    unsigned long int val = 0;
    unsigned int i;
    unsigned int user_len = strlen(user);

    // doing mutiple mutiplications to "randomize" the hashcode
    for (i = 0; i < user_len; ++i) {
        val = val * 27 + user[i];
    }

    val = val % TABLE_SIZE;
    return val;
}
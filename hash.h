

#ifndef HASH_H
#define HASH_H

#define TABLE_SIZE 1 // size of hash table

typedef struct Entry_ht {
    char* user;
    struct Entry_ht* next;
} entry_ht;

typedef struct {
    entry_ht** entries;
} hashtab;


unsigned int hash_user (const char* user);
int ht_add(hashtab* ht, const char* user);
void ht_print(hashtab* ht);
void ht_rm(hashtab* ht, const char* user);
entry_ht* ht_inst(const char* user);
hashtab* ht_create ();

#endif
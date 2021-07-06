#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8888
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



int main() {
    int sockfd, ret;
    struct sockaddr_in serverAddr;

    int newSocket;
    struct sockaddr_in newAddr;
    socklen_t addrsize = sizeof(newAddr);

    char buffer[1024], command[1024];
    pid_t childpid;

    hashtab* ptr_usertable = ht_create();

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("Error in connection!\n");
        exit(1);
    }
    printf("Server Socket is Created!\n");

    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    ret = bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (ret < 0) {
        printf("Error in Binding\n");
    } else {
        printf("Bind to Port %d\n", PORT);
    }

    if(listen(sockfd, 10) == 0) {
        printf("Listening.....\n");
    } else {
        printf("Error in Listening\n");
    }

    while (1) {
        newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addrsize);
        if(newSocket < 0) {
            exit(1);
        }
        printf("Connection accepted from addr:%s  port:%d\nwaiting for username.....\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));


        //open a thread for new client
        if ((childpid = fork()) == 0) {
            close(sockfd); /////////////////////////////////////////////////// double check
            
            //check username entering
            char collision[64];
            char username[1024];

            // bzero(username, sizeof(username));
            // recv(newSocket, username, 1024,0);
            // // check if the username is exist
            // collision[0] = ht_add(ptr_usertable, username);
            // printf("[DEBUG] return of collision: %d\n", collision[0]);
            // // if so inform the client and check new user name
            // while (collision[0]) {
            //     send(newSocket, collision, strlen(collision), 0);
            //     bzero(username, sizeof(username));
            //     recv(newSocket, collision, sizeof(collision), 0);
                
            //     recv(newSocket, username, 1024,0);
            //     collision[0] = ht_add(ptr_usertable, username);
            // }

            while(1) {
                bzero(username, sizeof(username));
                bzero(collision, sizeof(collision));
                recv(newSocket, username, 1024,0); //a
                send(newSocket, username, strlen(username), 0); //a

                // check if the username is exist  
                printf("[DEBUG] new socket = %d\n", newSocket);              
                recv(newSocket, collision, sizeof(collision), 0);
                printf("[DEBUG] recv of collision: %d\n", collision[0]);
                collision[0] = ht_add(ptr_usertable, username);
                printf("[DEBUG] return of collision: %d\n", collision[0]);
                send(newSocket, collision, strlen(collision), 0);
                printf("[DEBUG] send of collision: %d\n", collision[0]);
                
                // if so inform the client and check new user name
                if (collision[0] == 1) {
                    break;
                }
            }

            printf("username of port %d is %s\n", ntohs(newAddr.sin_port), username);
            send(newSocket, username, strlen(username), 0);
            printf("\nCurrent users:\n");
            ht_print(ptr_usertable);
            printf("\n");
            //

            while (1) {

                bzero(buffer, sizeof(buffer));
                recv(newSocket, buffer, 1024,0);
                //printf("newSocket: %d\n", newSocket); //debug
                if (strcmp(buffer, "&exit") == 0) {
                    ht_rm(ptr_usertable, username);
                    printf("%s disconnected from addr:%s  port:%d\n", username, inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
                    break;
                } else {
                    printf("%s: %s\n", username, buffer);
                    send(newSocket, buffer, strlen(buffer), 0);
                    ht_print(ptr_usertable);
                }
            }
        }
        
    }

    close(newSocket);

    return 0;

}



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
    entry->user = malloc(strlen(user)+1);

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
    printf("entries pointer before: %p\n", ht->entries[slot]);
    // if no entry in that slot, insert one
    if (entry == NULL) {
        ht->entries[slot] = ht_inst(user);
        printf("entries pointer after: %p\n", ht->entries[slot]);
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
    entry = ht_inst(user);
    return 3;
}

void ht_print(hashtab* ht) {
    printf("entries pointer: %p\n", ht->entries[0]);
    unsigned int i;
    for(i = 0; i < TABLE_SIZE; ++i) {
        entry_ht* entry = ht->entries[i];
        if (entry != NULL) {
            for(;;) {
                printf("%d: %s\n", i, entry->user);
                if (entry->next == NULL) {
                    break;
                }

                entry = entry->next;
                //debug
                printf("%d: %s\n", i, entry->user);
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

    while (strcmp(entry->user, user) != 0) {
        prev = entry;
        entry = prev->next;
    }
    free(entry->user);
    return;
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


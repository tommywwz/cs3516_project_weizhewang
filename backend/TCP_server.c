#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "hash.h"

#define PORT 8888
#define MAX_INPUT_SIZE 1024
#define MAX_USERNAME_LENGTH 64

typedef struct client_args
{
    hashtab* ptr_ht;
    uint16_t port;
    int sockID;
} client_args;

typedef struct server_args
{
    hashtab* ptr_ht;
    int sockfd;    
} server_args;

// load a user list from hashtable
void ht_load_list (char* userlist, hashtab* ht) {
    strncat(userlist, "\nOnline users:\n", strlen("\nOnline users:\n")+1);
    unsigned int i;
    for(i = 0; i < TABLE_SIZE; ++i) {
        entry_ht* entry = ht->entries[i];
        // look up each entry 
        if (entry == NULL) {
            continue;
        } else {
            // loop through the linkedlist to add all user names
            for(;;) {
                strncat(userlist, entry->user, strlen(entry->user));
                strncat(userlist, "\n", strlen("\n")+1);
                if (entry->next == NULL) {
                    break;
                }         
                entry = entry->next;
            }  
        }
    }
}

// broadcast message to all users
void ht_send_all (hashtab* ht, char* msg) {

    uint32_t i = 0;
    // load this entry
    entry_ht* entry;
    entry_ht* prev;

    while (i < TABLE_SIZE) {
        entry = ht->entries[i];
        //printf("[DEBUG] enter table\n");
        while (entry != NULL) {
            if (entry->user != NULL) {
                //printf("[DEBUG] user found\n");
                send(entry->sock_id, msg, strlen(msg), 0);
            }
            prev = entry;
            entry = prev->next;
        }
        i++;
    }
}

int ht_send_user (hashtab* ht, char* msg, const char* user) {
    // look up the slot of this user
    unsigned int slot = hash_user(user);
    char buffer [MAX_USERNAME_LENGTH + MAX_INPUT_SIZE + 30];
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
            sprintf(buffer, "Private Message from %s", msg);
            send(entry->sock_id, buffer, strlen(buffer), 0);
            return 0; // success
        }

        prev = entry;
        entry = prev->next;
        i++;
    }
}

void* newserver (void *arg) {
    server_args* args = arg;
    hashtab* ptr_usertable = args->ptr_ht;
    int sockfd = args->sockfd;
    char cmd[1024];
    while (1) {
        bzero(cmd, sizeof(cmd));
        if (fgets(cmd, MAX_INPUT_SIZE, stdin)) {
            if ((strlen(cmd) > 0) && (cmd[strlen (cmd) - 1] == '\n')) {
                cmd[strlen (cmd) - 1] = '\0';  // add null terminater at the end of input
            }
            if (strcmp(cmd, "&users") == 0) {
                printf("connected users:\n");
                ht_print(ptr_usertable);
            } else if (strcmp(cmd, "&terminate") == 0) {
                close(sockfd);
            } else if (strcmp(cmd, "&help") == 0) {
                printf("&users: display all users\n");
            } else {
                printf("your input is: %s\n", cmd);
                printf("Invaild command!\nEnter &help for all commands\n");
            }
        }
    }
}

void* newclient (void *arg) {
    client_args* args = arg;
    int newSocket = args->sockID;
    uint16_t client_port = args->port;
    hashtab* ptr_usertable = args->ptr_ht;
    //close(sockfd); /////////////////////////////////////////////////// double check
    char buffer[MAX_INPUT_SIZE];
    char send_buffer[MAX_INPUT_SIZE+MAX_USERNAME_LENGTH+32];
    
    //check username entering
    int collision [1] = {1};
    char username[MAX_USERNAME_LENGTH];

    while(1) {
        bzero(username, sizeof(username));
        bzero(collision, sizeof(collision));
        recv(newSocket, username, MAX_USERNAME_LENGTH,0); //a
        send(newSocket, username, strlen(username), 0); //a

        // check if the username is exist  
        printf("[DEBUG] waiting for recv a username\n");              
        recv(newSocket, collision, sizeof(collision), 0);
        printf("[DEBUG] recv of collision: %d\n", collision[0]);
        collision[0] = ht_add(ptr_usertable, username, newSocket);
        printf("[DEBUG] return of collision: %d\n", collision[0]);
        send(newSocket, collision, sizeof(collision), 0);
        //printf("[DEBUG] send of collision: %d\n", collision[0]);
        
        
        // when username is valid, break the loop
        if (collision[0] == 1) {
            break;
        }
    }


    // check if user is disconnected 
    if (strcmp(username, "&exit") == 0) {
        int* exit;
        printf("\n--- disconnected from port:%d ---\n", client_port);
        return exit;
    }

    printf("\n--- username of port %d is %s ---\n", client_port, username);
    send(newSocket, username, strlen(username), 0); // send back username to client

    // print all connected users on server end
    printf("\nCurrent users:\n");
    ht_print(ptr_usertable);
    printf("\n");
    //

    bzero(buffer, sizeof(buffer)); // reset buffer
    sprintf(buffer, "\n---%s entered chat---\n", username);
    ht_send_all (ptr_usertable, buffer);

    while (1) {
        bzero(buffer, sizeof(buffer));
        recv(newSocket, buffer, 1024,0);
        printf("[DEBUG] recv buffer: %s from newSocket: %d\n", buffer, newSocket); //debug


        if ((strncmp(buffer, "@xxx", 1) == 0)) {
            char command [1024]; // command for calling other user
            char recvr [MAX_USERNAME_LENGTH]; // msg recvier
            char msg [MAX_INPUT_SIZE]; // msg extrated from buffer
            char send_msg [MAX_INPUT_SIZE + MAX_USERNAME_LENGTH + 5]; // msg for sending

            // trim message
            sscanf(buffer, "%s", command);
            strcpy(recvr, command+1); // take out @
            strcpy(msg, buffer+strlen(command)+1); // take out message part
            sprintf(send_msg, "%s: %s", username, msg);
            int ret = ht_send_user (ptr_usertable, send_msg, recvr);
            if (ret == -1) {
                send(newSocket, "no such user!", 16, 0);
            }
            continue;
        } else if (strcmp(buffer, "&exit") == 0) {
            char member_left [MAX_INPUT_SIZE];
            printf("[DEBUG] exit entered\n");
            ht_rm(ptr_usertable, username);
            printf("\n--- %s disconnected from port:%d ---\n", username, client_port);
            printf("\nrest of users:\n");
            sprintf(member_left, "\n---%s left chat room---\n", username);
            ht_send_all (ptr_usertable, member_left);
            ht_print(ptr_usertable);
            break;
        } else if (strcmp(buffer, "&list") == 0) {
            char userlist [1024];
            ht_load_list(userlist, ptr_usertable);
            send(newSocket, userlist, strlen(userlist), 0);
        } else if (strcmp(buffer, "&help") == 0) {
            char help_msg [200];
            sprintf(help_msg, "----------------------\ncommands:\n &exit: quit the server\n &list: request user list \n &help: list all commands\n @username: send direct message to user\n----------------------\n");
            send(newSocket, help_msg, 200, 0);
        } else {
            printf("%s: %s\n", username, buffer);
            sprintf(send_buffer, "%s: %s", username, buffer);
            ht_send_all (ptr_usertable, send_buffer);
        }
    }
}


int main() {
    int sockfd, ret;
    struct sockaddr_in serverAddr;


    struct sockaddr_in newAddr;
    socklen_t addrsize = sizeof(newAddr);

    char command[1024];
    pthread_t pthread_id;

    int newSocket;
    
    hashtab* ptr_usertable = ht_create();
    server_args ser_arg; // pointer for passing to server admin thread


    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("[-] Error in connection!\n");
        exit(1);
    }
    printf("[+] Server Socket is Created!\n");

    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    ret = bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (ret < 0) {
        printf("[-] Error in Binding\n");
    } else {
        printf("[+] Bind to Port %d\n", PORT);
    }

    if(listen(sockfd, 10) == 0) {
        printf("Listening.....\n");
        ser_arg.ptr_ht = ptr_usertable;
        ser_arg.sockfd = sockfd;
        pthread_create(&pthread_id, NULL, newserver, &ser_arg); //create server admin thread
    } else {
        printf("[-] Error in Listening\n");
    }


    while (1) {
        newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addrsize);
        if(newSocket < 0) {
            exit(1);
        }
        printf("\n[+] Connection accepted from addr:%s  port:%d\nwaiting for username.....\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

        client_args client_args;
        client_args.port = ntohs(newAddr.sin_port);
        client_args.ptr_ht = ptr_usertable;
        client_args.sockID = newSocket;

        //open a thread for new client
        if (pthread_create(&pthread_id, NULL, newclient, &client_args) != 0){
		    printf("[-] ERROR: can't create client thread\n");
            exit(1);
        }
        printf("[DEBUG] new client thread opened\n");
	}
  
    printf("server terminated!\n");
    close(newSocket);

    return 0;

}






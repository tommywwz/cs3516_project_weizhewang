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


// broadcast message to all users
void ht_send_all (hashtab* ht, char* msg) {

    uint32_t i = 0;
    // load this entry
    entry_ht* entry;
    entry_ht* prev;

    while (i < TABLE_SIZE) {
        entry = ht->entries[i];
        printf("[DEBUG] enter table\n");
        while (entry != NULL) {
            if (entry->user != NULL) {
                printf("[DEBUG] user found\n");
                send(entry->sock_id, msg, strlen(msg), 0);
            }
            prev = entry;
            entry = prev->next;
        }
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
    char buffer[1024];
    char send_buffer[2048+32];
    
    //check username entering
    char collision[64];
    char username[1024];

    while(1) {
        bzero(username, sizeof(username));
        bzero(collision, sizeof(collision));
        recv(newSocket, username, 1024,0); //a
        send(newSocket, username, strlen(username), 0); //a

        // check if the username is exist  
        //printf("[DEBUG] new socket = %d\n", newSocket);              
        recv(newSocket, collision, sizeof(collision), 0);
        printf("[DEBUG] recv of collision: %d\n", collision[0]);
        collision[0] = ht_add(ptr_usertable, username, newSocket);
        printf("[DEBUG] return of collision: %d\n", collision[0]);
        send(newSocket, collision, strlen(collision), 0);
        //printf("[DEBUG] send of collision: %d\n", collision[0]);
        
        // if so inform the client and check new user name
        if (collision[0] == 1) {
            break;
        }
    }

    printf("\n--- username of port %d is %s ---\n", client_port, username);
    send(newSocket, username, strlen(username), 0);
    printf("\nCurrent users:\n");
    ht_print(ptr_usertable);
    printf("\n");
    //

    while (1) {
        bzero(buffer, sizeof(buffer));
        recv(newSocket, buffer, 1024,0);
        printf("[DEBUG] recv buffer: %s from newSocket: %d\n", buffer, newSocket); //debug

        char command [1024]; // command for calling other user
        char recvr [1024]; // msg recvier
        char msg [1024]; // msg to other user
        sscanf(buffer, "%s %s", command, msg);
        if ((strncmp(command, "@xxx", 1) == 0) && strlen(command) > 2) {
            strcpy(recvr, command+1);
            int ret = ht_find(ptr_usertable ,recvr);
            if (ret == -1) {
                send(newSocket, "no such user!", 16, 0);
            } else {
                send(ret, buffer, strlen(buffer), 0);
            }
            continue;
        } else if (strcmp(buffer, "&exit") == 0) {
            printf("[DEBUG] exit entered\n");
            ht_rm(ptr_usertable, username);
            printf("\n--- %s disconnected from port:%d ---\n", username, client_port);
            printf("rest of users:\n");
            ht_print(ptr_usertable);
            break;
        } else {
            printf("%s: %s\n", username, buffer);
            sprintf(send_buffer, "%s: %s\n", username, buffer);
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
        printf("[+] Connection accepted from addr:%s  port:%d\nwaiting for username.....\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

        client_args client_args;
        client_args.port = ntohs(newAddr.sin_port);
        client_args.ptr_ht = ptr_usertable;
        client_args.sockID = newSocket;

        //open a thread for new client
        pthread_create(&pthread_id, NULL, newclient, &client_args);
  
    }

    close(newSocket);

    return 0;

}






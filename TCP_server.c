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

typedef struct PTH_Args
{
    hashtab* ptr_ht;
    uint16_t port;
    int sockID;
} arguments;

void* newserver (void *arg) {
    hashtab* ptr_usertable = arg;
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
            } else if (strcmp(cmd, "&quit") == 0) {
                ///////////
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
    arguments* args = arg;
    int newSocket = args->sockID;
    uint16_t client_port = args->port;
    hashtab* ptr_usertable = args->ptr_ht;
    //close(sockfd); /////////////////////////////////////////////////// double check
    char buffer[1024];
    
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
        //printf("[DEBUG] recv of collision: %d\n", collision[0]);
        collision[0] = ht_add(ptr_usertable, username);
        //printf("[DEBUG] return of collision: %d\n", collision[0]);
        send(newSocket, collision, strlen(collision), 0);
        //printf("[DEBUG] send of collision: %d\n", collision[0]);
        
        // if so inform the client and check new user name
        if (collision[0] == 1) {
            break;
        }
    }

    printf("username of port %d is %s\n", client_port, username);
    send(newSocket, username, strlen(username), 0);
    printf("\nCurrent users:\n");
    ht_print(ptr_usertable);
    printf("\n");
    //

    while (1) {
        bzero(buffer, sizeof(buffer));
        recv(newSocket, buffer, 1024,0);
        //printf("newSocket: %d\n", newSocket); //debug
        if (strcmp(buffer, "&users") == 0) {
            ht_print(ptr_usertable);
        }
        if (strcmp(buffer, "&exit") == 0) {
            ht_rm(ptr_usertable, username);
            printf("%s disconnected from port:%d\n", username, client_port);
            ht_print(ptr_usertable);
            break;
        } else {
            printf("%s: %s\n", username, buffer);
            send(newSocket, buffer, strlen(buffer), 0);
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

    pthread_create(&pthread_id, NULL, newserver, ptr_usertable);

    while (1) {
        newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addrsize);
        if(newSocket < 0) {
            exit(1);
        }
        printf("Connection accepted from addr:%s  port:%d\nwaiting for username.....\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

        arguments args;
        args.port = ntohs(newAddr.sin_port);
        args.ptr_ht = ptr_usertable;
        args.sockID = newSocket;

        // DEBUG
        // ht_add(ptr_usertable, "Tommy");
        // ht_print(ptr_usertable);
        // ht_add(ptr_usertable, "Tom");
        // ht_print(ptr_usertable);
        // DEBUG


        //open a thread for new client
        pthread_create(&pthread_id, NULL, newclient, &args);
  
    }

    close(newSocket);

    return 0;

}






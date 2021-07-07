#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8888
#define MAX_INPUT_SIZE 1024

// globals
int clientSocket = 0;
char username[MAX_INPUT_SIZE];
int leave = 0;


void* client_recv_handler () {
    
    char msg [MAX_INPUT_SIZE+32];
    while(1) {
        bzero(msg, sizeof(msg));
        if(recv(clientSocket, msg, MAX_INPUT_SIZE, 0) < 0) {
            printf("Error in Connection [Fail to Recv]\n");
        } else {
            printf("%s\n", msg);
            printf("chat here:\n");
        }
    }
}

void* client_send_handler () {

    char msg [MAX_INPUT_SIZE];
    while (1) {

        if (fgets(msg, MAX_INPUT_SIZE, stdin)) {
            if ((strlen(msg) > 0) && (msg[strlen (msg) - 1] == '\n')) {
                msg[strlen (msg) - 1] = '\0';  // add null terminater at the end of input
                send(clientSocket, msg, strlen(msg), 0);
                if (strcmp(msg, "&exit") == 0) {
                    close(clientSocket);
                    printf("Disconnected from server.\n");
                    leave = 1;
                }
            }
        }
    }
}




int main() {
    
    struct sockaddr_in serverAddr;
    char msg[MAX_INPUT_SIZE];

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if(clientSocket < 0) {
        printf("[-] Error in connection.\n");
        exit(1);
    }
    printf("[+] Client Socket is created!\n");

    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int ret = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

    if (ret < 0) {
        printf("[-] Failed to Connect\n");
        exit(1);
    }

    printf("[+] Connected to server\n");

    // allow user to choose nickname at the begining

    char collision[64];
    
    while (1) {
        bzero(username, sizeof(username));
        printf("Your name please: ");
        if (fgets(username, MAX_INPUT_SIZE, stdin)) {
            if ((strlen(username) > 0) && (username[strlen (username) - 1] == '\n')) {
                    username[strlen (username) - 1] = '\0';
            }
            sscanf(username, "%s", username);

            if (strlen(username) < 3) {
                printf("username must be more than two characters and no space allowed!\n");
            } else {
                send(clientSocket, username, strlen(username), 0); 
                recv(clientSocket, username, 1024,0); 

                // check the username collision
                send(clientSocket, collision, strlen(collision), 0);
                printf("[DEBUG] send of collision: %d\n", collision[0]);
                recv(clientSocket, collision, sizeof(collision), 0);
                printf("[DEBUG] collision recv: %d\n", collision[0]);                

                if (collision[0] == 2) {
                    printf("This name is used! please try another name\n");
                    //bzero(collision, sizeof(collision));
                } else {
                    //bzero(collision, sizeof(collision));
                    break;
                }
            }
        }  
    }
    
    if(recv(clientSocket, username, MAX_INPUT_SIZE, 0) < 0) {
        printf("Error in Connection [Fail to Recv]\n");
    } else {
        printf("Server: your name is: %s\n", username);
    }
    printf("\n///////////////\nGreeting %s\n///////////////\n\n", username);
    printf("chat here:\n");


    pthread_t send_thread, recv_thread;
    if (pthread_create(&send_thread, NULL, (void *) client_send_handler, NULL) != 0){
		printf("[-] ERROR: can't create send thread\n");
        exit(1);
	}

    if (pthread_create(&recv_thread, NULL, (void *) client_recv_handler, NULL) != 0){
		printf("[-] ERROR: can't create recv thread\n");
        exit(1);
	}

    while (!leave) {

    }

    printf("bye\n");


    // while (1) {
    //     printf("%s: ", username);

    //     if (fgets(msg, MAX_INPUT_SIZE, stdin)) {
    //         if ((strlen(msg) > 0) && (msg[strlen (msg) - 1] == '\n')) {
    //             msg[strlen (msg) - 1] = '\0';  // add null terminater at the end of input
    //             send(clientSocket, msg, strlen(msg), 0);
    //             if (strcmp(msg, "&exit") == 0) {
    //                 close(clientSocket);
    //                 printf("Disconnected from server.\n");
    //                 exit(1);
    //             }
    //         }
    //         if (strlen(msg) > 0) {
    //             if(recv(clientSocket, msg, MAX_INPUT_SIZE, 0) < 0) {
    //                 printf("Error in Connection [Fail to Recv]\n");
    //             } else {
    //                 printf("Server: %s\n", msg);
    //                 bzero(msg, sizeof(msg));
    //             }
    //         } else {
    //             printf ("You have entered nothing!\n");
    //         }
    //     } 

    // }


    // while (1) {
    //     printf("%s: ", username);
    //     bzero(msg, sizeof(msg));

    //     fflush(stdin);
    //     //fgets(msg, MAX_INPUT_SIZE, stdin);

    //     while (fgets(msg, MAX_INPUT_SIZE, stdin)) {
    //         printf("yea\n");
    //         if ((strlen(msg) > 0) && (msg[strlen (msg) - 1] == '\n')) {
    //              msg[strlen (msg) - 1] = '\0';
    //         }


    //         send(clientSocket, msg, strlen(msg), 0);
    //         if (strcmp(msg, "&exit") == 0) {
    //             close(clientSocket);
    //             printf("Disconnected from server.\n");
    //             exit(1);
    //         }
        
    //         if(recv(clientSocket, msg, MAX_INPUT_SIZE, 0) < 0) {
    //             printf("Error in Connection [Fail to Recv]\n");
    //         } else {
    //             printf("Server: %s\n", msg);
    //             bzero(msg, sizeof(msg));
    //         }
    //     }
        // scanf("%1024[^\n]%*c", msg);
    // }

    return 0;
}































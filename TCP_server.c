#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8888

int main() {
    int sockfd, ret;
    struct sockaddr_in serverAddr;

    int newSocket;
    struct sockaddr_in newAddr;
    socklen_t addrsize = sizeof(newAddr);

    char buffer[1024], command[1024];
    pid_t childpid;

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

        // // check commands for server
        // scanf("%s", command);
        // if (strcmp(command, "&exit") == 0) {
        //     printf("Server terminated!\n");
        // }


        //open a thread for new client
        if ((childpid = fork()) == 0){
            close(sockfd); /////////////////////////////////////////////////// double check
            
            //check username entering
            char username[1024];
            bzero(username, sizeof(username));
            recv(newSocket, username, 1024,0);
            printf("username of port %d is %s\n", ntohs(newAddr.sin_port), username);
            send(newSocket, username, strlen(username), 0);
            //

            while (1) {

                bzero(buffer, sizeof(buffer));
                recv(newSocket, buffer, 1024,0);
                //printf("newSocket: %d\n", newSocket); //debug
                if (strcmp(buffer, "&exit") == 0) {
                    printf("%s disconnected from addr:%s  port:%d\n", username, inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
                    break;
                } else {
                    printf("%s: %s\n", username, buffer);
                    send(newSocket, buffer, strlen(buffer), 0);
                }
            }
        }
        
    }

    close(newSocket);

    return 0;

}


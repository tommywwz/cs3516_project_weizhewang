#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8888
#define MAX_INPUT_SIZE 1024

int main() {

    int clientSocket;
    struct sockaddr_in serverAddr;
    char msg[MAX_INPUT_SIZE];
    char username[MAX_INPUT_SIZE];

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if(clientSocket < 0) {
        printf("Error in connection.\n");
        exit(1);
    }
    printf("Client Socket is created!\n");

    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int ret = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

    if (ret < 0) {
        printf("Failed to Connect\n");
        exit(1);
    }

    printf("Connected to server\n");

    // allow user to choose nickname at the begining
    printf("Your name please: ");
    scanf("%s", username);
    send(clientSocket, username, strlen(username), 0);
    //

    if(recv(clientSocket, username, MAX_INPUT_SIZE, 0) < 0) {
        printf("Error in Connection [Fail to Recv]\n");
    } else {
        printf("Server: your name is: %s\n", username);
    }
    printf("Fuck you %s\n", username);
    printf("%s: \n", username);

    while (1) {
        printf("%s: ", username);
        bzero(msg, sizeof(msg));

        fflush(stdin);
        //fgets(msg, MAX_INPUT_SIZE, stdin);

        while (fgets(msg, MAX_INPUT_SIZE, stdin) == 0) {
            printf("yea\n");
        }
        // scanf("%1024[^\n]%*c", msg);

        if ((strlen(msg) > 0) && (msg[strlen (msg) - 1] == '\n')) {
            msg[strlen (msg) - 1] = '\0';
        }


        send(clientSocket, msg, strlen(msg), 0);
        if (strcmp(msg, "&exit") == 0) {
            close(clientSocket);
            printf("Disconnected from server.\n");
            exit(1);
        }
        
        if(recv(clientSocket, msg, MAX_INPUT_SIZE, 0) < 0) {
            printf("Error in Connection [Fail to Recv]\n");
        } else {
            printf("Server: %s\n", msg);
            bzero(msg, sizeof(msg));
        }
    }

    return 0;
}































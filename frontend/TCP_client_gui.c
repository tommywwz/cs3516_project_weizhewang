#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <gtk/gtk.h>

#define PORT 8888
#define MAX_INPUT_SIZE 1024
#define MAX_USERNAME_SIZE 128

void exiting ();
void  Ctrl_C_Handler(int sig);
void* client_recv_handler ();
void* client_send_handler ();
int client_main();




// globals
int clientSocket = 0;
char username[MAX_USERNAME_SIZE];
int leave = 0;
pthread_t backend;

static void
print_hello (GtkWidget *widget,
             gpointer   data)
{
  g_print ("Hello World\n");
}

void exiting () {
    send(clientSocket, "&exit", strlen("&exit"), 0);
    close(clientSocket);
    printf("\nDisconnected from server.\n");
    leave = 1;
}

void  Ctrl_C_Handler(int sig)
{
    exiting ();
}

void* client_recv_handler () {
    char msg [MAX_INPUT_SIZE+MAX_USERNAME_SIZE+30];
    while(!leave) {
        bzero(msg, sizeof(msg));
        if(recv(clientSocket, msg, sizeof(msg), 0) < 0) {
            printf("Error in Connection [Fail to Recv]\n");
        } else if (strlen(msg) == 0) {
            printf("\nSERVER IS DOWN!\nConnection breaked!\n");
            exiting ();
        } else {
            printf("%s\n", msg);
            printf("chat here:\n");
        }
    }
}

void* client_send_handler () {

    char msg [MAX_INPUT_SIZE];
    while (!leave) {

        if (fgets(msg, MAX_INPUT_SIZE, stdin)) {
            if ((strlen(msg) > 0) && (msg[strlen (msg) - 1] == '\n')) {
                msg[strlen (msg) - 1] = '\0';  // add null terminater at the end of input
                if (strcmp(msg, "&exit") == 0) {
                    exiting ();
                }
                send(clientSocket, msg, strlen(msg), 0);
            }
        }
    }
}





int client_main() {
    
    struct sockaddr_in serverAddr;
    char msg[MAX_INPUT_SIZE];

    signal(SIGINT, Ctrl_C_Handler);

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

    int collision [1] = {1};
    
    while (!leave) {
        bzero(username, sizeof(username));
        printf("Your name please: ");
        if (fgets(username, MAX_USERNAME_SIZE, stdin)) {
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
                send(clientSocket, collision, sizeof(collision), 0);
                printf("[DEBUG] send of collision: %d\n", collision[0]);
                recv(clientSocket, collision, sizeof(collision), 0);
                printf("[DEBUG] collision recv: %d\n", collision[0]);                

                if (collision[0] == 2) {
                    printf("This name is used! please try another name\n");
                    //bzero(collision, sizeof(collision));
                } else {
                    //bzero(collision, sizeof(collision));
                    if(recv(clientSocket, username, MAX_USERNAME_SIZE, 0) < 0) {
                        printf("Error in Connection [Fail to Recv]\n");
                    } else {
                        printf("Server: your name is: %s\n", username);
                    }
                    printf("\n///////////////\nGreeting %s\n///////////////\n\n", username);
                    printf("----------------------\n"
                        "commands:\n &exit: quit the server\n &list: request user list \n &help: list all commands\n @username: send direct message to user\n"
                        "----------------------\n");
                    break;
                }
            }
        }  
    }
    

    if (!leave) {
        pthread_t send_thread, recv_thread;
        if (pthread_create(&send_thread, NULL, (void *) client_send_handler, NULL) != 0){
            printf("[-] ERROR: can't create send thread\n");
            exit(1);
        }

        if (pthread_create(&recv_thread, NULL, (void *) client_recv_handler, NULL) != 0){
            printf("[-] ERROR: can't create recv thread\n");
            exit(1);
        }
    }

    while (!leave) {

    }

    printf("\nBye~\n");

    return 0;
}

void* initialize_client(void* wut){
    if (pthread_create(&backend, NULL, (void *) client_main, NULL) != 0){
        printf("[-] ERROR: can't create backend thread\n");
    }
}

int
main (int   argc,
      char *argv[])
{
    GtkBuilder *builder;
    GObject *window;
    GObject *button;
    GError *error = NULL;

    gtk_init (&argc, &argv);

    /* Construct a GtkBuilder instance and load our UI description */
    builder = gtk_builder_new ();
    if (gtk_builder_add_from_file (builder, "builder.html", &error) == 0)
    {
        g_printerr ("Error loading file: %s\n", error->message);
        g_clear_error (&error);
        return 1;
    }

    /* Connect signal handlers to the constructed widgets. */
    window = gtk_builder_get_object (builder, "window");
    g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

    button = gtk_builder_get_object (builder, "button1");
    g_signal_connect (button, "clicked", G_CALLBACK (initialize_client), NULL);

    button = gtk_builder_get_object (builder, "quit");
    g_signal_connect (button, "clicked", G_CALLBACK (exiting), NULL);

    gtk_main ();
    

    return 0;
}































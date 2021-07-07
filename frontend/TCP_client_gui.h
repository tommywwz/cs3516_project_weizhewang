#ifndef TCP_CLIENT_GUI
#define TCP_CLIENT_GUI

#define PORT 8888
#define MAX_INPUT_SIZE 1024
#define MAX_USERNAME_SIZE 128

void exiting ();
void  Ctrl_C_Handler(int sig);
void* client_recv_handler ();
void* client_send_handler ();
int client_main();

#endif
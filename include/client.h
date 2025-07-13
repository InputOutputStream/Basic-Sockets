
#ifndef CLIENT_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <ncurses.h>
#include <unistd.h>
#include <chat.h>

// client.h
typedef struct CLIENT_APP {
    // Connexion réseau
    int server_fd;
    struct sockaddr_in server_addr;
    
    // Interface TUI
    window_t *ui;
    
    // État local
    char username[64];
    chat_t user_chats[MAX_CHATS];
    int client_number;
    message_t message_cache[MAX_CHAT_MESSAGES_NUMBER];
    int message_count;
    
    // Threads
    pthread_t network_thread;    // Réception messages
    pthread_t ui_thread;         // Interface utilisateur
    pthread_t heartbeat_thread;  // Keep-alive
    
    // Synchronisation
    pthread_mutex_t message_mutex;
    int running;
} client_app_t;

typedef struct ARGS
{
    window_t *window;
    int fd;
}args_t;


int auto_reconnect(const char *ip, int port);
int send_message_to_user(int receiver_id, const char *message, int client_fd);

char* get_message_content(char *buffer);
int get_message_type(char *buffer);
void update_client_list_from_server(window_t *window, const char *client_data);
int connect_to_server(const char *addr, int port);
void client_signal_handler(int sig, int client_fd);
void* receive_messages(void *arg);
int send_formatted_message(int msg_type, const char *message, int client_fd);
char* extract_delimited_content(char *buffer, char start_char, char end_char, int *start_pos);

window_t* init_interface();
void cleanup_interface(window_t *window);
void display_messages(window_t *window, const char *sender, const char *message);
void update_status(window_t *window, const char *status);
void update_client_list(window_t *window);



#endif // !CLIENT_H
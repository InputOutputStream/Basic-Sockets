#ifndef CHAT_H
#define CHAT_H

#include <stdint.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ncurses.h>
#include <pthread.h>


#define MAX_CLIENT 250
#define MAX_CHAT_MESSAGES_NUMBER 1000 
#define MAX_MESSAGE_LENGHT 1024 // increase later or not
#define DEFAULT_PORT 8008
#define NAME_LENGHT 128

/***
 * Chat handling header file
*/

/**
 * From the oldest message to the newest
 */

typedef struct MESSAGES
{
    char *payload[MAX_MESSAGE_LENGHT];
    uint16_t sender_user_id;
    uint16_t receiver_user_id;
    time_t timestamp;
}message_t;


typedef struct CLIENT
{
    struct sockaddr_in client_addr; 
    unsigned int client_addr_len;
    size_t client_id;
    char client_name[NAME_LENGHT];
    time_t connect_time;
    size_t client_number;
    int client_fd;
    pthread_t thread; // Client execution thread
    int active;
}client_t;


typedef struct CHAT
{
    // Properties of the chat
    u_int16_t chat_id;
    message_t messages[MAX_CHAT_MESSAGES_NUMBER];
    size_t messges_count;
    client_t *participants[MAX_CLIENT];
    size_t participants_count;
}chat_t;


typedef struct CHAT_WINDOW_STRUCT
{
    WINDOW *msg_win;
    WINDOW *input_win;
    WINDOW *users_win;
    WINDOW *status_win;
    int rows, cols; // Sera defini en fonction du contexte

}window_t;

// Variables globales avec mutex pour le controle d'acces
extern client_t clients[MAX_CLIENT]; // Liste des clients en cours
extern int client_count; // Nombre de clients connecter
extern pthread_mutex_t clients_mutex; 
extern int server_running; 


#endif // !CHAT_H
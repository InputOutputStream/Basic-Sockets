#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <ncurses.h>
#include <unistd.h>
#include <signal.h>

#include <chat.h>
#include <client.h>

static int client_running = 1;
static int client_fd = -1;
static pthread_t receive_thread;

void client_signal_handler(int sig) {
    
    switch (sig)
    {
        case SIGINT:
        case SIGTERM:
            printf("\nDéconnexion...\n");
            client_running = 0;
            if (client_fd >= 0) {
                close(client_fd);
            }
            break;
        default:
            break;
    }
}

// Thread pour recevoir messages
void *receive_messages(void *arg) {
    int fd = *((int*)arg);
    char buffer[MAX_MESSAGE_LENGHT];
    
    while (client_running) {
        ssize_t bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            if (client_running) {
                printf("Connexion fermée par le serveur\n");
                client_running = 0;
            }
            break;
        }
        buffer[bytes] = '\0';
    }
    return NULL;
}

int connect_to_server(const char *ip, int port) {
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(client_fd);
        return -1;
    }

    if (connect(client_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(client_fd);
        return -1;
    }

    return 0;
}

int send_formatted_message(int msg_type, const char *message) {
    char type_buffer[16];
    snprintf(type_buffer, sizeof(type_buffer), "#%d#", msg_type);
    
    if (send(client_fd, type_buffer, strlen(type_buffer), 0) < 0) {
        perror("send type");
        return -1;
    }
    
    if (send(client_fd, message, strlen(message), 0) < 0) {
        perror("send message");
        return -1;
    }
    
    return 0;
}

int auto_reconnect(const char *ip, int port) {
    int attempts = 0;
    const int max_attempts = 5;
    const int retry_delay = 2;
    
    while (attempts < max_attempts && client_running) {
        printf("Tentative de reconnexion %d/%d...\n", attempts + 1, max_attempts);
        
        if (connect_to_server(ip, port) == 0) {
            printf("Reconnecté !\n");
            return 0;
        }
        
        attempts++;
        sleep(retry_delay * attempts);
    }
    
    return -1;
}


void broadcast_message(void *sender, const char *message)
{

}

int send_message_to_user(int receiver_id, const char *message) {
    char formatted_msg[MAX_MESSAGE_LENGHT];
    snprintf(formatted_msg, sizeof(formatted_msg), "%%%d%%%s", receiver_id, message);
    return send_formatted_message(MSG_SEND, formatted_msg);
}
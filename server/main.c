#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include <chat.h>
#include <server.h>


client_t clients[MAX_CLIENT];
chat_t chats[MAX_CHATS];
int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t chats_mutex = PTHREAD_MUTEX_INITIALIZER;
int server_running = 1;

// Use a global counter that never decreases

int main() {

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    printf("Démarrage de Low Chat Server...\n");
    
    // Initialize globals
    server_running = 1;
    client_count = 0;
    pthread_mutex_init(&clients_mutex, NULL);
    pthread_mutex_init(&chats_mutex, NULL);
    
    if(start_message_server() != 0) {
        fprintf(stderr, "❌ Échec de démarrage du serveur\n");
        exit(EXIT_FAILURE);
    }

    printf("✅ Serveur démarré sur le port %d\n", DEFAULT_PORT);

    // Keep server running until signal
    while(server_running) {
        sleep(1);
    }
    
    printf("✅ Low Chat fermé\n");
    return 0;
}
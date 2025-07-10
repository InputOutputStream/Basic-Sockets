#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <ncurses.h>

#include <chat.h>
#include <client.h>

// Variables globales
pthread_t receive_thread;
int client_fd = -1;
volatile int client_running = 1;


int main() {
    const char *str_addr = "127.0.0.1";
    const int port = 8008;
    
    printf("Démarrage de Low Chat...\n");
    
    // Configuration signaux
    signal(SIGINT, client_signal_handler);
    signal(SIGTERM, client_signal_handler);
    
    // Initialisation interface
    window_t *window = init_interface();
    if (!window) {
        fprintf(stderr, "ERROR❌: Echec du démarrage de l'interface\n");
        exit(EXIT_FAILURE);
    }
    
    update_status(window, "Connexion au serveur...");
    
    // Connexion au serveur
    if (connect_to_server(str_addr, port) < 0) {
        update_status(window, "❌ Impossible de se connecter au serveur");
        cleanup_interface(window);
        return EXIT_FAILURE;
    }
    
    update_status(window, "✅ Connecté au serveur - Tapez vos messages");
    
    // Lancement thread de réception
    pthread_create(&receive_thread, NULL, receive_messages, &client_fd);
    
    // Boucle principale avec interface ncurses
    char input_buffer[MAX_MESSAGE_LENGHT];
    int ch;
    int input_pos = 0;
    
    while (client_running) {
        ch = wgetch(window->input_win);
        
        switch (ch) {
            case '\n':
            case '\r':
            case KEY_ENTER:
                if (input_pos > 0) {
                    input_buffer[input_pos] = '\0';
                    
                    // Commandes
                    if (strcmp(input_buffer, "/quit") == 0) {
                        client_running = 0;
                    } else {
                        // Afficher le message localement
                        display_messages(window, "Vous", input_buffer);
                        
                        // Envoyer au serveur
                        if (send_formatted_message(MSG_BROADCAST_CLIENTS, input_buffer) < 0) {
                            update_status(window, "❌ Erreur envoi message");
                        }
                    }
                    
                    // Nettoyer la zone d'entrée
                    werase(window->input_win);
                    box(window->input_win, 0, 0);
                    mvwprintw(window->input_win, 0, 2, "Input: ");
                    wrefresh(window->input_win);
                    input_pos = 0;
                }
                break;
                
            case KEY_BACKSPACE:
            case 127:
                if (input_pos > 0) {
                    input_pos--;
                    mvwdelch(window->input_win, 1, input_pos + 1);
                    wrefresh(window->input_win);
                }
                break;
                
            default:
                if (ch >= 32 && ch <= 126 && input_pos < MAX_MESSAGE_LENGHT - 1) {
                    input_buffer[input_pos++] = ch;
                    mvwaddch(window->input_win, 1, input_pos, ch);
                    wrefresh(window->input_win);
                }
                break;
        }
        
        // Mise à jour périodique de la liste des clients
        static time_t last_update = 0;
        time_t now = time(NULL);
        if (now - last_update > 1) {
            update_client_list(window);
            last_update = now;
        }
    }
    
    // Nettoyage
    client_running = 0;
    if (client_fd >= 0) {
        close(client_fd);
    }
    pthread_cancel(receive_thread);
    pthread_join(receive_thread, NULL);
    
    cleanup_interface(window);
    printf("✅ Chat fermé proprement\n");
    return 0;
}

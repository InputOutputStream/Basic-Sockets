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
    client_fd = connect_to_server(str_addr, port);
    if ( client_fd < 0) {
        fprintf(stderr, "ERROR❌: Impossible de se connecter au serveur\n");
        update_status(window, "❌ Impossible de se connecter au serveur");
        cleanup_interface(window);
        return EXIT_FAILURE;
    }

    
    update_status(window, "✅ Connecté au serveur - Tapez vos messages");

    args_t args = {window, client_fd};
    
    // Lancement thread de réception
    pthread_create(&receive_thread, NULL, receive_messages, &args);
    
    // Boucle principale avec interface ncurses
    char input_buffer[MAX_MESSAGE_LENGHT];
    int ch;
    int input_pos = 0;


    // Commandes list
    char *commands[] = {"/help", "/quit", "/join <channel_id>", "/chat", 
        "/leave_channel", "/create_channel <name>", ""};
    static int commandes_number = 6;

    while (client_running) {
        ch = wgetch(window->input_win);
        
        switch (ch) {
            case '\n':
            case '\r':
            case KEY_ENTER:
                if (input_pos > 0) {
                    input_buffer[input_pos] = '\0';
                    
                    // Commandes
                    if (strcmp(input_buffer, "/help") == 0) {
                        display_messages(window, "Aide", "Commandes disponibles:");
                        for(int i = 0; i < commandes_number; i++)
                            display_messages(window, "Commandes", commands[i]);
                    }
                    else if (strcmp(input_buffer, "/quit") == 0) {
                        client_running = 0;
                    }
                    else if (strncmp(input_buffer, "/join ", 6) == 0) {
                        char* channel_id = get_command_arg(input_buffer);
                        if (channel_id) {
                            char formatted_msg[MAX_MESSAGE_LENGHT];
                            snprintf(formatted_msg, sizeof(formatted_msg), "[%s]", channel_id);
                            
                            if (send_formatted_message(MSG_JOIN_CHANNEL, formatted_msg, client_fd) < 0) {
                                update_status(window, "❌ Erreur lors de la connexion au canal");
                            } else {
                                display_messages(window, "Système", "Demande de connexion au canal envoyée");
                            }
                        }
                        else{
                            display_messages(window, "Erreur", "Usage: /join <channel_id>");
                        }
                    }else if (strncmp(input_buffer, "/create_channel ", 16) == 0) {
                        char* channel_name = get_command_arg(input_buffer);
                        if (channel_name) {
                            if (send_formatted_message(MSG_CREATE_CHANNEL, channel_name, client_fd) < 0) {
                                update_status(window, "❌ Erreur lors de la création du canal");
                            } else {
                                display_messages(window, "Système", "Demande de création de canal envoyée");
                            }
                        } else {
                            display_messages(window, "Erreur", "Usage: /create_channel <nom_du_canal>");
                        } 
                    }else if (strcmp(input_buffer, "/leave_channel") == 0) {
                        if (send_formatted_message(MSG_LEAVE_CHANNEL, "", client_fd) < 0) {
                        update_status(window, "❌ Erreur lors de la sortie du canal");
                        } else {
                            display_messages(window, "Système", "Demande de sortie du canal envoyée");
                        }
                    }
                    else if(strcmp(input_buffer, "/chat") == 0) {
                        display_messages(window, "Info", "Vous êtes en mode chat général");
                    } 
                    else {
                        display_messages(window, "Vous", input_buffer);
                        
                        if (send_formatted_message(MSG_BROADCAST_CLIENTS, input_buffer, client_fd) < 0) {
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

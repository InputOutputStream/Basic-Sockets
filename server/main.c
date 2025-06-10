#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <chat.h>
#include <server.h>

// Ce menu est sense rester comme ca on va voir comment tout implmenter ailleur
int main() {

    printf("Démarrage de Low Chat...\n");
   
    chat_t *chats = calloc(1, sizeof(struct CHAT));
    window_t *window = init_interface();

    if(!window)
    {
        fprintf(stderr, "ERROR❌: Echec du demarage du serveur");
        exit(EXIT_FAILURE);
    }

    update_status(window, "Démarrage du serveur...");


    // Démarrage du serveur
    if(start_message_server(chats) != 0){ // Start the server to load the messages
        cleanup_interface(window);
        fprintf(stderr, "❌ Échec de démarrage du serveur\n");
        exit(EXIT_FAILURE);
    }
    
    update_status(window, "Serveur démarré sur le port 8008 - Tapez vos messages");


    char message_buffer[MAX_MESSAGE_LENGHT];
    int ch;
    int input_pos = 0;

    while (server_running)
    {
        ch = wgetch(window->input_win);

        switch (ch)
        {
            case '\n':
            case '\r':
            case KEY_ENTER:
                if(input_pos > 0)
                {
                    message_buffer[input_pos] = '\0';

                    //Commandes...
                    if(strcmp(message_buffer, "/quit") == 0)
                        server_running  = 0;
                    else{
                        display_messages(window, "Vous", message_buffer);
                        /**
                         * 
                         * La logique ici est a completer
                         */
                    }

                    // Nettoyer la zone d'entrer

                    werase(window->input_win);
                    box(window->input_win, 0, 0);
                    mvwprintw(window->input_win, 0, 2, "Input: ");
                    wrefresh(window->input_win);
                    input_pos = 0;
                }

                break;
            case KEY_BACKSPACE:
            case 127:
                if(input_pos > 0)
                {
                    input_pos--;
                    mvwdelch(window->input_win, 1, input_pos+1);
                    wrefresh(window->input_win);
                }
                break;

            default:
                if(ch >= 32 && ch <= 126 && input_pos < MAX_MESSAGE_LENGHT -1)
                {
                    message_buffer[input_pos++]=ch;
                    mvwaddch(window->input_win, 1, input_pos, ch);
                    wrefresh(window->input_win);
                }
                break;
        }

        static time_t last_update = 0;
        time_t now = time(NULL);
        if(now - last_update > 1)
        {
            update_client_list(window);
            last_update = now;
        }
    }

    //Clean
    cleanup_interface(window);
    free(chats);
    printf("✅ Low Chat fermé proprement\n");
    return 0;
}
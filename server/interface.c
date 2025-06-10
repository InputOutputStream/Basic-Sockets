#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <ncurses.h>

#include <chat.h>
#include <server.h>

//Display the chat interface in the terminal with the messages, plus the keyboad + mouse support handleling
/***
 * Would stay like this cause a few objects must be return for the inteface to be updated
 * we will se as we go
*/


// Variables globales
client_t clients[MAX_CLIENT];
int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
int server_running = 1;


window_t *init_interface()
{
    initscr();              // Démarre ncurses
    cbreak();               // Pas de buffer ligne
    noecho();               // Ne pas afficher les touches
    keypad(stdscr, TRUE);   // Activer touches spéciales
    curs_set(1);            // Affiche le curseur


    if(has_colors())
    {
        start_color();
        init_pair(1, COLOR_CYAN, COLOR_BLACK); // Messages
        init_pair(2, COLOR_GREEN, COLOR_BLACK); // Status
        init_pair(3, COLOR_YELLOW, COLOR_BLACK); // Input msg text
    }

    window_t *window = calloc(1, sizeof(window_t));
    getmaxyx(stdscr, window->rows, window->cols);


    // Layout: Messages 70% | users 30%
    int msg_width = (window->cols * 7)/10;
    int users_width = window->cols - msg_width;
    
    //Fenetre des messages, en haut a gauche, plus grande que les autres
    window->msg_win = newwin(window->rows - 4, msg_width, 0, 0);

    // Fenetre des utilisateur, en haut a droite
    window->input_win = newwin(window->rows - 4, users_width, 0, msg_width);

    // Fenêtre de saisie, en bas
    window->input_win = newwin(2, window->cols, window->rows - 3, 0);
    
    // Fenêtre de status (tout en bas)
    window->status_win = newwin(1, window->cols, window->rows - 1, 0);

    
    // Configuration des fenêtres
    scrollok(window->msg_win, TRUE);
    box(window->msg_win, 0, 0); 
    box(window->users_win, 0, 0);
    box(window->input_win, 0, 0);
    
    // Titres de chaque fenetres
    mvwprintw(window->msg_win, 0, 2, " Messages ");
    mvwprintw(window->users_win, 0, 2, " Users ");
    mvwprintw(window->input_win, 0, 2, " Input ");
    
    // refreshing all different windows to setup the changes
    wrefresh(window->msg_win);
    wrefresh(window->users_win);
    wrefresh(window->input_win);
    wrefresh(window->status_win);
    
    return window;
}


void display_messages(window_t *window, const char *user_name, const char *messages){
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_str[16];
    strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);

    if(has_colors())
        wattron(window->msg_win, COLOR_PAIR(1));
    wprintw(window->msg_win, "[%s] %s: %s\n", time_str, user_name, messages);
    if(has_colors())
            wattroff(window->msg_win, COLOR_PAIR(1));

    wrefresh(window->msg_win);
}

void update_client_list(window_t *window)
{
    werase(window->users_win);
    box(window->msg_win, 0, 0);
    mvwprintw(window->users_win, 0, 2,  "Users (%d) ", client_count);

    pthread_mutex_lock(&clients_mutex);
        for(int i = 0; i < client_count; i++)
        {
            if(clients[i].active)
            {
                mvwprintw(window->users_win, i+1, 1, "• %s", clients[i].client_name);
            }
        }
    pthread_mutex_unlock(&clients_mutex);

    wrefresh(window->users_win);
}

void update_status(window_t *window, const char *status) {
    werase(window->status_win);
    if (has_colors()) wattron(window->status_win, COLOR_PAIR(2));
    mvwprintw(window->status_win, 0, 0, "%s", status);
    if (has_colors()) wattroff(window->status_win, COLOR_PAIR(2));
    wrefresh(window->status_win);
}

void cleanup_interface(window_t *window) {
    if (window) {
        delwin(window->msg_win);
        delwin(window->users_win);
        delwin(window->input_win);
        delwin(window->status_win);
        free(window);
    }
    endwin();
}



/**
 * 
  *  ┌────|────────────────────────┐
  *  │    |     Messages           │  <- Fenêtre messages (défilement)
  *  │    |                        │
  *  │    |                        │
  *  ├────|────────────────────────┤
  *  │  > Votre message ici...     │  <- Ligne d’entrée utilisateur
*    |_____________________________|
  *  └─────────────────────────────┘
 */
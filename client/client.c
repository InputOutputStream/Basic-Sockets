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


void client_signal_handler(int sig, int client_fd) {
    
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


char* extract_delimited_content(char *buffer, char start_char, char end_char, int *start_pos) {
    char *pos = strchr(buffer + *start_pos, start_char);
    if (!pos) {
        return NULL;
    }
    
    int start_idx = pos - buffer + 1;
    char *end_pos = strchr(pos + 1, end_char);
    if (!end_pos) {
        return NULL;
    }
    
    int len = end_pos - pos - 1;
    if (len <= 0) {
        return NULL;
    }
    
    char *result = malloc(len + 1);
    strncpy(result, buffer + start_idx, len);
    result[len] = '\0';
    
    *start_pos = end_pos - buffer + 1; // Update position for next search
    return result;
}


char* get_command_arg(const char* input) {
    char* space = strchr(input, ' ');
    if (!space) return NULL;
    
    // Skip spaces
    while (*space == ' ') space++;
    
    return (*space != '\0') ? space : NULL;
}

// Thread pour recevoir messages
void *receive_messages(void *arg) {
    args_t args = *((args_t*)arg);

    int fd = args.fd;
    window_t *window = args.window;

    char buffer[MAX_MESSAGE_LENGHT];
    time_t last_beat = time(NULL);

    while (client_running) {
        ssize_t bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            if (client_running) {
                update_status(window, "Connexion fermée \n");
                client_running = 0;
            }
            break;
        }
        buffer[bytes] = '\0';

        client_t c = {0};
        int i=0;

        while( i<client_count)
        {
            if(fd == clients[i].client_fd)
            {    c = clients[i];
                break;
            }
            i++;
        }

        // Parse message type
        int msg_type = get_message_type(buffer);
        char *content = get_message_content(buffer);
        
        if (!content){
            fprintf(stderr,"Message is empty\n");
            continue;
        }

        switch (msg_type) {
            case MSG_AUTH_ACK:
                break;

            case MSG_AUTH_FAIL:
                break;

            case MSG_BROADCAST: 
                // Format: sender_id:message
                char *colon = strchr(content, ':');
                if (colon) {
                    *colon = '\0';
                    char *sender_id = content;
                    char *message = colon + 1;
                    
                    char sender_name[32];
                    snprintf(sender_name, sizeof(sender_name), "User%s", sender_id);
                    display_messages(window, sender_name, message);
                }
                break;
            
            case MSG_USER_JOIN: 
                display_messages(window, "Système", content);
                break;
            
            case MSG_USER_LEAVE: 
                display_messages(window, "Système", content);
                break;

            case MSG_CHANNEL_CREATION: 
                display_messages(window, "Système", content);
                break;
            
            case MSG_MSG: 
                // Private message from another user
                int start_pos = 0;
                char *sender_content = extract_delimited_content(content, '%', '%', &start_pos);
                if (sender_content) {
                    char sender_name[32];
                    snprintf(sender_name, sizeof(sender_name), "User%s", sender_content);
                    display_messages(window, sender_name, content + start_pos);
                    free(sender_content);
                }
                break;
            
            case MSG_USER_LIST: 
                update_client_list_from_server(window, content);
                break;
            
            case MSG_PING: 
                send_formatted_message(MSG_HEARTBEAT, "", fd);
                break;
            
            case MSG_SERVER_ERROR: 
                update_status(window, content);
                break;
            
            case MSG_SERVER_INFO: 
                display_messages(window, "Serveur", content);
                break;
            
            default:
                display_messages(window, "Inconnu", content);
                break;
        }
        
        free(content);

        // Send heartbeat every 5 minutes
        if (time(NULL) - last_beat > 300) {
            send_formatted_message(MSG_HEARTBEAT, "", fd);
            last_beat = time(NULL);
        }
    }
    return NULL;
}

int get_message_type(char *buffer) {
    int start_pos = 0;
    char *type_str = extract_delimited_content(buffer, '#', '#', &start_pos);
    if (!type_str) return -1;
    
    int type = atoi(type_str);
    free(type_str);
    return type;
}

char* get_message_content(char *buffer) {
    char *first_hash = strchr(buffer, '#');
    if (!first_hash) return NULL;
    
    char *second_hash = strchr(first_hash + 1, '#');
    if (!second_hash) return NULL;
    
    return strdup(second_hash + 1);
}

void update_client_list_from_server(window_t *window, const char *client_data) {
    // Parse client list format: "id1,id2,id3"
    werase(window->users_win);
    box(window->users_win, 0, 0);
    mvwprintw(window->users_win, 0, 2, "Clients");
    
    char *data_copy = strdup(client_data);
    char *token = strtok(data_copy, ",");
    int line = 1;
    
    while (token && line < getmaxy(window->users_win) - 1) {
        mvwprintw(window->users_win, line++, 1, "Client_%s", token);
        token = strtok(NULL, ",");
    }
    
    wrefresh(window->users_win);
    free(data_copy);
}

int connect_to_server(const char *ip, int port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        fprintf(stderr, "Failed to connect to server\n");
        return -1;
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(fd);
        return -1;
    }

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(fd);
        return -1;
    }

    return fd;
}

int send_formatted_message(int msg_type, const char *message, int client_fd) {
    char msg[MAX_MESSAGE_LENGHT];
    snprintf(msg, sizeof(msg), "#%d#%s", msg_type, message);
    
    if (send(client_fd, msg, strlen(msg), 0) < 0) {
        perror("send formatted message");
        return -1;
    }
    return 0;
}

int auto_reconnect(const char *ip, int port) {
    int attempts = 0;
    const int max_attempts = 5;
    const int retry_delay = 2;
    int fd = -1;

    while (attempts < max_attempts && client_running) {
        printf("Tentative de reconnexion %d/%d...\n", attempts + 1, max_attempts);
        
        if (connect_to_server(ip, port) == 0) {
            printf("Reconnecté !\n");
            return fd;
        }
        
        attempts++;
        sleep(retry_delay * attempts);
    }
    
    return -1;
}

int send_message_to_user(int receiver_id, const char *message, int client_fd) {
    char formatted_msg[MAX_MESSAGE_LENGHT];
    snprintf(formatted_msg, sizeof(formatted_msg), "%%%d%%%s", receiver_id, message);
    return send_formatted_message(MSG_SEND, formatted_msg, client_fd);
}

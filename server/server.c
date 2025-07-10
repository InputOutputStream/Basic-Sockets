#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>

#include <chat.h>
#include <server.h>


int start_message_server()
{
    //Création du socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socketcreation failed");
        return -1;
    }

    // Réutilisation de l'adresse, de la machine pour l'interface 1 je supose
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Préparation de la structure sockaddr_in pour le serveur
    struct sockaddr_in server_addr = {0};    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DEFAULT_PORT);  // Convertit en big-endian
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_in)) < 0) 
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

   
    if (listen(server_fd, MAX_CLIENT) < 0) {
        perror("listen failed");
        close(server_fd);
        return -1;
    }

    //C'est Ici que tout commence, logique du server de chat
    pthread_t accept_thread;
    int *server_fd_ptr = calloc(1, sizeof(int));
    *server_fd_ptr = server_fd;

    pthread_create(&accept_thread, NULL, accept_connections, server_fd_ptr);
    pthread_detach(accept_thread); // terminate automatically on close

    clean_up_clients(accept_thread);
    return 0;
}

void *accept_connections(void *arg) {
    int server_fd = *((int*)arg);
    free(arg);

    while(server_running)
    {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd < 0) continue; // on n'a pas reussi a le connecter bah on passe

        pthread_mutex_lock(&clients_mutex);
        if(client_count < MAX_CLIENT)
        {
            client_t *client = &clients[client_count];
            client->client_fd = client_fd;
            client->client_addr = client_addr;
            client->client_id = client_count;
            client->client_addr_len = addr_len;
            client->msg_count = 0;
            client->active = 1;
            client->connect_time = time(NULL);
            client->last_activity = client->connect_time;
            snprintf(client->client_name, sizeof(client->client_name), "User%d", client_count);
            
            pthread_create(&client->thread, NULL, handle_client, client);
            client_count++;
        }
        else {
            close(client_fd); // Reject connection
        }

        pthread_mutex_unlock(&clients_mutex);
    }

    close(server_fd);
    return NULL;
}
 

message_t init_message(client_t client, char *buffer)
{
    message_t new = {0};
    strcpy(new.payload, buffer);
    new.length = strlen(buffer);
    new.sender_user_id = client.client_id;
    new.receiver_user_id = -1;
    new.timestamp = time(NULL);
    
    return new;
}

int get_chat_id(char *buffer)
{
    int i = 1, st = 0, stp = 0; 
    if(buffer[0] != '[')
    {
        fprintf(stderr, "Missing chat id from message\n");
        return -1;
    }

    st = i;
    while(buffer[i] != ']')
    {
        i++;
    }
    stp = i;

    char *sid = (buffer + st + 1);
    char *dest = malloc(stp-st+1);
    strncpy(dest, sid, stp-st);
    dest[stp-st] = '\0';
    int result = atoi(dest);
    free(dest);
    return result;
}

int get_rec_id(char *buffer)
{
    int i = 1, st = 0, stp = 0; 
    if(buffer[0] != '%')
    {
        fprintf(stderr, "Missing receiver id from message\n");
        return -1;
    }

    st = i;
    while(buffer[i] != '%')
    {
        i++;
    }
    stp = i;

    char *sid = (buffer + st + 1);
    char *dest = malloc(stp-st+1);
    strncpy(dest, sid, stp-st);
    dest[stp-st] = '\0';
    int result = atoi(dest);
    free(dest);
    return result;
}

void clean_up_clients(pthread_t accept_thread)
{
    // Cleanup
    pthread_cancel(accept_thread);
    // Close remaining client connections
    for(int i = 0; i < client_count; i++) {
        if(clients[i].active) {
            close(clients[i].client_fd);
        }
    }

    return;
}

int *get_chat_clients_ids(char *buffer, int *id_numbers)
{

    if(buffer[0] != '%') {
        fprintf(stderr, "Missing receiver id from message\n");
        *id_numbers = 0;
        return NULL;
    }
    
    // Find the closing %
    int end_pos = 1;
    while(buffer[end_pos] != '%' && buffer[end_pos] != '\0') {
        end_pos++;
    }

    if(buffer[end_pos] != '%') {
        *id_numbers = 0;
        return NULL;
    }

    // Extract the ID string
    int len = end_pos - 1;
    char *id_str = malloc(len + 1);
    strncpy(id_str, buffer + 1, len);
    id_str[len] = '\0';


    // Count commas to determine array size
    int count = 1;
    for(int i = 0; i < len; i++) {
        if(id_str[i] == ',') count++;
    }
    
    int *result = malloc(count * sizeof(int));
    int index = 0;
    
    char *token = strtok(id_str, ",");
    while(token != NULL && index < count) {
        result[index++] = atoi(token);
        token = strtok(NULL, ",");
    }

    *id_numbers = index;
    free(id_str);
    return result;
}

client_message_type_t init_type_message(char *msg_types_buffer)
{
    int i = 1, st = 0, stp = 0; 
    if(msg_types_buffer[0] != '#')
    {
        fprintf(stderr, "Missing msg_type from message\n");
        return -1;
    }

    st = i;
    while(msg_types_buffer[i] != '#')
    {
        i++;
    }
    stp = i;

    char *sid = (msg_types_buffer + st + 1);
    char *dest = malloc(stp-st+1);
    strncpy(dest, sid, stp-st);
    dest[stp-st] = '\0';
    int result = atoi(dest);
    free(dest);
    return result;
}


void *handle_client(void *arg)
{
    client_t *client =  (client_t *)arg;
    char buffer[MAX_MESSAGE_LENGHT];
    char msg_type_buffer[16];   

    // Separate thread to handle each client operation
    while(server_running && client->active)
    {
        size_t __attribute_maybe_unused__ msg_types = recv(client->client_fd, msg_type_buffer, sizeof(msg_type_buffer)-1, 0);
        size_t __attribute_maybe_unused__ bytes = recv(client->client_fd, buffer, sizeof(buffer)-1, 0);

        if(msg_types <= 0 || bytes <= 0) {
            break; // Connection closed or error
        }

        if(msg_types > 0) {
            msg_type_buffer[msg_types] = '\0';
        }
        if(bytes > 0) {
            buffer[bytes] = '\0';
        }
        client->last_activity = time(NULL);

        
        int rec_id = -1;
        int chat_id = -1;
        int *ids  = {0};
        int id_number = 0;

        if(msg_types == 0) break;

        client_message_type_t new = init_type_message(msg_type_buffer);

        switch(new){
            case MSG_AUTH:
                // Will be implemented later
                break;
            case MSG_BROADCAST_CLIENTS:
                broadcast_message(client, buffer);
                break;
            case MSG_DISCONNECT:
                remove_client(client);
                break;
            case MSG_SEND: 
                rec_id = get_rec_id(buffer);
                if(rec_id < 0) break;
                client_t *receiver = find_client_by_id(rec_id);
                if(receiver == NULL || !receiver->active) {
                        char error_msg[] = "User not found";
                        send(client->client_fd, error_msg, strlen(error_msg), 0);
                        break;
                }

                message_t message = init_message(*client, buffer);
                message.receiver_user_id = rec_id;
                send(receiver->client_fd, (void *)&message, sizeof(message), 0);
                break;

            case MSG_CHANNEL: 
                ids = get_chat_clients_ids(buffer, &id_number);
                if(ids != NULL) {
                    send_to_chat(client, ids, id_number, buffer);
                    free(ids);
                }
                break;
                
            case MSG_LEAVE_CHANNEL: 
                chat_id = get_chat_id(buffer);
                if(chat_id < 0)
                {
                    break;
                }
                leave_chat(client, chat_id);
                break;
            case MSG_LIST_USERS: 
                list_users();
                break;
            case MSG_HEARTBEAT: 
                break;       
            case MSG_CREATE_CHANNEL:
                break;
            case MSG_JOIN_CHANNEL:
                break;
        }

        if(ids != NULL) {
            free(ids);
            ids = NULL;
        }
    }

    client->active = 0;
    close(client->client_fd);
    remove_client(client);
    return NULL;
}

void send_to_chat(client_t *sender, int *chat_clients_ids, int id_number, char *message)
{
    pthread_mutex_lock(&clients_mutex);
    for(int j = 0; j < id_number; j++) {
        int client_id = chat_clients_ids[j];
        if(client_id >= 0 && client_id < client_count && 
           clients[client_id].active && 
           clients[client_id].client_fd != sender->client_fd) {
            send(clients[client_id].client_fd, message, strlen(message), 0);
            clients[client_id].msg_count++;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Broadcast a cleint message to all other clients
void broadcast_message(client_t *sender, const char *message) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i].active && clients[i].client_fd != sender->client_fd) {
            send(clients[i].client_fd, message, strlen(message), 0);
            clients[i].msg_count++;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}


void mark_as_inactive(client_t *client) {
    pthread_mutex_lock(&clients_mutex);
    for(int i = 0; i < client_count; i++) {
        if(&clients[i] == client) {
            // Mark as inactive instead of shifting
            clients[i].active = 0;
            close(clients[i].client_fd);
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void remove_client(client_t *client) {
    mark_as_inactive(client);
}


int __attribute_maybe_unused__ leave_chat(client_t *client, size_t chat_id)
{
    /***
     * Will implement but lets first finish with the one on one communication
     */
    return 0;
}

char *list_users()
{
    /***
     * Will implement but lets first finish with the one on one communication
     */
    return 0;
}

void compact_clients() {
    pthread_mutex_lock(&clients_mutex);
    int write_pos = 0;
    for(int read_pos = 0; read_pos < client_count; read_pos++) {
        if(clients[read_pos].active) {
            if(write_pos != read_pos) {
                clients[write_pos] = clients[read_pos];
            }
            write_pos++;
        }
    }
    client_count = write_pos;
    pthread_mutex_unlock(&clients_mutex);
}

client_t* find_client_by_id(int client_id) {
    for(int i = 0; i < client_count; i++) {
        if(clients[i].active && clients[i].client_id == client_id) {
            return &clients[i];
        }
    }
    return NULL;
}

void *handle_client_timeout(void *arg) {
    client_t *client = (client_t*)arg;    
    while (client->active) {
        if (time(NULL) - client->last_activity > TIMEOUT) {
            // Client trop lent, déconnexion
            disconnect_slow_client(client);
            break;
        }
        sleep(60);
    }
    return NULL;
}

int disconnect_slow_client(client_t *client)
{   
    printf("Disconnecting.......\n");
    if(send_signal(client->client_fd, MSG_DISCONNECT) == -1)
    {
        fprintf(stderr, "Could Not Disconect %s\n", client->client_name);
        return -1;
    }

    mark_as_inactive(client);
    return 0;
}

int send_signal(int rec_fd, client_message_type_t msg)
{
    char signal_buffer[16];
    snprintf(signal_buffer, sizeof(signal_buffer), "#%d#", msg);
    
    if(send(rec_fd, signal_buffer, strlen(signal_buffer), 0) < 0) {
        perror("Failed to send signal");
        return -1;
    }
    return 0;
}

// Validation côté serveur
int validate_client_message(client_t *client, message_t *msg) {
    // Anti-spam
    if (client->msg_count > MAX_MESSAGES_PER_MINUTE) {
        return -1;
    }
    
    // Validation contenu
    if (msg->length > MAX_MESSAGE_LENGHT) {
        return -1;
    }
    
    return 0;
}


void free_chats(chat_t *chat)
{
    free(chat);
}


// int broadcast_updates(chat_t *chats, send_signal_t signal)
// {

//     return 0;
// }



void signal_handler(int sig) {

    switch (sig)
    {
        case SIGINT:
        case SIGTERM:
            printf("\nArrêt du serveur...\n");
            server_running = 0;   
            break;
        default:
            break;
    }
}
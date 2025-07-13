#ifndef SERVER_H
#define SERVER_H

#include "chat.h"

/***
 *The server 
 * header file 
 */

extern int start_message_server();

void *accept_connections(void *arg);

extern void display_messages(window_t *window, const char *user_name, const char *messages);
extern void update_client_list(window_t *window);
extern void update_status(window_t *window, const char *status);
extern void cleanup_interface(window_t *window);
extern void *handle_client(void *arg);
extern void broadcast_message(client_t *sender, const char *message);
int safe_send(int sockfd, const char *message, size_t len);
extern void free_chats(chat_t *chat);
extern int validate_client_message(client_t *client, message_t *msg);
extern int disconnect_slow_client(client_t *client);
extern void *handle_client_timeout(void *arg);
void signal_handler(int sig);
void remove_client(client_t *client);
void mark_as_inactive(client_t *client);
message_t init_message(client_t client, char *buffer);
client_message_type_t init_type_message(char* msg_types_buffer);
void send_to_chat(client_t *sender, int *ids, int id_number, char *message);
int send_signal(int rec_fd, client_message_type_t msg);
int get_chat_id(char *buffer);
int get_rec_id(char *buffer);
client_t* find_client_by_id(int client_id);
void clean_up_clients(pthread_t accept_thread);
void compact_clients();
char* extract_delimited_content(char *buffer, char start_char, char end_char, int *start_pos);
void _mark_as_incative(int i);


char *list_users();
int leave_chat(client_t *client, size_t chat_id);

#endif // !
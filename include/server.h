#ifndef SERVER_H
#define SERVER_H

#include "chat.h"

/***
 *The server 
 * header file 
 */

extern window_t *init_interface();
extern int start_message_server();

void *accept_connections(void *arg);

extern void display_messages(window_t *window, const char *user_name, const char *messages);
extern void update_client_list(window_t *window);
extern void update_status(window_t *window, const char *status);

extern void cleanup_interface(window_t *window);

extern void *handle_client(void *arg);
extern void broadcast_message(client_t *sender, const char *message);

#endif // !
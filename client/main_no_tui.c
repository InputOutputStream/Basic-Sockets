
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <signal.h>
// #include <unistd.h>
// #include <pthread.h>
// #include <time.h>

// #include <chat.h>
// #include <client.h>

// // Variables globales (same as original)
// pthread_t receive_thread;
// int client_fd = -1;
// volatile int client_running = 1;

// // Simple replacement functions for ncurses calls
// void simple_display_messages(const char* sender, const char* message) {
//     time_t now = time(NULL);
//     char* time_str = ctime(&now);
//     time_str[strlen(time_str)-1] = '\0'; // Remove newline
    
//     printf("[%s] %s: %s\n", time_str, sender, message);
//     fflush(stdout);
// }

// void simple_update_status(const char* status) {
//     printf("[STATUS] %s\n", status);
//     fflush(stdout);
// }

// void simple_update_client_list() {
//     // Just print a simple indicator - you can enhance this
//     printf("[DEBUG] Client list updated\n");
// }

// // Modified receive_messages for simple output
// void *simple_receive_messages(void *arg) {
//     int fd = *((int*)arg);  // This is client_fd - our connection to server
//     char buffer[MAX_MESSAGE_LENGHT];
//     time_t last_beat = time(NULL);

//     printf("[DEBUG] Receive thread started, listening on fd=%d\n", fd);

//     while (client_running) {
//         ssize_t bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);  // Receive from SERVER
//         if (bytes <= 0) {
//             if (client_running) {
//                 printf("[ERROR] Connection closed by server\n");
//                 client_running = 0;
//             }
//             break;
//         }
//         buffer[bytes] = '\0';
        
//         printf("[DEBUG] Received raw message: %s\n", buffer);

//         // Parse message type using existing function
//         int msg_type = get_message_type(buffer);
//         char *content = get_message_content(buffer);
        
//         if (!content) continue;

//         switch (msg_type) {
//             case MSG_BROADCAST: 
//                 // Format: sender_id:message
//                 char *colon = strchr(content, ':');
//                 if (colon) {
//                     *colon = '\0';
//                     char *sender_id = content;
//                     char *message = colon + 1;
                    
//                     char sender_name[32];
//                     snprintf(sender_name, sizeof(sender_name), "User%s", sender_id);
//                     simple_display_messages(sender_name, message);
//                 }
//                 break;
            
//             case MSG_USER_JOIN: 
//                 simple_display_messages("Système", content);
//                 break;
            
//             case MSG_USER_LEAVE: 
//                 simple_display_messages("Système", content);
//                 break;

//             case MSG_CHANNEL_CREATION: 
//                 simple_display_messages("Système", content);
//                 break;
            
//             case MSG_SEND: 
//                 // Private message from another user
//                 int start_pos = 0;
//                 char *sender_content = extract_delimited_content(content, '%', '%', &start_pos);
//                 if (sender_content) {
//                     char sender_name[32];
//                     snprintf(sender_name, sizeof(sender_name), "User%s", sender_content);
//                     simple_display_messages(sender_name, content + start_pos);
//                     free(sender_content);
//                 }
//                 break;
            
//             case MSG_USER_LIST: 
//                 printf("[INFO] User list updated: %s\n", content);
//                 break;
            
//             case MSG_PING: 
//                 send_formatted_message(MSG_HEARTBEAT, "", client_fd);
//                 printf("[DEBUG] Heartbeat sent\n");
//                 break;
            
//             case MSG_SERVER_ERROR: 
//                 simple_update_status(content);
//                 break;
            
//             case MSG_SERVER_INFO: 
//                 simple_display_messages("Serveur", content);
//                 break;
            
//             default:
//                 simple_display_messages("Inconnu", content);
//                 break;
//         }
        
//         free(content);

//         // Send heartbeat every 5 minutes (using existing function)
//         if (time(NULL) - last_beat > 300) {
//             send_formatted_message(MSG_HEARTBEAT, "", client_fd);
//             last_beat = time(NULL);
//         }
//     }
//     return NULL;
// }

// int main() {
//     const char *str_addr = "127.0.0.1";
//     const int port = 8008;
    
//     printf("=== SIMPLE DEBUG CHAT CLIENT ===\n");
//     printf("Démarrage de Low Chat...\n");
    
//     // Configuration signaux (using existing handler)
//     signal(SIGINT, client_signal_handler);
//     signal(SIGTERM, client_signal_handler);
    
//     simple_update_status("Connexion au serveur...");
    
//     // Connexion au serveur (using existing function)
//     client_fd = connect_to_server(str_addr, port);
//     if (client_fd < 0) {
//         fprintf(stderr, "ERROR❌: Impossible de se connecter au serveur\n");
//         return EXIT_FAILURE;
//     }

//     printf("[DEBUG] Connected to server, client_fd = %d\n", client_fd);
//     simple_update_status("✅ Connecté au serveur - Tapez vos messages");
//     printf("Type '/help' for commands, '/quit' to exit\n");
    
//     // Lancement thread de réception
//     pthread_create(&receive_thread, NULL, simple_receive_messages, &client_fd);
    
//     // Boucle principale - simple input loop
//     char input_buffer[MAX_MESSAGE_LENGHT];
    
//     // Commandes list (same as original)
//     char *commands[] = {"/help", "/quit", "/join <channel_id>", "/chat", 
//         "/leave_channel", "/create_channel <name>"};
//     static int commandes_number = 6;

//     while (client_running) {
//         printf(">> ");
//         fflush(stdout);
        
//         if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) {
//             break;
//         }
        
//         // Remove newline
//         input_buffer[strcspn(input_buffer, "\n")] = '\0';
        
//         if (strlen(input_buffer) == 0) continue;
        
//         // Commandes (same logic as original)
//         if (strcmp(input_buffer, "/help") == 0) {
//             simple_display_messages("Aide", "Commandes disponibles:");
//             for(int i = 0; i < commandes_number; i++)
//                 simple_display_messages("Commandes", commands[i]);
//         }
//         else if (strcmp(input_buffer, "/quit") == 0) {
//             client_running = 0;
//         }
//         else if (strncmp(input_buffer, "/join ", 6) == 0) {
//             char* channel_id = get_command_arg(input_buffer);
//             if (channel_id) {
//                 char formatted_msg[MAX_MESSAGE_LENGHT];
//                 snprintf(formatted_msg, sizeof(formatted_msg), "[%s]", channel_id);
                
//                 if (send_formatted_message(MSG_JOIN_CHANNEL, formatted_msg, client_fd) < 0) {
//                     simple_update_status("❌ Erreur lors de la connexion au canal");
//                 } else {
//                     simple_display_messages("Système", "Demande de connexion au canal envoyée");
//                 }
//             }
//             else{
//                 simple_display_messages("Erreur", "Usage: /join <channel_id>");
//             }
//         }
//         else if (strncmp(input_buffer, "/create_channel ", 16) == 0) {
//             char* channel_name = get_command_arg(input_buffer);
//             if (channel_name) {
//                 if (send_formatted_message(MSG_CREATE_CHANNEL, channel_name, client_fd) < 0) {
//                     simple_update_status("❌ Erreur lors de la création du canal");
//                 } else {
//                     simple_display_messages("Système", "Demande de création de canal envoyée");
//                 }
//             } else {
//                 simple_display_messages("Erreur", "Usage: /create_channel <nom_du_canal>");
//             } 
//         }
//         else if (strcmp(input_buffer, "/leave_channel") == 0) {
//             if (send_formatted_message(MSG_LEAVE_CHANNEL, "", client_fd) < 0) {
//                 simple_update_status("❌ Erreur lors de la sortie du canal");
//             } else {
//                 simple_display_messages("Système", "Demande de sortie du canal envoyée");
//             }
//         }
//         else if(strcmp(input_buffer, "/chat") == 0) {
//             simple_display_messages("Info", "Vous êtes en mode chat général");
//         } 
//         else {
//             printf("[DEBUG] Sending message: %s\n", input_buffer);
//             simple_display_messages("Vous", input_buffer);
            
//             if (send_formatted_message(MSG_BROADCAST_CLIENTS, input_buffer, client_fd) < 0) {
//                 simple_update_status("❌ Erreur envoi message");
//             } else {
//                 printf("[DEBUG] Message sent successfully\n");
//             }
//         }
//     }
    
//     // Cleanup (same as original)
//     client_running = 0;
//     if (client_fd >= 0) {
//         close(client_fd);
//     }
//     pthread_cancel(receive_thread);
//     pthread_join(receive_thread, NULL);

//     printf("✅ Chat fermé proprement\n");
//     return 0;
// }
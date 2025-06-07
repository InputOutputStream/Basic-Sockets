#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main() {
    // 1. Paramètres
    const char *ip_str = "192.168.232.171";
    int port = 8008;

    // 2. Création du socket
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // 3. Préparation de la structure sockaddr_in
    struct sockaddr_in *addr;
    addr = calloc(1, sizeof(struct sockaddr_in));
    
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);  // Convertit en big-endian
    if (inet_pton(AF_INET, ip_str, &addr->sin_addr) <= 0) {
        perror("inet_pton");
        exit(EXIT_FAILURE);
    }

    // 4. Bind
    if (bind(fd, (struct sockaddr*)addr, sizeof(struct sockaddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // 5. Écoute
    if (listen(fd, 10) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("✅ Serveur en écoute sur %s:%d\n", ip_str, port);

    // 6. Accept (bloquant)
    int client_fd = accept(fd, NULL, NULL);
    if (client_fd < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    printf("📡 Client connecté !\n");

    // Tu peux maintenant `recv()` ou `send()` depuis client_fd
    char *buff=calloc(6, sizeof(char)); 
    if(recv(client_fd, buff, 6, 0) < 0) {perror("Recv Failed\n"); exit(EXIT_FAILURE);}
    
    fprintf(stdout, "Server recieved Buff: %s\n", buff);

    free(addr);
    free(buff);
    return 0;
}

#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>



int main(){

    char str_addr[] = "192.168.232.171";
    int port = 8008;


    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { perror("socket"); exit(EXIT_FAILURE); }

    //Le client ce connecte 
    struct sockaddr_in addr = {0};

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, str_addr, &addr.sin_addr) <= 0) { perror("inet_pton"); exit(EXIT_FAILURE); }
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { perror("connect"); exit(EXIT_FAILURE); }
    if (send(fd, "Hello", sizeof(char)*6, 0) < 0) { perror("send"); exit(EXIT_FAILURE); }

    return 0;
}
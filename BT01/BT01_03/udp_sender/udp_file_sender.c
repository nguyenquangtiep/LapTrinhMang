#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {

    if (argc != 4) {
        printf("Use: %s <IP Address> <Port> <Filename>\n", argv[0]);
        return 1;
    }
    char *ip = argv[1];
    int port = atoi(argv[2]);
    char *filename = argv[3];

    int server_sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (server_sockfd < 0) {
        perror("[ERROR] Socket() failed.\n");
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("[ERROR] Opening file failed.\n");
        return 1;
    }

    int n = sendto(server_sockfd, filename, sizeof(filename), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (n < 0) {
        perror("[ERROR] Sending file name failed.\n");
        return 1;
    }

    char buffer[BUFFER_SIZE];

    while (fgets(buffer, BUFFER_SIZE, file) != NULL) {
        printf("[SENDING] Data: %s\n", buffer);

        if (sendto(server_sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
            perror("[ERROR] Sending data failed.\n");
            return 1;
        }
        memset(buffer, 0, BUFFER_SIZE);
    }

    strcpy(buffer, "END");
    sendto(server_sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));

    printf("[SUCCESS] Data transfer complete.\n");
    printf("[CLOSING] Disconnecting from the server.\n");

    fclose(file);
    close(server_sockfd);

    return 0;
}
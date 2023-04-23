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

    if (argc != 2) {
        printf("Use: %s <Port>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);

    int server_sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (server_sockfd < 0) {
        perror("[ERROR] Socket() failed.\n");
        return 1;
    }

    struct sockaddr_in server_addr, client_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int e = bind(server_sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (e < 0) {
        perror("[ERROR] Bind() failed.\n");
        return 1;
    }

    char buffer[BUFFER_SIZE];

    socklen_t addr_size = sizeof(client_addr);
    int n = recvfrom(server_sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &addr_size);
    if (n < 0) {
        perror("[ERROR] Receiving file name failed.\n");
        return 1;
    }
    printf("[SUCCESS] Received file name.\n");

    char *filename = buffer;

    FILE *file = fopen(filename, "w");
    if (file < 0) {
        perror("[ERROR] Opening file failed.\n");
        return 1;
    }
    printf("[SUCCESS] Opened file.\n");
    
    while (1) {
        n = recvfrom(server_sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &addr_size);
        if (strcmp(buffer, "END") == 0) {
            break;
        }
        printf("[RECEIVING] Data: %s\n", buffer);
        fprintf(file, "%s", buffer);
        memset(buffer, 0, BUFFER_SIZE);
    }

    printf("[SUCCESS] Data transfer complete.\n");
    printf("[CLOSING] Closing the server.\n");

    fclose(file);
    close(server_sockfd);

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>

#define IP "127.0.0.1"
#define PORT 9090
#define FILE_NAME "SOICT.txt"
#define BUFFER_SIZE 20

int main() {

    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        perror("[ERROR] Socket() failed.\n");
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(IP);
    server_addr.sin_port = htons(PORT);

    int e = connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (e == -1) {
        perror("[ERROR] Connect() failed.\n");
        return 1;
    }

    FILE *file = fopen(FILE_NAME, "r");
    if (file == NULL) {
        perror("[ERROR] Opening file failed.\n");
        return 1;
    }

    char buffer[BUFFER_SIZE];

    while (fgets(buffer, BUFFER_SIZE, file) != NULL) {
        if (send(sock, buffer, BUFFER_SIZE, 0) == -1) {
            perror("[ERROR] Send() failed.\n");
            return 1;
        }
    }

    printf("File data sent succesfully.\n");
    printf("Closing the connection.\n");

    fclose(file);
    close(sock);

    return 0;
}
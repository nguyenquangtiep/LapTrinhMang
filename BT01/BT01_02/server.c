#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>

#define IP "127.0.0.1"
#define PORT 8005
#define FILE_NAME "SOICT"
#define BUFFER_SIZE 20

int main() {

    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        perror("[ERROR] Socket() failed.\n");
        return 1;
    }

    struct sockaddr_in server_addr, client_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    int e = bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (e < 0) {
        perror("[ERROR] Bind() failed.\n");
        return 1;
    }

    if (listen(sock, 3) == 0) {
        printf("Listening...\n");
    }
    else {
        perror("[ERROR] Listen() failed.\n");
        return 1;
    }

    int client_size = sizeof(client_addr);
    int client_sock = accept(sock, (struct sockaddr*)&client_addr, &client_size);
    if (client_sock == -1) {
        perror("[ERROR] Accept() failed.\n");
        return 1;
    }

    char buffer[BUFFER_SIZE];
    char *p;
    int n;
    char pre_buf[10], cur_buf[10];
    char check_buf[20];
    int count = 0;

    while(1) {
        if (recv(client_sock, buffer, BUFFER_SIZE, 0) <= 0) {
            break;
        }

        memcpy(cur_buf, buffer, 9);
        cur_buf[9] = '\0';

        p = buffer;
        
        while ((p = strstr(p, "0123456789")) != NULL) {
            count++;
            p++;
        }

        if (pre_buf[0] != '\0' && cur_buf[0] != '\0') {
            strcpy(check_buf, pre_buf);
            strcat(check_buf, cur_buf);
            if (strstr(check_buf, "0123456789") != NULL) {
                count++;
            }
            else {
                memset(pre_buf, 0, sizeof(pre_buf));
                memset(cur_buf, 0, sizeof(cur_buf));
                memset(check_buf, 0, sizeof(check_buf));
            }
        }

        memcpy(pre_buf, buffer + strlen(buffer) - 9, 9);
        pre_buf[9] = '\0';

        memset(buffer, 0, BUFFER_SIZE);
    }

    printf("The result: %d\n", count);

    close(sock);

    return 0;
}
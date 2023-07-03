 #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_BUFFER_SIZE 1024
int main() {
    // Tạo socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        perror("Không thể tạo socket");
        exit(EXIT_FAILURE);
    }
    // Thiết lập địa chỉ và cổng của server
struct sockaddr_in serverAddress;
serverAddress.sin_family = AF_INET;
   serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); // Địa chỉ IP của server
    serverAddress.sin_port = htons(8080); // Cổng của server


// Kết nối đến server
    if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("Không thể kết nối đến server");
        exit(EXIT_FAILURE);
    }

    printf("Đã kết nối đến server.\n");

    // Tạo file MP3 để ghi dữ liệu nhận được
    FILE *file = fopen("received.mp3", "wb");
    if (file == NULL) {
        perror("Không thể tạo file");
        exit(EXIT_FAILURE);
    }

    // Nhận và ghi dữ liệu vào file từ server
    char buffer[MAX_BUFFER_SIZE];
    ssize_t bytesRead;
    while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
        fwrite(buffer, 1, bytesRead, file);
    }

    // Đóng file và đóng socket
    fclose(file);
    close(clientSocket);

    printf("File đã được nhận thành công.\n");

    return 0;
}
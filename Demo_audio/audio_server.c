#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_BUFFER_SIZE 1024

int main() {
    // Tạo socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Không thể tạo socket");
        exit(EXIT_FAILURE);
    }

    // Thiết lập địa chỉ và cổng của server
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(8080);

    // Bind socket với địa chỉ và cổng
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("Không thể bind socket");
        exit(EXIT_FAILURE);
    }

    // Lắng nghe các kết nối đến
    if (listen(serverSocket, 5) == -1) {
        perror("Không thể lắng nghe kết nối");
        exit(EXIT_FAILURE);
    }

    printf("Server đang lắng nghe kết nối...\n");

    // Chấp nhận kết nối từ client
    struct sockaddr_in clientAddress;
    socklen_t clientAddressLength = sizeof(clientAddress);
    int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLength);
    if (clientSocket == -1) {
        perror("Không thể chấp nhận kết nối");
        exit(EXIT_FAILURE);
    }

    printf("Client đã kết nối.\n");

    // Mở file MP3 để đọc
    FILE *file = fopen("audio.mp3", "rb");
    if (file == NULL) {
        perror("Không thể mở file");
        exit(EXIT_FAILURE);
    }

    // Đọc và gửi từng phần của file cho client
    char buffer[MAX_BUFFER_SIZE];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        if (send(clientSocket, buffer, bytesRead, 0) == -1) {
            perror("Không thể gửi dữ liệu");
            exit(EXIT_FAILURE);
        }
    }

    // Đóng file và đóng socket
    fclose(file);
    close(clientSocket);
    close(serverSocket);

    printf("File đã được gửi thành công.\n");

    return 0;
}
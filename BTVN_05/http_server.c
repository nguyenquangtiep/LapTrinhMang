#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/select.h>
#include <poll.h>
#include <dirent.h>
#include <sys/stat.h>

#define PORT 9090
#define BUFFER_SIZE 1024

void send_directory(int client_sock, const char* directory);
void send_file(int client_sock, const char* file_path);
void send_response(int client_sock, const char* response);
void handle_request(int client_sock, const char* request);

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Tạo socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Không thể tạo socket");
        exit(1);
    }

    // Cấu hình địa chỉ và cổng của server
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Gắn socket với địa chỉ và cổng
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Không thể gắn socket với địa chỉ và cổng");
        exit(1);
    }

    // Lắng nghe kết nối từ client
    if (listen(server_sock, 1) < 0) {
        perror("Lỗi trong quá trình lắng nghe");
        exit(1);
    }

    printf("HTTP server đang lắng nghe trên cổng %d...\n", PORT);

    while (1) {
        // Chấp nhận kết nối từ client
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_len);
        if (client_sock < 0) {
            perror("Lỗi trong quá trình chấp nhận kết nối");
            exit(1);
        }

        // Xử lý yêu cầu từ client
        handle_request(client_sock, ".");

        // Đóng socket của client
        close(client_sock);
    }

    // Đóng socket của server
    close(server_sock);

    return 0;
}

void send_directory(int client_sock, const char* directory) {
    DIR *dir;
    struct dirent *entry;
    char response[BUFFER_SIZE];

    dir = opendir(directory);
    if (dir == NULL) {
        sprintf(response, "Lỗi: Không thể mở thư mục %s\n", directory);
        send_response(client_sock, response);
        return;
    }

    sprintf(response, "<html><body><h2>Thư mục gốc</h2><ul>");

    // Đọc tất cả các tệp tin và thư mục trong thư mục hiện tại
    while ((entry = readdir(dir)) != NULL) {
        char file_path[BUFFER_SIZE];
        sprintf(file_path, "%s/%s", directory, entry->d_name);

        // Kiểm tra nếu là thư mục
        if (entry->d_type == DT_DIR) {
            sprintf(response + strlen(response), "<li><strong><a href=\"%s/\">%s</a></strong></li>", entry->d_name, entry->d_name);
        } else {
            sprintf(response + strlen(response), "<li><em><a href=\"%s\">%s</a></em></li>", entry->d_name, entry->d_name);
        }
    }

    sprintf(response + strlen(response), "</ul></body></html>");

    closedir(dir);

    send_response(client_sock, response);
}

void send_file(int client_sock, const char* file_path) {
    FILE *file;
    char buffer[BUFFER_SIZE];
    int bytes_read;

    file = fopen(file_path, "rb");
    if (file == NULL) {
        char response[BUFFER_SIZE];
        sprintf(response, "Lỗi: Không thể mở tệp tin %s\n", file_path);
        send_response(client_sock, response);
        return;
    }

    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        send(client_sock, buffer, bytes_read, 0);
    }

    fclose(file);
}

void send_response(int client_sock, const char* response) {
    char header[BUFFER_SIZE];
    sprintf(header, "HTTP/1.1 200 OK\r\nContent-Length: %lu\r\n\r\n", strlen(response));

    send(client_sock, header, strlen(header), 0);
    send(client_sock, response, strlen(response), 0);
}

void handle_request(int client_sock, const char* request) {
    char buffer[BUFFER_SIZE];
    recv(client_sock, buffer, BUFFER_SIZE, 0);

    char method[BUFFER_SIZE];
    char path[BUFFER_SIZE];

    sscanf(buffer, "%s %s", method, path);

    printf("Yêu cầu: %s %s\n", method, path);

    if (strcmp(method, "GET") == 0) {
        if (strcmp(path, "/") == 0) {
            send_directory(client_sock, ".");
        } else {
            // Xóa dấu '/' ở đầu path
            memmove(path, path + 1, strlen(path));

            struct stat path_stat;
            if (stat(path, &path_stat) == 0 && S_ISREG(path_stat.st_mode)) {
                send_file(client_sock, path);
            } else if (stat(path, &path_stat) == 0 && S_ISDIR(path_stat.st_mode)) {
                send_directory(client_sock, path);
            } else {
                send_response(client_sock, "Lỗi: Đường dẫn không hợp lệ\n");
            }
        }
    } else {
        send_response(client_sock, "Lỗi: Phương thức không được hỗ trợ\n");
    }
}

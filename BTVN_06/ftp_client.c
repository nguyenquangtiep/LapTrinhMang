#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

#define IP "172.22.192.1"
#define PORT 21
#define FILE_NAME "test.mp4"

int main() {
    int ctrl_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in ctrl_addr;
    ctrl_addr.sin_family = AF_INET;
    ctrl_addr.sin_addr.s_addr = inet_addr(IP);
    ctrl_addr.sin_port = htons(PORT); 

    if (connect(ctrl_socket, (struct sockaddr *)&ctrl_addr, sizeof(ctrl_addr))) {
        perror("connect() failed");
        return 1;
    }

    char buf[2048];
    
    // Nhan xau chao tu server
    int len = recv(ctrl_socket, buf, sizeof(buf), 0);
    buf[len] = 0;
    puts(buf);

    char username[64], password[64];

    while (1)
    {
        printf("Nhap username: ");
        scanf("%s", username);
        printf("Nhap password: ");
        scanf("%s", password);

        // Gui lenh USER
        sprintf(buf, "USER %s\r\n", username);
        send(ctrl_socket, buf, strlen(buf), 0);

        len = recv(ctrl_socket, buf, sizeof(buf), 0);
        buf[len] = 0;
        puts(buf);

        // Gui lenh PASS
        sprintf(buf, "PASS %s\r\n", password);
        send(ctrl_socket, buf, strlen(buf), 0);

        len = recv(ctrl_socket, buf, sizeof(buf), 0);
        buf[len] = 0;
        puts(buf);

        if (strncmp(buf, "230", 3) == 0)
        {
            printf("Dang nhap thanh cong.\n");
            break;
        }
        else
        {
            printf("Dang nhap that bai.\n");
        }
    }

    {
        // Mo ket noi du lieu
        // Gui lenh PASV
        send(ctrl_socket, "PASV\r\n", 6, 0);
        len = recv(ctrl_socket, buf, sizeof(buf), 0);
        buf[len] = 0;
        puts(buf);

        // Xu ly ket qua lenh PASV
        char *pos1 = strchr(buf, '(') + 1;
        char *pos2 = strchr(pos1, ')');
        char temp[256];
        int n = pos2 - pos1;
        memcpy(temp, pos1, n);
        temp[n] = 0;

        char *p = strtok(temp, ",");
        int i1 = atoi(p);
        p = strtok(NULL, ",");
        int i2 = atoi(p);
        p = strtok(NULL, ",");
        int i3 = atoi(p);
        p = strtok(NULL, ",");
        int i4 = atoi(p);
        p = strtok(NULL, ",");
        int p1 = atoi(p);
        p = strtok(NULL, ",");
        int p2 = atoi(p);

        struct sockaddr_in data_addr;
        data_addr.sin_family = AF_INET;
        sprintf(temp, "%d.%d.%d.%d", i1, i2, i3, i4);
        data_addr.sin_addr.s_addr = inet_addr(temp);
        data_addr.sin_port = htons(p1 * 256 + p2);

        int data_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        if (connect(data_socket, (struct sockaddr *)&data_addr, sizeof(data_addr))) {
            perror("connect() failed");
            return 1;
        }

        // Gui lenh LIST
        send(ctrl_socket, "LIST\r\n", 6, 0);
        len = recv(ctrl_socket, buf, sizeof(buf), 0);
        buf[len] = 0;
        puts(buf);

        // Nhan du lieu
        while (1)
        {
            len = recv(data_socket, buf, sizeof(buf), 0);
            if (len <= 0)
                break;
            buf[len] = 0;
            puts(buf);
        }

        close(data_socket);

        // Nhan phan hoi con lai cua lenh LIST
        len = recv(ctrl_socket, buf, sizeof(buf), 0);
        buf[len] = 0;
        puts(buf);
    }

    // Upload file
    {
        // Mo ket noi du lieu
        // Gui lenh PASV
        send(ctrl_socket, "PASV\r\n", 6, 0);
        len = recv(ctrl_socket, buf, sizeof(buf), 0);
        buf[len] = 0;
        puts(buf);

        // Xu ly ket qua lenh PASV
        char *pos1 = strchr(buf, '(') + 1;
        char *pos2 = strchr(pos1, ')');
        char temp[256];
        int n = pos2 - pos1;
        memcpy(temp, pos1, n);
        temp[n] = 0;

        char *p = strtok(temp, ",");
        int i1 = atoi(p);
        p = strtok(NULL, ",");
        int i2 = atoi(p);
        p = strtok(NULL, ",");
        int i3 = atoi(p);
        p = strtok(NULL, ",");
        int i4 = atoi(p);
        p = strtok(NULL, ",");
        int p1 = atoi(p);
        p = strtok(NULL, ",");
        int p2 = atoi(p);

        struct sockaddr_in data_addr;
        data_addr.sin_family = AF_INET;
        sprintf(temp, "%d.%d.%d.%d", i1, i2, i3, i4);
        data_addr.sin_addr.s_addr = inet_addr(temp);
        data_addr.sin_port = htons(p1 * 256 + p2);

        int data_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        if (connect(data_socket, (struct sockaddr *)&data_addr, sizeof(data_addr))) {
            perror("connect() failed");
            return 1;
        }

        /*
            Su dung lenh EPSV

            // Gui lenh EPSV
            send(ctrl_socket, "PASV\r\n", 6, 0);
            len = recv(ctrl_socket, buf, sizeof(buf), 0);
            buf[len] = 0;
            puts(buf);
        
            // Xu ly ket qua lenh EPSV
            
            char *pos1 = strchr(buf, '(') + 1;
            char *pos2 = strchr(pos1, ')');
            char temp[256];
            int n = pos2 - pos1;
            memcpy(temp, pos1, n);
            temp[n] = 0;

            char *p = strtok(temp, "|");
            p = strtok(NULL, "|");
            p = strtok(NULL, "|");
            p = strtok(NULL, "|");
            int pasv_port = atoi(p); 

            struct sockaddr_in data_addr;
            data_addr.sin_family = AF_INET;
            data_addr.sin_addr.s_addr = inet_addr(IP);
            data_addr.sin_port = htons(pasv_port);

            int data_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

            if (connect(data_socket, (struct sockaddr *)&data_addr, sizeof(data_addr))) {
                perror("connect() failed.\n");
                return 1;
            }

        */

        // Gui lenh STOR
        char tmp[7 + strlen(FILE_NAME)];
        sprintf(tmp, "STOR %s\r\n", FILE_NAME);
        puts(tmp);
        send(ctrl_socket, tmp, sizeof(tmp), 0);
        len = recv(ctrl_socket, buf, sizeof(buf), 0);
        buf[len] = 0;
        puts(buf);

        // Nhan du lieu
        FILE *f = fopen(FILE_NAME, "rb");
        size_t bytes_read;
        while ((bytes_read = fread(buf, 1, sizeof(buf), f)) > 0)
        {
            if (send(data_socket, buf, bytes_read, 0) == -1)
            {
                perror("Send() failed.\n");
            }
        }

        fclose(f);
        close(data_socket);

        // Nhan phan hoi con lai cua lenh LIST
        len = recv(ctrl_socket, buf, sizeof(buf), 0);
        buf[len] = 0;
        puts(buf);
    }

    {
        // Mo ket noi du lieu
        // Gui lenh PASV
        send(ctrl_socket, "PASV\r\n", 6, 0);
        len = recv(ctrl_socket, buf, sizeof(buf), 0);
        buf[len] = 0;
        puts(buf);

        // Xu ly ket qua lenh PASV
        char *pos1 = strchr(buf, '(') + 1;
        char *pos2 = strchr(pos1, ')');
        char temp[256];
        int n = pos2 - pos1;
        memcpy(temp, pos1, n);
        temp[n] = 0;

        char *p = strtok(temp, ",");
        int i1 = atoi(p);
        p = strtok(NULL, ",");
        int i2 = atoi(p);
        p = strtok(NULL, ",");
        int i3 = atoi(p);
        p = strtok(NULL, ",");
        int i4 = atoi(p);
        p = strtok(NULL, ",");
        int p1 = atoi(p);
        p = strtok(NULL, ",");
        int p2 = atoi(p);

        struct sockaddr_in data_addr;
        data_addr.sin_family = AF_INET;
        sprintf(temp, "%d.%d.%d.%d", i1, i2, i3, i4);
        data_addr.sin_addr.s_addr = inet_addr(temp);
        data_addr.sin_port = htons(p1 * 256 + p2);

        int data_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        if (connect(data_socket, (struct sockaddr *)&data_addr, sizeof(data_addr))) {
            perror("connect() failed");
            return 1;
        }

        // Gui lenh LIST
        send(ctrl_socket, "LIST\r\n", 6, 0);
        len = recv(ctrl_socket, buf, sizeof(buf), 0);
        buf[len] = 0;
        puts(buf);

        // Nhan du lieu
        while (1)
        {
            len = recv(data_socket, buf, sizeof(buf), 0);
            if (len <= 0)
                break;
            buf[len] = 0;
            puts(buf);
        }

        close(data_socket);

        // Nhan phan hoi con lai cua lenh LIST
        len = recv(ctrl_socket, buf, sizeof(buf), 0);
        buf[len] = 0;
        puts(buf);
    }

    // Gui lenh QUIT
    send(ctrl_socket, "QUIT\r\n", 6, 0);
    len = recv(ctrl_socket, buf, sizeof(buf), 0);
    buf[len] = 0;
    puts(buf);

    // Ket thuc, dong socket
    close(ctrl_socket);

    return 0;
}
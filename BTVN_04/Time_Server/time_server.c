#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

#define PORT 9090
#define BUFFER_SIZE 1024

void *client_thread(void *);
void process_request(int, char *);

int main()
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed.\n");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed.\n");
        return 1;
    }

    if (listen(listener, 5))
    {
        perror("listen() failed.\n");
        return 1;
    }

    while (1)
    {
        int client = accept(listener, NULL, NULL);
        if (client == -1)
        {
            perror("accept() failed.\n");
            continue;
        }
        printf("New client connected: %d\n", client);

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, client_thread, &client);
        pthread_detach(thread_id);
    }

    close(listener);

    return 0;
}

void *client_thread(void *param)
{
    int client = *(int *)param;
    char buf[BUFFER_SIZE];

    while (1)
    {
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
        {
            break;
        }

        buf[ret] = 0;
        printf("Received from %d: %s\n", client, buf);

        process_request(client, buf);
    }

    close(client);
}

void process_request(int client, char *buf)
{
    
    char format[12], tmp[12];
    int ret = sscanf(buf, "GET_TIME %s%s", format, tmp);
    if (ret == 1)
    {
        printf("Format: %s\n", format);
        if (strcmp(format, "[dd/mm/yyyy]") != 0 && strcmp(format, "[dd/mm/yy]") != 0 && strcmp(format, "[mm/dd/yyyy]") != 0 && strcmp(format, "[mm/dd/yy]") != 0)
        {
            char *msg = "Nhap sai cu phap. Hay nhap lai.\n";
            send(client, msg, strlen(msg), 0);
        }
        else
        {
            printf("Dang xu ly///\n");
            memset(tmp, 0, sizeof(tmp));
            time_t t;
            time(&t);
            struct tm *tm;
            tm = localtime(&t);
            if (strcmp(format, "[dd/mm/yyyy]") == 0)
            {
                printf("Dinh dang: %s\n", format);
                strftime(tmp, sizeof(tmp), "%d/%m/%Y\n", tm);
                printf("Thoi gian: %s\n", tmp);
            }
            else if (strcmp(format, "[mm/dd/yyyy]") == 0)
            {
                printf("Dinh dang: %s\n", format);
                strftime(tmp, sizeof(tmp), "%m/%d/%Y\n", tm);
                printf("Thoi gian: %s\n", tmp);
            }
            else if (strcmp(format, "[dd/mm/yy]") == 0)
            {
                printf("Dinh dang: %s\n", format);
                strftime(tmp, sizeof(tmp), "%d/%m/%y\n", tm);
                printf("Thoi gian: %s\n", tmp);
            }
            else
            {
                printf("Dinh dang: %s\n", format);
                strftime(tmp, sizeof(tmp), "%x\n", tm);
                printf("Thoi gian: %s\n", tmp);
            }
            send(client, tmp, strlen(tmp), 0);
        }
    }
    else
    {
        char *msg = "Nhap sai cu phap. Hay nhap lai.\n";
        send(client, msg, strlen(msg), 0);
    }
}
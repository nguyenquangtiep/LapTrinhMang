#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <signal.h>
#include <string.h>
#include <time.h>

#define PORT 9090
#define BUFFER_SIZE 1024

void signalHandler(int signo)
{
    int pid = wait(NULL);
    printf("Child %d terminated.\n", pid);
}

void process_request(int client, char *buf);

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

    signal(SIGCHLD, signalHandler);

    while (1)
    {
        printf("Waiting for new client.\n");
        int client = accept(listener, NULL, NULL);
        printf("New client accepted: %d.\n", client);
        if (fork() == 0)
        {
            close(listener);
            char buf[BUFFER_SIZE];
            while(1)
            {
                int ret = recv(client, buf, sizeof(buf), 0);
                if (ret <= 0)
                    break;
                buf[ret] = 0;
                printf("Received from %d: %s\n", client, buf);
                send(client, buf, strlen(buf), 0);
                process_request(client, buf);
            }
            close(client);
            exit(0);
        }
        close(client);
    }

    close(listener);

    return 0;
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
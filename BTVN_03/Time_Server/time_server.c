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
    char format[10], tmp[10];
    int ret = sscanf(buf, "GET_TIME %s%s", format, tmp);
    if (ret == 1)
    {
        printf("Format: %s\n", format);
    }
    else
    {
        char *msg = "Nhap sai cu phap. Hay nhap lai.\n";
        send(client, msg, strlen(msg), 0);
    }
}
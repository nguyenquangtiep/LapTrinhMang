#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>

#define PORT 9000
#define BUFFER_SIZE 1024

int main()
{
    // Khoi tao server
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

    int num_process = 8;
    for (int i = 0; i < num_process; i++)
    {
        if (fork() == 0)
        {
            char buf[BUFFER_SIZE];
            while (1)
            {
                int client = accept(listener, NULL, NULL);
                printf("New client connected: %d\n", client);
                int ret = recv(client, buf, sizeof(buf), 0);
                if (ret <= 0)
                {
                    continue;
                }
                buf[ret] = 0;
                printf("Received from %d: %s\n", client, buf);
                char *msg = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Xin chao cac ban</h1></body></html>";
                send(client, msg, strlen(msg), 0);
                close(client);
            }
            exit(0);
        }
    }

    getchar();

    close(listener);
    killpg(0, SIGKILL);

    return 0;
}
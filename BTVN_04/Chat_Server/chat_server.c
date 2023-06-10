#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define BUFFER_SIZE 1024
#define PORT 9090

void *client_thread(void *);

int users[64];
char *user_ids[64];
int num_users = 0;

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

        int j = 0;
        for (; j < num_users; j++)
        {
            if (users[j] == client) break;
        }

        if (j == num_users)
        {
            char cmd[32], id[32], tmp[32];
            ret = sscanf(buf, "%s%s%s", cmd, id, tmp);
            if (ret == 2)
            {
                if (strcmp(cmd, "client_id:") == 0)
                {
                    char *msg = "Dung cu phap. Gui tin nhan.\n";
                    send(client, msg, strlen(msg), 0);

                    int k = 0;
                    for(; k < num_users; k++)
                    {
                        if (strcmp(user_ids[k], id) == 0)
                            break;
                    }

                    if (k < num_users)
                    {
                        char *msg = "ID da ton tai. Yeu cau nhap lai.\n";
                        send(client, msg, strlen(msg), 0);
                    }
                    else
                    {
                        users[num_users] = client;
                        user_ids[num_users] = malloc(strlen(id) + 1);
                        strcpy(user_ids[num_users], id);
                        num_users++;
                    }
                }
                else
                {
                    char *msg = "Nhap sai. Yeu cau nhap lai.\n";
                    send(client, msg, strlen(msg), 0);
                }
            }
            else
            {
                char *msg = "Nhap sai. Yeu cau nhap lai.\n";
                send(client, msg, strlen(msg), 0);
            }
        }
        else
        {
            char sendbuf[2*BUFFER_SIZE];
            sprintf(sendbuf, "%s: %s", user_ids[j], buf);

            for (int k = 0; k < num_users; k++)
            {
                if (users[k] != client)
                {
                    send(users[k], sendbuf, strlen(sendbuf), 0);
                }
            }
        }
    }

    close(client);
}
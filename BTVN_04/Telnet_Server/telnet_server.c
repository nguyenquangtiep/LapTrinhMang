#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

#define PORT 9090
#define FILE_USER "users.txt"
#define BUFFER_SIZE 1024

int users_id[64];
int num_users = 0;

void *thread_proc(void *);
void process_request(int, char*);
void remove_user(int);

int main()
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("listen() failed.\n");
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

    int num_threads = 8;
    pthread_t thread_id;
    for (int i = 0; i < num_threads; i++)
    {
        pthread_create(&thread_id, NULL, thread_proc, &listener);
    }
    pthread_join(thread_id, NULL);

    close(listener);

    return 0;
}

void *thread_proc(void *param)
{
    int listener = *(int *)param;
    char buf[BUFFER_SIZE];
    while(1)
    {
        int client = accept(listener, NULL, NULL);
        if (client == -1)
        {
            perror("accept() failed.\n");
            continue;
        }

        printf("New client accepted: %d\n", client);

        while (1)
        {
            int ret = recv(client, buf, sizeof(buf), 0);
            if (ret <= 0)
            {
                remove_user(client);
                continue;
            }

            buf[ret] = 0;
            printf("Received from %d: %s", client, buf);

            process_request(client, buf);
        }
    }
    close(listener);
}

void process_request(int client, char *buf)
{
    int i = 0;
    for(;i < num_users; i++)
    {
        if (users_id[i] == client)
            break;
    }

    if (i == num_users)
    {
        char user[32], pass[32], tmp[66], line[66];
        int ret = sscanf(buf, "%s%s%s", user, pass, tmp);
        if (ret == 2)
        {
            sprintf(tmp, "%s %s\n", user, pass);
            FILE *f = fopen(FILE_USER, "r");
            int found = 0;
            while (fgets(line, sizeof(line), f) != NULL)
            {
                if (strcmp(line, tmp) == 0)
                {
                    found = 1;
                    break;
                }
            }
            fclose(f);

            if (found)
            {
                char *msg = "Dang nhap thanh cong. Hay nhap lenh de thuc hien.\n";
                send(client, msg, strlen(msg), 0);

                users_id[num_users] = client;
                num_users++;
            }
            else
            {
                char *msg = "Nhap sai tai khoan. Hay nhap lai.\n";
                send(client, msg, strlen(msg), 0);
            }
        }
        else
        {
            char *msg = "Nhap sai cu phap. Hay nhap lai.\n";
            send(client, msg, strlen(msg), 0);
        }
    }
    else
    {
        // Da dang nhap
        char tmp[BUFFER_SIZE];

        // Xoa dau xuong dong neu co
        if (buf[strlen(buf) - 1] == '\n')
        {
            buf[strlen(buf) - 1] = '\0';
        }

        sprintf(tmp, "%s > out.txt", buf);
        int ret = system(tmp);
        
        if (ret == 0)
        {
            FILE *f = fopen("out.txt", "rb");
            while(!feof(f))
            {
                ret = fread(tmp, 1, sizeof(tmp), f);
                if (ret <= 0)
                {
                    break;
                }
                send(client, tmp, ret, 0);
            }
            fclose(f);
        }
        else
        {
            char *msg = "Lenh khong thuc hien duoc.\n";
            send(client, msg, strlen(msg), 0);
        }
    }
}

void remove_user(int client)
{
    int i = 0;
    for (; i < num_users; i++)
    {
        if (users_id[i] == client)
        {
            break;
        }
    }
    if (i < num_users)
    {
        if (i < num_users - 1)
        {
            users_id[i] = users_id[num_users - 1];
        }
        num_users--;
    }
}
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>

#define PORT 9000
#define BUFFER_SIZE 1024
#define DIR_PATH "."

void *client_thread(void *);

int main() 
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) 
    {
        perror("bind() failed");
        return 1;
    }

    if (listen(listener, 5)) 
    {
        perror("listen() failed");
        return 1;
    }

    while (1)
    {
        int client = accept(listener, NULL, NULL);
        if (client == -1)
        {
            perror("accept() failed");
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

    // Doc noi dung thu muc
    DIR *dir = opendir(DIR_PATH);
    struct dirent *entry;
    int count = 0;

    char *file_list = malloc(1);
    file_list[0] = 0;

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_REG)
        {
            file_list = realloc(file_list, strlen(file_list) + strlen(entry->d_name) + 2 + 1);
            strcat(file_list, entry->d_name);
            strcat(file_list, "\r\n");
            count++;
        }
    }

    closedir(dir);

    if (count == 0)
    {
        char *msg = "ERROR No files to download.\r\n";
        send(client, msg, strlen(msg), 0);
        close(client);
        exit(0);
    }
    else{
        sprintf(buf, "OK %d\r\n", count);
        send(client, buf, strlen(buf), 0);
        send(client, file_list, strlen(file_list), 0);
        send(client, "\r\n", 2, 0);
    }

    while (1)
    {
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
            break;
        
        if (buf[ret-1] == '\n')
        {
            buf[ret-1] = 0;
        }
        else
        {
            buf[ret] = 0;
        }

        FILE *f = fopen(buf, "rb");
        if (f == NULL)
        {
            char *msg = "ERROR Cannot open file.\r\n";
            send(client, msg, strlen(msg), 0);
            close(client);
            exit(0);
        }
        else
        {
            fseek(f, 0, SEEK_END);
            long file_size = ftell(f);
            fseek(f, 0, SEEK_SET);

            sprintf(buf, "OK %ld\r\n", file_size);
            send(client, buf, strlen(buf), 0);

            while(!feof(f))
            {
                int ret = fread(buf , 1, sizeof(buf), f);
                if (ret <= 0)
                {
                    break;
                }
                send(client, buf, ret, 0);
            }

            fclose(f);
        }
    }

    close(client);
}
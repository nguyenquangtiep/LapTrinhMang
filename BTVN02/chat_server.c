#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <poll.h>

#define MAX_CLIENTS 64

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
    addr.sin_port = htons(9009);

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

    struct pollfd fds[MAX_CLIENTS];
    char *user_ids[MAX_CLIENTS];
    int num_clients = 0;
    int nfds = 1;

    fds[0].fd = listener;
    fds[0].events = POLLIN;

    char buf[256];

    while (1)
    {
        int ret = poll(fds, nfds, -1);
        if (ret < 0)
        {
            perror("poll() failed");
            break;
        }

        printf("ret = %d\n", ret);

        if (fds[0].revents & POLLIN)
        {
            int client = accept(listener, NULL, NULL);
            if (nfds < MAX_CLIENTS)
            {
                printf("New client connected: %d\n", client);
                fds[nfds].fd = client;
                fds[nfds].events = POLLIN;
                nfds++;
            }
            else
            {
                printf("Too many connections\n");
                close(client);
            }
        }

        for (int i = 1; i < nfds; i++)
            if (fds[i].revents & POLLIN)
            {
                ret = recv(fds[i].fd, buf, sizeof(buf), 0);
                if (ret <= 0)
                {
                    printf("Client %d disconnected.\n", fds[i].fd);
                    close(fds[i].fd);
                    
                    // Xoa phan tu i khoi mang
                    if (i < nfds - 1)
                    {
                        fds[i] = fds[nfds - 1];
                        user_ids[i-1] = user_ids[num_clients - 1];
                    }
                    nfds--;
                    num_clients--;
                    i--;
                }
                else
                {
                    int client = fds[i].fd;
                    buf[ret] = 0;
                    printf("Received from %d: %s\n", fds[i].fd, buf);


                    if (user_ids[i-1] == NULL)
                    {
                        // Xu ly cu phap yeu cau dang nhap
                        char cmd[32], id[32], tmp[32];
                        ret = sscanf(buf, "%s%s%s", cmd, id, tmp);
                        if (ret == 2)
                        {
                            if (strcmp(cmd, "client_id:") == 0)
                            {
                                char *msg = "Dung cu phap. Gui tin nhan.\n";
                                send(client, msg, strlen(msg), 0);

                                int k = 0;
                                for (; k < num_clients; k++)
                                    if (strcmp(user_ids[k], id) == 0)
                                        break;
                                
                                if (k < num_clients)
                                {
                                    char *msg = "ID da ton tai. Yeu cau nhap lai.\n";
                                    send(client, msg, strlen(msg), 0);
                                }
                                else
                                {
                                    user_ids[num_clients] = malloc(strlen(id) + 1);
                                    strcpy(user_ids[num_clients], id);
                                    num_clients++;
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
                }
            }
    }

    close(listener);    

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

void *client_thread(void *);

int users[64];
int num_users = 0;
char *user_ids[64];
int root;

int check_upper_case(char c) {
    if (('a' <= c && c <= 'z') || ('0' <= c && c <= '9')) return 1;
    return 0;
}

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
    addr.sin_port = htons(9090);

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
    char buf[256];
    int isLog = 0;
    char cmd[32], id[32], tmp[32];

    while (1)
    {
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
            break;
        
        buf[ret] = 0;
        printf("Received from %d: %s\n", client, buf);

        if (!isLog)
        {
            // Chua dang nhap
            // Xu ly cu phap yeu cau dang nhap
            
            ret = sscanf(buf, "%s%s%s", cmd, id, tmp);
            if (ret == 2)
            {
                if (strcmp(cmd, "join") == 0)
                {
                    int k = 0;
                    for (; k < num_users; k++)
                        if (strcmp(user_ids[k], id) == 0) break;
                    
                    if (k < num_users)
                    {
                        char *msg = "200 NICKNAME IN USE\n";
                        send(client, msg, strlen(msg), 0);
                    }
                    else
                    {
                        int h = 0;
                        for (; h < strlen(id); h++)
                        {
                          if (!check_upper_case(id[h]))
                          {
                            break;
                          }
                        }
                        if (h < strlen(id))
                        {
                            char *msg = "201 INVALID NICK NAME\n";
                            send(client, msg, strlen(msg), 0);
                        }
                        else
                        {
                            users[num_users] = client;
                            user_ids[num_users] = malloc(strlen(id) + 1);
                            strcpy(user_ids[num_users], id);
                            num_users++;
                            isLog = 1;
                            char *msg = "100 OK\n";
                            send(client, msg, strlen(msg), 0);
                            for (int i = 0; i < num_users; i++)
                            {
                                char sendbuf[512];
                                sprintf(sendbuf, "JOIN %s\n", id);
                                if (users[i] != client)
                                    send(users[i], sendbuf, strlen(sendbuf), 0);
                            }
                            if (num_users == 1) root = users[0];
                        }
                    }                                    
                }
                else
                {
                    char *msg = "999 UNKNOWN ERROR\n";
                    send(client, msg, strlen(msg), 0);
                }
            }
            else
            {
                char *msg = "999 UNKNOWN ERROR\n";
                send(client, msg, strlen(msg), 0);
            }
        }
        else
        {
            // Da dang nhap
            char cmd[32];
            ret = sscanf(buf, "%s", cmd);
            char sendbuf[512];
            
            if (strcmp(cmd, "msg") == 0)
            {
                char recvbuf[100], tmp[32];
                ret = sscanf(buf, "%s%s%s", cmd, recvbuf, tmp);

                if (ret >= 2) {
                    char *msg = "100 OK\n";
                    send(client, msg, strlen(msg), 0);
                    sprintf(sendbuf, "MSG %s %s", id, buf + 4);
                    for (int i = 0; i < num_users; i++)
                    {
                        if (users[i] != client)
                            send(users[i], sendbuf, strlen(sendbuf), 0);
                    }
                }
                else
                {
                    char *msg = "999 UNKNOWN ERROR\n";
                    send(client, msg, strlen(msg), 0);
                }
            }
            else if (strcmp(cmd, "pmsg") == 0)
            {
                char recvbuf[100], tmp[32], recv_id[32];
                ret = sscanf(buf, "%s%s%s%s", cmd, recv_id, recvbuf, tmp);

                if (ret >= 3) {
                    char *msg = "100 OK\n";
                    send(client, msg, strlen(msg), 0);
                    sprintf(sendbuf, "PMSG %s %s", id, buf + strlen(id) + 6);
                    for (int i = 0; i < num_users; i++)
                    {
                        if (strcmp(user_ids[i], recv_id) == 0)
                        {
                            send(users[i], sendbuf, strlen(sendbuf), 0);
                            break;
                        }
                    }
                }
                else
                {
                    char *msg = "999 UNKNOWN ERROR\n";
                    send(client, msg, strlen(msg), 0);
                }
            }
            else if (strcmp(cmd, "op") == 0)
            {
                char id[100], tmp[32];
                ret = sscanf(buf, "%s%s%s", cmd, id, tmp);
                if (ret == 2) {
                    if (root != client) {
                        char *msg = "203 DENIED\n";
                        send(client, msg, strlen(msg), 0);  
                    }
                    else
                    {
                        int j = 0;
                        for (; j < num_users; j++)
                        {
                            if (strcmp(user_ids[j], id) == 0)
                                break;
                        }
                        if (j < num_users)
                        {
                            char *msg = "100 OK\n";
                            send(client, msg, strlen(msg), 0);
                            sprintf(sendbuf, "OP %s\n", id);
                            for (int i = 0; i < num_users; i++)
                            {
                                if (users[i] != client)
                                    send(users[i], sendbuf, strlen(sendbuf), 0);
                                if (strcmp(user_ids[i], id) == 0)
                                    root = users[i];
                            }
                        }
                        else
                        {
                            char *msg = "202 UNKNOWN NICKNAME\n";
                            send(client, msg, strlen(msg), 0);
                        }
                    }
                }
                else
                {
                    char *msg = "999 UNKNOWN ERROR\n";
                    send(client, msg, strlen(msg), 0);
                }
            }
            else
            {
                char *msg = "999 UNKNOWN ERROR\n";
                send(client, msg, strlen(msg), 0);
            }
        }
    }

    close(client);
}
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>

#define PORT 9090
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 64

void deleteElement(int clients[], char clients_name[][BUFFER_SIZE], int n, int *num_clients) {
    for (int i = n; i < *num_clients; i++) {
        clients[i] = clients[i+1];
        strcpy(clients_name[i], clients_name[i+1]);
    }
    *num_clients = *num_clients - 1;
    clients[*num_clients] = 0;
}

char* getTime() {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char *c;
    char tmp[20];
    sprintf(c, "%d/", tm.tm_year + 1900);
    if (tm.tm_mon + 1 < 10) {
        if (tm.tm_mday < 10) {
            sprintf(tmp, "0%d/0%d ", tm.tm_mon + 1, tm.tm_mday);
        }
        else {
            sprintf(tmp, "0%d/%d ", tm.tm_mon + 1, tm.tm_mday);
        }
    }
    else {
        if (tm.tm_mday < 10) {
            sprintf(tmp, "%d/0%d ", tm.tm_mon + 1, tm.tm_mday);
        }
        else {
            sprintf(tmp, "%d/%d ", tm.tm_mon + 1, tm.tm_mday);
        }
    }
    strcat(c, tmp);
    memset(tmp, 0, sizeof(tmp));
    if (tm.tm_hour > 12) {
        if (tm.tm_hour - 12 < 10) {
            if (tm.tm_min < 10) {
                if (tm.tm_sec < 10) {
                    sprintf(tmp, "0%d:0%d:0%dPM", tm.tm_hour - 12, tm.tm_min, tm.tm_sec);
                }
                else {
                    sprintf(tmp, "0%d:0%d:%dPM", tm.tm_hour - 12, tm.tm_min, tm.tm_sec);
                }
            }
            else {
                if (tm.tm_sec < 10) {
                    sprintf(tmp, "0%d:%d:0%dPM", tm.tm_hour - 12, tm.tm_min, tm.tm_sec);
                }
                else {
                    sprintf(tmp, "0%d:%d:%dPM", tm.tm_hour - 12, tm.tm_min, tm.tm_sec);
                }
            }
        }
        else {
            if (tm.tm_min < 10) {
                if (tm.tm_sec < 10) {
                    sprintf(tmp, "%d:0%d:0%dPM", tm.tm_hour - 12, tm.tm_min, tm.tm_sec);
                }
                else {
                    sprintf(tmp, "%d:0%d:%dPM", tm.tm_hour - 12, tm.tm_min, tm.tm_sec);
                }
            }
            else {
                if (tm.tm_sec < 10) {
                    sprintf(tmp, "%d:%d:0%dPM", tm.tm_hour - 12, tm.tm_min, tm.tm_sec);
                }
                else {
                    sprintf(tmp, "%d:%d:%dPM", tm.tm_hour - 12, tm.tm_min, tm.tm_sec);
                }
            }
        }
    }
    else {
        if (tm.tm_hour < 10) {
            if (tm.tm_min < 10) {
                if (tm.tm_sec < 10) {
                    sprintf(tmp, "0%d:0%d:0%dAM", tm.tm_hour, tm.tm_min, tm.tm_sec);
                }
                else {
                    sprintf(tmp, "0%d:0%d:%dAM", tm.tm_hour, tm.tm_min, tm.tm_sec);
                }
            }
            else {
                if (tm.tm_sec < 10) {
                    sprintf(tmp, "0%d:%d:0%dAM", tm.tm_hour, tm.tm_min, tm.tm_sec);
                }
                else {
                    sprintf(tmp, "0%d:%d:%dAM", tm.tm_hour, tm.tm_min, tm.tm_sec);
                }
            }
        }
        else {
            if (tm.tm_min < 10) {
                if (tm.tm_sec < 10) {
                    sprintf(tmp, "%d:0%d:0%dAM", tm.tm_hour, tm.tm_min, tm.tm_sec);
                }
                else {
                    sprintf(tmp, "%d:0%d:%dAM", tm.tm_hour, tm.tm_min, tm.tm_sec);
                }
            }
            else {
                if (tm.tm_sec < 10) {
                    sprintf(tmp, "%d:%d:0%dAM", tm.tm_hour, tm.tm_min, tm.tm_sec);
                }
                else {
                    sprintf(tmp, "%d:%d:%dAM", tm.tm_hour, tm.tm_min, tm.tm_sec);
                }
            }
        }
    }
    strcat(c, tmp);
    return c;
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

    fd_set fdread;
    int clients[MAX_CLIENTS];  
    char clients_name[MAX_CLIENTS][BUFFER_SIZE];
    int num_clients = 0;

    char buf[BUFFER_SIZE];
    char err_full[] = "Connecting new client failed. Client list was full.\n";
    char name_rcv_msg[] = "Enter the client name with the following syntax: \"client_id: client_name\"\n";

    while (1)
    {
        FD_ZERO(&fdread);

        FD_SET(listener, &fdread);
        for (int i = 0; i < num_clients; i++)
            FD_SET(clients[i], &fdread);

        int ret = select(FD_SETSIZE, &fdread, NULL, NULL, NULL);

        if (FD_ISSET(listener, &fdread))
        {
            int client = accept(listener, NULL, NULL);
            // TODO: Kiem tra gioi han
            if (num_clients == sizeof(clients)/sizeof(int)) {
                printf("%s", err_full);
                send(client, err_full, sizeof(err_full), 0);
                close(client);
            }
            else {
                clients[num_clients++] = client;
                printf("New client connected %d\n", client);
                send(client, name_rcv_msg, sizeof(name_rcv_msg), 0);
            }
        }

        for (int i = 0; i < num_clients; i++)
            if (FD_ISSET(clients[i], &fdread) && strlen(clients_name[i]) != 0)
            {
                int ret = recv(clients[i], buf, sizeof(buf), 0);
                if (ret <= 0)
                {
                    printf("Closed from %d\n", clients[i]);
                    // TODO: Xoa client ra khoi mang
                    deleteElement(clients, clients_name, i, &num_clients);
                    continue;
                }

                buf[ret] = 0;
                time_t t = time(NULL);
                struct tm tm = *localtime(&t);

                char *time = getTime();

                printf("Received from %d: %s\n", clients[i], buf);

                char tmp[ret];
                char client_name[strlen(clients_name[i])];

                strcpy(tmp, buf);
                strcpy(client_name, clients_name[i]);

                memset(buf, 0, BUFFER_SIZE);
                sprintf(buf, "%s %s: %s", time, client_name, tmp);

                for (int j = 0; j < num_clients; j++) {
                    if (i != j && strlen(clients_name[j]) != 0) {
                        send(clients[j], buf, sizeof(buf), 0);
                    }
                }
            }
            else if (FD_ISSET(clients[i], &fdread) && strlen(clients_name[i]) == 0) {
                int ret = recv(clients[i], buf, sizeof(buf), 0);
                if (ret <= 0) {
                    printf("Closed from %d\n", clients[i]);
                    deleteElement(clients, clients_name, i, &num_clients);
                }
                buf[ret] = 0;
                if (sscanf(buf, "client_id: %s", clients_name[i]) == 1) {
                    send(clients[i], "Connected to server.\n", sizeof("Connected to server.\n"), 0);
                }
                else {
                    send(clients[i], name_rcv_msg, sizeof(name_rcv_msg), 0);
                }
            }
    }
    
    close(listener);    

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>

#define PORT 8989
#define BUFFER_SIZE 1024
#define CLIENT_NUM 64

void deleteElement(int clients[], int n, int *num_clients) {
    for (int i = n; i < *num_clients; i++) {
        clients[i] = clients[i+1];
    }
    *num_clients = *num_clients - 1;
    clients[*num_clients] = 0;
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
    int clients[CLIENT_NUM];  
    char clients_name[CLIENT_NUM][BUFFER_SIZE];
    int num_clients = 0;

    char buf[BUFFER_SIZE];
    char tmp[] = "Connecting new client failed. Client list was full.\n";

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
                printf("%s",tmp);
                send(client, tmp, sizeof(tmp), 0);
            }
            else {
                do {
                    
                } while(1);
                clients[num_clients++] = client;
                printf("New client connected %d\n", client);
            }
        }

        for (int i = 0; i < num_clients; i++)
            if (FD_ISSET(clients[i], &fdread))
            {
                int ret = recv(clients[i], buf, sizeof(buf), 0);
                if (ret <= 0)
                {
                    printf("Closed from %d\n", clients[i]);
                    // TODO: Xoa client ra khoi mang
                    deleteElement(clients, i, &num_clients);
                    continue;
                }

                buf[ret] = 0;
                printf("Received from %d: %s\n", clients[i], buf);
            }
    }
    
    close(listener);    

    return 0;
}
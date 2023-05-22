#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <poll.h>

#define PORT 7777
#define MAX_CLIENTS 64
#define BUFFER_SIZE 1024
#define FILE_INPUT "account.txt"
#define FILE_OUTPUT "output.txt"

int main() {

    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("[ERROR] Socket() failed.\n");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    if (bind(listener, (struct sockaddr*)&addr, sizeof(addr)))
    {
        perror("[ERROR] Bind() failed.\n");
        return 1;
    }

    if (listen(listener, 5))
    {
        perror("[ERROR] Listen() failed.\n");
        return 1;
    }

    struct pollfd fds[MAX_CLIENTS];
    int user[MAX_CLIENTS];
    user[0] = 1;
    for (int i = 1; i < MAX_CLIENTS; i++)
    {
        user[i] = 0;
    }
    int nfds = 1;

    char buf1[BUFFER_SIZE];
    char buf2[BUFFER_SIZE];

    fds[0].fd = listener;
    fds[0].events = POLLIN;

    

    while (1)
    {
        int ret = poll(fds, nfds, -1);
        if (ret < 0)
        {
            perror("[ERROR] Poll() failed.\n");
            break;
        }

        // Xu ly su kien co client ket noi
        if (fds[0].revents & POLLIN)
        {
            int client = accept(listener, NULL, NULL);
            printf("[PROCESS1] New client connected.\n");
            if (nfds < MAX_CLIENTS)
            {
                printf("[PROCESS] New client connected.\n");
                fds[nfds].fd = client;
                fds[nfds].events = POLLIN;
                nfds++;
                char *tmp = "[CONNECTED] Input your account by the following: \"user password\"\n";
                send(client, tmp, strlen(tmp), 0);
            }
            else
            {
                char *tmp = "[REFUSE] Too many connections.\n";
                printf("%s", tmp);
                send(client, tmp, strlen(tmp), 0);
                close(client);
            }
        }


        // Xu ly su kien co du lieu tu client da ket noi
        for (int i = 1; i < nfds; i++)
        {
            if (fds[i].revents & POLLIN)
            {
                ret = recv(fds[i].fd, buf1, BUFFER_SIZE, 0);
                if (ret <= 0)
                {
                    printf("[NOTIFY] Client %d disconnected.\n", fds[i].fd);
                    close(fds[i].fd);

                    // Xoa phan tu ra khoi mang
                    if (i < nfds - 1)
                    {
                        fds[i] = fds[nfds - 1];
                    }
                    nfds--;
                    i--;
                }
                else
                {
                    buf1[ret-1] = 0;

                    printf("[CLIENT] Data: %s\n", buf1);
                    
                    // Kiem tra dang nhap
                    if (user[i] == 0)
                    {
                        int check = 0;
                        FILE *file = fopen(FILE_INPUT, "r");
                        while (fgets(buf2, BUFFER_SIZE, file) != NULL)
                        {
                            buf2[strlen(buf2) - 1] = 0;
                            if (strcmp(buf1, buf2) == 0)
                            {
                                check = 1;
                                break;
                            }
                            memset(buf2, 0, BUFFER_SIZE);
                        }
                        fclose(file);

                        // Xu ly dang nhap
                        if (check)
                        {
                            char *tmp = "[SERVER] Successful connection.\n";
                            send(fds[i].fd, tmp, strlen(tmp), 0);
                            char username[32], password[32];
                            sscanf(buf1, "%s %s", username, password);
                            printf("[NOTIFY] Account \"user: %s - password: %s\" has been login.\n", username, password);
                            user[i] = 1;
                        }
                        else
                        {
                            char *tmp = "[SERVER] Login error.\n";
                            send(fds[i].fd, tmp, strlen(tmp), 0);
                        }
                    }
                    else
                    {   
                        char query[strlen(buf1) + strlen(FILE_OUTPUT) + 3];
                        strcpy(query, buf1);
                        strcat(query, " > ");
                        strcat(query, FILE_OUTPUT);

                        printf("[EXE] Query: %s\n", query);

                        system(query);

                        FILE *fp = fopen(FILE_OUTPUT, "r");
                        while (fgets(buf2, BUFFER_SIZE, fp) != NULL)
                        {
                            printf("Data: %s\n", buf2);
                            send(fds[i].fd, buf2, strlen(buf2), 0);
                            memset(buf2, 0, BUFFER_SIZE);
                        }
                        fclose(fp);
                    }
                }
            }
        }
    }

    close(listener);

    return 0;

}
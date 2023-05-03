#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

static int port;

int checkParam(int argc, char const *argv[])
{
    if (argc != 2)
    {
        printf("Check params\n");
        return 1;
    }

    // port check
    if (sscanf(
        argv[1],
        "%d",
        &port
    ) != 1)
    {
        printf("Check port\n");
        return 1;
    }
}

int main(int argc, char const *argv[])
{
    checkParam(argc, argv);

    int socket_receiver = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in receiver_info;
    receiver_info.sin_family = AF_INET;
    receiver_info.sin_addr.s_addr = htonl(INADDR_ANY);
    receiver_info.sin_port = htons(port);

    int bind_1 = bind(socket_receiver, (struct sockaddr*)&receiver_info, sizeof(receiver_info));
    if (bind_1)
    {
        printf("Unable to bind: %d - %s\n",
            errno,
            strerror(errno)
        );
        return 1;
    } printf("Bind success\n");

    char buf[256];
    FILE *f = fopen("receive.dat", "wb");

    struct sockaddr_in client_info;
    int client_infoLength = sizeof(client_info);
    char client_ip[15];
    char client_port[5];
    while (1)
    {
        int recv_1 = recvfrom(socket_receiver, buf, sizeof(buf)
        , 0, (struct sockaddr*)&client_info, &client_infoLength);
        printf("Receive: %d\n",recv_1);
        buf[recv_1] = 0;

        strcpy(client_ip, inet_ntoa(client_info.sin_addr));
        fwrite("\n", 1, 1, f);
        fwrite(client_ip, 1, strlen(client_ip), f);
        printf("ip: %s\n",client_ip);
        sprintf(client_port, "%d", htons(client_info.sin_port));
        printf("port: %s\n",client_port);
        fwrite("-", 1, 1, f);
        fwrite(client_port, 1, strlen(client_port), f);
        fwrite(": ", 1, 2, f);
        fwrite(buf, 1, strlen(buf), f);

        memset(client_ip, 0, sizeof(client_ip));
        memset(client_port, 0, sizeof(client_port));

        if (strstr(buf,"[_stop server_]")) break;
    }
    
    fclose(f);

    return 0;
}

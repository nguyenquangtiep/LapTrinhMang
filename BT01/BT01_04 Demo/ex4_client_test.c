#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>

static int byte1,byte2,byte3,byte4;
static int port;
static char fileName[256];

int checkParam(int argc, char const *argv[])
{
    if (argc != 4)
    {
        printf("Check params\n");
        return 1;
    }

    // check file name
    if (sscanf(
        argv[1],
        "%s",
        fileName
    ) != 1)
    {
        printf("Check hello file\n");
        return 1;
    }

    // check ip
    if (sscanf(
        argv[2],
        "%d.%d.%d.%d",
        &byte1,&byte2,&byte3,&byte4
        ) != 4
    )
    {
        printf("Check params IP");
        return 1;
    }

    // port check
    if (sscanf(
        argv[3],
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

    int socket_1 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in receiver_info;
    receiver_info.sin_family = AF_INET;
    char address[16];
    sprintf(address, "%d.%d.%d.%d",byte1,byte2,byte3,byte4);
    receiver_info.sin_addr.s_addr = inet_addr(address);
    receiver_info.sin_port = htons(port);

    char buf[256];

    FILE *f = fopen(fileName, "rb");
    sendto(socket_1, fileName, strlen(fileName), 0
    , (struct sockaddr*)&receiver_info, sizeof(receiver_info)
    );
    sendto(socket_1, "\n", 1, 0
    , (struct sockaddr*)&receiver_info, sizeof(receiver_info)
    );

    while(!feof(f))
    {
        int read_1 = fread(buf, 1, sizeof(buf), f);
        buf[read_1] = 0;
        sendto(socket_1, buf, strlen(buf), 0
        , (struct sockaddr*)&receiver_info, sizeof(receiver_info)
        );
    }

    fclose(f);

    return 0;
}

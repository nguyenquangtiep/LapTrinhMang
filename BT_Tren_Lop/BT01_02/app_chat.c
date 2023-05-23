#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

int main(int argc, char *argv[])
{
    // argv[1]: dia chi IP
    // argv[2]: cong nhan du lieu
    // argv[3]: cong gui du lieu

    if (argc != 4)
    {
        printf("Su dung: %s <IP> <des_port> <src_por>", argv[0]);
        return 1;
    }

    int sender_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sender_sock < 0)
    {
        perror("Socket() failed.\n");
        return 1;
    }

    int receiver_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (receiver_sock < 0)
    {
        perror("Socket() failed.\n");
        return 1;
    }

    struct sockaddr_in receiver_addr;
    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_addr.s_addr = INADDR_ANY;
    receiver_addr.sin_port = htons(atoi(argv[3]));

    struct sockaddr_in sender_addr;
    sender_addr.sin_family = AF_INET;
    sender_addr.sin_addr.s_addr = inet_addr(argv[1]);
    sender_addr.sin_port = htons(atoi(argv[2]));

    int e = bind(receiver_sock, (struct sockaddr*)&receiver_addr, sizeof(receiver_addr));
    if (e < 0)
    {
        perror("Bind() failed.\n");
        return 1;
    }

    char buf[1024];

    fd_set fdread, fdtest;
    FD_ZERO(&fdread);
    FD_SET(STDIN_FILENO, &fdread);
    FD_SET(receiver_sock, &fdread);

    while (1)
    {
        fdtest = fdread;
        int ret = select(receiver_sock + 1, &fdtest, NULL, NULL, NULL);
        if (ret < 0)
        {
            perror("select() failed.\n");
            return 1;
        }

        if (FD_ISSET(STDIN_FILENO, &fdtest))
        {
            fgets(buf, sizeof(buf), stdin);
            sendto(sender_sock, buf, strlen(buf), 0, (struct sockaddr*)&sender_addr, sizeof(sender_addr));
        }

        if (FD_ISSET(receiver_sock, &fdtest))
        {
            ret = recvfrom(receiver_sock, buf, sizeof(buf), 0, NULL, NULL);
            buf[ret] = 0;
            printf("[Received]: %s\n", buf);
        }
    }

    close(receiver_sock);
    close(sender_sock);

    return 0;
}
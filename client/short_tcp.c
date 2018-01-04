#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define KEEPALIVE_TIME 1000  //保持连接的时间
#define HEARTBEAT_TIME 10  //发送心跳包时间间隔

in_addr_t inet_addr(const char *cp);

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        printf("Usage: %s ip port message\n", argv[0]);
        exit(1);
    }
    struct sockaddr_in addr;
    int sock;

    if ( (sock=socket(AF_INET, SOCK_STREAM, 0)) <0)
    {
        perror("socket");
        exit(1);
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    if (addr.sin_addr.s_addr == INADDR_NONE)
    {
        printf("Incorrect ip address!");
        close(sock);
        exit(1);
    }
    int n,i;
    n = connect(sock,(struct sockaddr *)&addr,sizeof(addr));
    if (n < 0)
    {
        perror("connect");
        close(sock);
    }
    char buff[512];
    for(i=0;i<KEEPALIVE_TIME;i+=HEARTBEAT_TIME)
    {
        n = send(sock, argv[3], strlen(argv[3]), 0);
        if (n < 0)
        {
            perror("sendto");
            close(sock);
        }
        n = recv(sock, buff, 512, 0);
        if (n>0)
        {
            buff[n] = 0;
            printf("received:\n");
            puts(buff);
        }
        else if (n==0)
        {
            printf("server closed\n");
            close(sock);
        }
        sleep(3);
    }
    if (n == -1)
    {
        perror("recv");
        //close(sock);
    }
    return 0;
}

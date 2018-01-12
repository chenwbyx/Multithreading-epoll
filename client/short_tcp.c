#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "pack_JSON.h"

#define IP_LEN 16
#define MAC_LEN 18
#define UDP_PACKAGE_LEN 55

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
    char pack_ip[IP_LEN],pack_mac[MAC_LEN];

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
    int n;
    n = connect(sock,(struct sockaddr *)&addr,sizeof(addr));
    if (n < 0)
    {
        perror("connect");
        close(sock);
    }
    char buff[512]={"short"};
    int len;
    strcat(buff,argv[3]);
    n = send(sock, buff, strlen(buff), 0);
    if (n < 0)
    {
        perror("sendto");
        close(sock);
    }
    buff[0]='\0';
    n = recv(sock, buff, 512, 0);
    if (n>0)
    {
        buff[n] = 0;
        dis_udp_package(buff, pack_ip, pack_mac);
        printf("received:\nip:%s\nmac:%s\n",pack_ip,pack_mac);
    }
    else if (n==0)
    {
        printf("server closed\n");
        close(sock);
    }
    else if (n == -1)
    {
        perror("recv");
        close(sock);
    }

    return 0;
}

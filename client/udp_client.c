#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
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

    if ( (sock=socket(AF_INET, SOCK_DGRAM, 0)) <0)
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

    int len;
    char pack_mes[UDP_PACKAGE_LEN],pack_ip[IP_LEN],pack_mac[MAC_LEN];

    int n;
    n = sendto(sock, argv[3], strlen(argv[3]), 0, (struct sockaddr *)&addr, sizeof(addr));
    if (n < 0)
    {
        perror("sendto");
        close(sock);
    }
    n = recvfrom(sock, pack_mes, 512, 0, (struct sockaddr *)&addr, &len);
    if (n>0)
    {
        printf("received:\n");
        dis_udp_package(pack_mes, pack_ip, pack_mac);
        printf( "IP: %s \nMAC: %s \n",pack_ip,pack_mac );
    }
    else if (n==0)
    {
        printf("server closed\n");
        close(sock);
    }
    else if (n == -1)
    {
        perror("recvfrom");
        close(sock);
    }

    return 0;
}

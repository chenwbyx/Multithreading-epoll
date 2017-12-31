#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <pthread.h>
#include "thread_pool.h"
#include "pack_JSON.h"

#define MAX_EVENT_NUMBER 1024
#define TCP_BUFFER_SIZE 512
#define UDP_BUFFER_SIZE 1024
#define THREAD_NUMBER 10
#define IP_LEN 16
#define MAC_LEN 18
#define UDP_PACKAGE_LEN 55
#define ETH_NAME "ens33" //网卡名字

int setnonblocking( int fd )
{//设置非阻塞
    int old_option = fcntl( fd, F_GETFL );
    int new_option = old_option | O_NONBLOCK;
    fcntl( fd, F_SETFL, new_option );
    return old_option;
}

void addfd( int epollfd, int fd )
{//注册套接字句柄
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN|EPOLLET;
    epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &event );
    setnonblocking( fd );
}

void get_ip_mac( char *ip,char *mac,int len_limit)
{//获取网卡ip地址和mac地址
    int   sock;
    struct   sockaddr_in   sin;
    struct   ifreq   ifr;

    sock   =   socket(AF_INET,   SOCK_DGRAM,   0);
    if   (sock   ==   -1)
    assert( sock >= 0 );

    strncpy(ifr.ifr_name,   ETH_NAME,   IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ   -   1]   =   0;

    if   (ioctl(sock,   SIOCGIFADDR,   &ifr)   ==  0)  //获取ip
    {
        memcpy(&sin,   &ifr.ifr_addr,   sizeof(sin));
        strcpy(ip,inet_ntoa(sin.sin_addr));
        //fprintf(stdout,   "eth0:   %s\n",   inet_ntoa(sin.sin_addr));
    }
    if( ioctl( sock, SIOCGIFHWADDR, &ifr ) == 0 )   //获取mac
    {
        snprintf ( mac, len_limit, "%X:%X:%X:%X:%X:%X", (unsigned char) ifr.ifr_hwaddr.sa_data[0],
                (unsigned char) ifr.ifr_hwaddr.sa_data[1], (unsigned char) ifr.ifr_hwaddr.sa_data[2],
                (unsigned char) ifr.ifr_hwaddr.sa_data[3], (unsigned char) ifr.ifr_hwaddr.sa_data[4],
                (unsigned char) ifr.ifr_hwaddr.sa_data[5] );
        //printf( "adapter hardware address %x:%x:%x:%x:%x:%x\n",arp[0], arp[1], arp[2], arp[3], arp[4], arp[5] );
    }
}

void * handle_tcp_mes( void *arg )
{//处理任务
    int rs=1,ret=999,fd=(long)arg;
    char recvBuf[TCP_BUFFER_SIZE];
    while(rs)
    {
        printf("received tcp message: \n");
        ret=recv(fd,recvBuf,TCP_BUFFER_SIZE,0);
        if(ret<0)
        {
            if(errno==EAGAIN){printf("EAGAIN\n");break;}//缓冲区无数据
            else {printf("recv error!\n");close(fd);break;}
        }
        else if(ret==0)
        {//socket正常关闭
            rs=0;
            printf("%s", recvBuf);
        }
        else
            printf("%s", recvBuf);

        //需要再次读取
        if(ret==sizeof(recvBuf))
            rs=1;
        else
            rs=0;
    }
    printf("\n");
    if(ret>0)
    {//服务器回复消息并关闭scket
        char buf[1000] = {0};
        sprintf(buf,"hello! I am epoll_server.");
        send( fd, buf, strlen(buf), 0 );
        //close(fd);
    }
    return NULL;
}

int main( int argc, char* argv[] )
{
    if( argc <= 2 )
    {
        printf( "usage: %s ip_address port_number\n", argv[0]);
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi( argv[2] );
    //创建线程池
    thread_pool_t pool;
    pool=thread_pool_create(THREAD_NUMBER);

    char pack_mes[UDP_PACKAGE_LEN],pack_ip[IP_LEN];
    char pack_mac[MAC_LEN];

    int ret = 0;
    //创建tcp套接字，并绑定 、监听
    struct sockaddr_in address;
    bzero( &address, sizeof( address ) );
    address.sin_family = AF_INET;
    inet_pton( AF_INET, ip, &address.sin_addr );
    address.sin_port = htons( port );

    int listenfd = socket( PF_INET, SOCK_STREAM, 0 );
    assert( listenfd >= 0 );

    ret = bind( listenfd, ( struct sockaddr* )&address, sizeof( address ) );
    assert( ret != -1 );

    ret = listen( listenfd, 5 );
    assert( ret != -1 );
    //创建udp套接字，并绑定
    bzero( &address, sizeof( address ) );
    address.sin_family = AF_INET;
    inet_pton( AF_INET, ip, &address.sin_addr );
    address.sin_port = htons( port );
    int udpfd = socket( PF_INET, SOCK_DGRAM, 0 );
    assert( udpfd >= 0 );

    ret = bind( udpfd, ( struct sockaddr* )&address, sizeof( address ) );
    assert( ret != -1 );

    struct epoll_event events[ MAX_EVENT_NUMBER ];
    int epollfd = epoll_create( 5 );
    assert( epollfd != -1 );
    //注册tcp和udp套接字的可读事件
    addfd( epollfd, listenfd );
    addfd( epollfd, udpfd );

    while( 1 )
    {
        int number = epoll_wait( epollfd, events, MAX_EVENT_NUMBER, -1 );
        if ( number < 0 )
        {
            printf( "epoll failure\n" );
            break;
        }
        int i;
        long arg;
        for (i = 0; i < number; i++ )
        {
            int sockfd = events[i].data.fd;

            if ( sockfd == listenfd )
            {
            	//tcp有新的可读事件，也即接受到了新的连接
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof( client_address );
                int connfd = accept( listenfd, ( struct sockaddr* )&client_address, &client_addrlength );
                //将新的连接套接字也注册可读事件
                addfd( epollfd, connfd );
            }

            else if ( sockfd == udpfd )
            {
            	//udp套接字事件处理
                char buf[ UDP_BUFFER_SIZE ];
                memset( buf, '\0', UDP_BUFFER_SIZE );
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof( client_address );

                ret = recvfrom( udpfd, buf, UDP_BUFFER_SIZE-1, 0, ( struct sockaddr* )&client_address, &client_addrlength );
                printf("received udp message from %s : \n %s\n", inet_ntoa(client_address.sin_addr),buf);
                if( ret > 0 )
                {
                    get_ip_mac( pack_ip,pack_mac,sizeof(pack_mac) );
                    create_udp_package( pack_mes, pack_ip, pack_mac );
                    sendto( udpfd, pack_mes, sizeof(pack_mes), 0, ( struct sockaddr* )&client_address, client_addrlength );
                }
            }
            //注册的socket发生可读事件
            else if ( events[i].events & EPOLLIN )
            {
                arg=sockfd;
                thread_pool_add_task(pool, handle_tcp_mes, (void*)arg);
            }
            else
            {
                printf( "something else happened \n" );
            }
        }
    }
    close( listenfd );
    return 0;
}

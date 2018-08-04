//heart.h
#ifndef HEART_H_INCLUDED
#define HEART_H_INCLUDED

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


typedef struct cli_session
{
    char name[16];
    int sockfd;
}c_s_t;

typedef struct session
{
    char name[16];
    int sockfd;
    int count;
    struct session *next;
}s_t;
s_t *s_head = NULL;


void *send_heart(void *addr)
{
    int* client_sockfd = (int*)addr;
    printf("client_socket: %d\n", *client_sockfd);
    char buf[BUFSIZ]="127.0.0.1";
    //s_t *pd = (s_t *)malloc(sizeof(s_t));
    //strcpy(pd->name, "127.0.0.1");
    while(1)
    {
        send(*client_sockfd,buf,sizeof(buf),0);
        sleep(3);
    }
    free(client_sockfd);
    return NULL;
}

void init_shead()
{
    s_head = (s_t *)malloc(sizeof(s_t));
}

void heart_handler(int sockfd,char *name)
{
    s_t *cur = s_head->next;

    while( NULL != cur){
        if(strcmp(cur->name,name) == 0){
                cur->count = 0;
                printf("客户端  %s 正常在线 \n",name);
            }
            cur = cur->next;
        }
}

void check_handler()
{
    s_t *temp = NULL;
    s_t **ppNode = &s_head->next;
    while(NULL != (*ppNode))
    {
        if((*ppNode)->count == 5)
        {
            printf("客户端 %s 已经掉线\n",(*ppNode)->name);
            //close((*ppNode)->sockfd);
            temp = *ppNode;
            *ppNode = (*ppNode)->next;
            free(temp);
            temp = NULL;
            continue;
        }
        else if((*ppNode)->count > 0)
        {
            printf("客户端 %s 连接异常\n",(*ppNode)->name);
            (*ppNode)->count++;
            printf("count = %d\n",(*ppNode)->count);
            ppNode = &((*ppNode)->next);
            continue;
        }
        else if((*ppNode)->count == 0)
        {
            (*ppNode)->count++;
            ppNode = &((*ppNode)->next);
        }
        else;
    }
}

void *heart_check(void *p)
{
    printf("心跳检测线程已开启！\n");
    while(1)
    {
        check_handler(); //心跳检测处理函数
        sleep(3); //定时3秒
    }
    return NULL;
}


#endif // HEART_H_INCLUDED

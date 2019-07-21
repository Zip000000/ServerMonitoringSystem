/*************************************************************************
	> File Name: x.Master.c
	> Author: Zip 
	> Mail: 307110017@qq.com 
	> Created Time: 2019年07月20日 星期六 14时24分45秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>
#include <sys/ioctl.h>


#include "Sock.c"
#include "ClntList.c"
#include "Epoll.c"


#define PORT 8888
#define MAX_EVENTS 100




int get_conf_value(const char *file, const char *key, char *val) {
    if (key == NULL || val == NULL) {printf("Wrong parameters\n"); return -1;}
    
    FILE *fp = fopen(file, "r");
    if (fp == NULL) {perror("fopen"); return -1;}
    
    int readnum;
    char *line= (char *)malloc(sizeof(char) * 100);
    size_t n;

    while ((readnum = getline(&line, &n, fp)) != -1) {
        char *p = strstr(line, key);
        if (p == NULL) continue;
        int len = strlen(key);
        if(p[len] != '=') continue;
        strncpy(val, p+len+1, (int)readnum - len - 2);
        break;
    }
    if(readnum == 0) {
        printf("%s Not Found!\n", key);
        free(line);
        return 1;
    }
    return 0;
}

void *heartbeat (void *arg) {
    int cnt = 0;
    while(1) {
        printf("子线程进行第%d次心跳遍历检测\n",cnt);
        
        clntnode *c = all_clnt->head->next;
		char clntHPORT[100];
		memset(clntHPORT, 0, sizeof(clntHPORT));
		get_conf_value("config", "ClntHPORT", clntHPORT);

        while(c) {
            int sock = socket(AF_INET, SOCK_STREAM, 0);
            struct sockaddr_in serv_addr;
            memset(&serv_addr, 0, sizeof(serv_addr));
            serv_addr.sin_family = AF_INET;

            struct in_addr in;
            in.s_addr = c->ip;
            serv_addr.sin_addr.s_addr = inet_addr(inet_ntoa(in));
            serv_addr.sin_port = htons(atoi(clntHPORT));
            printf("clntHPORT = %s\n", clntHPORT);
            unsigned long ul = 1;
            ioctl(sock, FIONBIO, &ul);
            int con_ret = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
            perror("connect");
            if(con_ret < 0 && errno == EINPROGRESS) {
                fd_set wfds;
                FD_ZERO(&wfds);
                FD_SET(sock, &wfds);
                struct timeval tv;
                tv.tv_sec = 0;
                tv.tv_usec = 300000;
                int ret = select(sock + 1, NULL, &wfds, NULL, &tv);
                if(ret == 0) {
                    printf("<%s>超时， 心跳失败，收尸！\n", inet_ntoa(in));
                } else if(ret >= 0) {
                    int error = -1;
                    int len = sizeof(error);
                    if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len) < 0) {
                        perror("getsockopt");
                    }else if(error == 0) {
                        printf("getsockopt, error = %d\n", error);
                        printf("<%s> getsockopt之后，心跳成功！！！\n", inet_ntoa(in));
                    } else {
                        printf("getsockopt, error = %d\n", error);
                        printf("<%s> getsockopt之后，心跳失败，收尸！\n", inet_ntoa(in));
                    }
                } else {
                    perror("select");
                }
                c=c->next;
            } else {
                printf("<%s> 心跳异常，收尸！\n", inet_ntoa(in));
                c=c->next;
            }
            close(sock);
        }
        
        //show_list(all_clnt);
        sleep(1);
        printf("第%d次心跳遍历检测, OVER\n\n",cnt);
        cnt++;
    }

}

int main() {
    all_clnt = Clnt_Info_list_init();
    
    char masterIP[100];
    char masterPORT[100];
    memset(masterIP, 0, sizeof(masterIP));
    memset(masterPORT, 0, sizeof(masterPORT));
    get_conf_value("config", "MasterIP", masterIP);
    get_conf_value("config", "MasterPORT", masterPORT);
    char startIP[100];
    char endIP[100];
    memset(startIP, 0, sizeof(startIP));
    memset(endIP, 0, sizeof(endIP));
    get_conf_value("config", "startIP", startIP);
    get_conf_value("config", "endIP", endIP);
    printf("start = %s end = %s \n", startIP, endIP);

    unsigned int startip = htonl(inet_addr(startIP));
    unsigned int endip = htonl(inet_addr(endIP));
    
    for(unsigned i = startip; i <= endip; i++) {
        struct in_addr in;
        in.s_addr = ntohl(i);
        all_clnt = List_add(all_clnt, ntohl(i));
        printf("%s 插入完毕\n", inet_ntoa(in));
    }

    pthread_t pthread_id;
	pthread_create(&pthread_id, NULL, heartbeat, NULL);

    int listen_socket = get_listen_socket(masterIP, atoi(masterPORT));
    if(listen_socket < 0) exit(1);
    
    int epollfd;
    struct epoll_event events[MAX_EVENTS];
    epollfd = epoll_create(1);
    add_event(epollfd, listen_socket, EPOLLIN);
    int clnt_socket;

   while(1) {
       printf("正在 epollwait\n");
        int nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        printf("epollwait over");
        if(nfds == -1) {
            perror("epoll_wait");
            exit(1);
        }
        int f_cnt = 0;
        for(int i = 0; i < nfds; i++) {
            int now_fd = events[i].data.fd;
            printf("收到信号！ fd = %d \n", now_fd);
            if(now_fd == listen_socket) {
                clnt_socket = accept_clnt(listen_socket);
                printf("accept 成功, 加入链表。\n");
                struct sockaddr_in clnt_addr;
                socklen_t len = sizeof(clnt_addr);
                getpeername(clnt_socket, (struct sockaddr *)&clnt_addr, &len);
                List_add(all_clnt, clnt_addr.sin_addr.s_addr);
            }
        }
    }
    close(epollfd);

    
	return 0;	
}

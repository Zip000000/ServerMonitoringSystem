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
#include "Common.c"


#define PORT 8888
#define MAX_EVENTS 100

int do_heartbeat(clntnode *c) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    char *c_ip_str = get_ip_str(c);
    serv_addr.sin_addr.s_addr = inet_addr(c_ip_str);
    serv_addr.sin_port = htons(atoi(clntHPORT));
    //printf("clntHPORT = %s\n", clntHPORT);
    unsigned long ul = 1;
    ioctl(sock, FIONBIO, &ul);
    int con_ret = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    //perror("connect");
    int heartbeatflag = 0;

    if(con_ret < 0 && errno == EINPROGRESS) {
        fd_set wfds;
        FD_ZERO(&wfds);
        FD_SET(sock, &wfds);
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100000;
        int sel_ret = select(sock + 1, NULL, &wfds, NULL, &tv);
        if(sel_ret == 0) {
            printf("<%s>超时， 心跳失败，收尸！\n", c_ip_str);
        } else if(sel_ret >= 0) {
            int error = -1;
            int len = sizeof(error);
            if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len) < 0) {
                perror("getsockopt");
            }else if(error == 0) {
                //printf("getsockopt, error = %d\n", error);
                printf("<%s> getsockopt之后，心跳成功！！！\n", c_ip_str);
                heartbeatflag = 1;
            } else {
                //printf("getsockopt, error = %d\n", error);
                printf("<%s> getsockopt之后，心跳失败，收尸！\n", c_ip_str);
            }
        } else {
            perror("select");
        }
    } else {
        printf("<%s> 心跳异常，收尸！\n", c_ip_str);
    }
    close(sock);
    return heartbeatflag;

}

void *heartbeat (void *arg) {
    printf("5s之后开启心跳监测\n");
    sleep(5);
    int cnt = 0;
    while(1) {
        printf("子线程进行第%d次心跳遍历检测\n",cnt);

        clntnode *c = all_clnt->head;
        while(c->next) {
            c=c->next;
            int now_id = c->id;
            if(do_heartbeat(c)) continue;
            printf("删除该服务器\n");
            List_delete(all_clnt, now_id);
            //sleep(1);
        }
        //show_list(all_clnt);
        printf("第%d次心跳遍历检测, OVER\n",cnt);
        printf("--------------------------------\n");
        cnt++;
        sleep(3);
    }
}
int add_clnt(int listen_socket) {
    int clnt_socket;
    clnt_socket = accept_clnt(listen_socket);
    printf("accept 成功, 加入链表。\n");
    struct sockaddr_in clnt_addr;
    socklen_t len = sizeof(clnt_addr);
    getpeername(clnt_socket, (struct sockaddr *)&clnt_addr, &len);
    unsigned ip = clnt_addr.sin_addr.s_addr;
    if(!is_in_list(all_clnt, ip)) {
        List_add(all_clnt, clnt_addr.sin_addr.s_addr);
        return 1;
    } else {
        return 0;
    }
}

void add_all_clnt() {
    printf("start = %s end = %s \n", startIP, endIP);
    unsigned int startip = htonl(inet_addr(startIP));
    unsigned int endip = htonl(inet_addr(endIP));
    for(unsigned i = startip; i <= endip; i++) {
        struct in_addr in;
        in.s_addr = ntohl(i);
        if(i << 24 >> 24  == 0 || i << 24 >> 24 == 255) { printf("%s is passed\n", inet_ntoa(in)); continue; }
        all_clnt = List_add(all_clnt, ntohl(i));
    }
    printf("%s~%s 插入完毕\n", startIP, endIP);
    show_list(all_clnt);
}

int main() {
    do_master_config();
    all_clnt = Clnt_Info_list_init();
    add_all_clnt();
    pthread_t pthread_id;
	pthread_create(&pthread_id, NULL, heartbeat, NULL);

    int listen_socket = get_listen_socket(masterIP, atoi(masterPORT));
    if(listen_socket < 0) exit(1);
    int epollfd;
    struct epoll_event events[MAX_EVENTS];
    epollfd = epoll_create(1);
    add_event(epollfd, listen_socket, EPOLLIN);
    while(1) {
        printf("正在 epollwait\n");
        int nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        printf("nfds = %d\n", nfds);
        if(nfds == -1) {
            perror("epoll_wait");
        }
        if(add_clnt(listen_socket) == 1) {
            printf("加入成功\n");
        } else {
            printf("该客户端已存在\n");
        }
    }
    close(epollfd);
    
	return 0;	
}


        /*
        for(int i = 0; i < nfds; i++) {
            //int now_fd = events[i].data.fd;
            //printf("收到信号！ fd = %d \n", now_fd);
            //if(now_fd == listen_socket) {
            clnt_socket = accept_clnt(listen_socket);
            printf("accept 成功, 加入链表。\n");
            struct sockaddr_in clnt_addr;
            socklen_t len = sizeof(clnt_addr);
            getpeername(clnt_socket, (struct sockaddr *)&clnt_addr, &len);
            List_add(all_clnt, clnt_addr.sin_addr.s_addr);
            //}
        }
        */

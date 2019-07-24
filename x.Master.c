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


int Ins;

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

    if (con_ret < 0 && errno == EINPROGRESS) {
    //if (con_ret < 0) {
        fd_set wfds;
        FD_ZERO(&wfds);
        FD_SET(sock, &wfds);
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 300000;
        int sel_ret = select(sock + 1, NULL, &wfds, NULL, &tv);
        if (sel_ret == 0) {
            printf("<%s>超时， 心跳失败，收尸！\n", c_ip_str);
        } else if (sel_ret >= 0) {
            int error = -1;
            int len = sizeof(error);
            if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len) < 0) {
                perror("getsockopt");
            }else if (error == 0) {
                //printf("getsockopt, error = %d\n", error);
                printf("\033[32m <%s> getsockopt之后，心跳成功！！！\033[0m\n", c_ip_str);
                heartbeatflag = 1;
            } else {
                //printf("getsockopt, error = %d\n", error);
                printf("<%s> getsockopt之后，心跳失败，收尸！\n", c_ip_str);
            }
        } else {
            perror("select");
        }
    } else {
        printf("\033[31m <%s> 心跳异常，收尸！\033[0m\n", c_ip_str);
    }
    close(sock);
    return heartbeatflag;

}

void *heartbeat (void *arg) {
    PClntInfoList *clntlist = (PClntInfoList *)arg;
    printf("2s之后开启心跳监测\n");
    sleep(2);
    int cnt = 0;
    while(1) {
        printf("子线程进行第%d次心跳遍历检测\n", cnt);
        for (int i = 0; i < Ins; i++) {
            ClntInfoList *all_clnt = clntlist[i];
            
            clntnode *c = all_clnt->head;
            while(c->next) {
                //int now_id = c->next->id;
                if (!do_heartbeat(c->next)){
                    printf("[No. %d] 删除该服务器\n", all_clnt->my_id);
                    clntnode *d = c->next;
                    c->next = c->next->next;
                    free(d);
                    continue;
                    //List_delete(all_clnt, now_id);
                }
                c=c->next;
                //sleep(1);
            }    
        }
        //show_list(all_clnt);
        printf("第%d次心跳遍历检测, OVER\n",cnt);
        printf("--------------------------------\n");
        cnt++;
        usleep(500000);
    }
}
int add_clnt(int listen_socket, PClntInfoList *all_clnt) {
    int clnt_socket;
    clnt_socket = accept_clnt(listen_socket);
    printf("accept 成功, 准备加入链表。\n");
    struct sockaddr_in clnt_addr;
    socklen_t len = sizeof(clnt_addr);
    getpeername(clnt_socket, (struct sockaddr *)&clnt_addr, &len);
    unsigned ip = clnt_addr.sin_addr.s_addr;
    if (!is_in_list(all_clnt, ip, Ins)) {
        int min_list_id = get_min_list_id(all_clnt, Ins);
        List_add(all_clnt[min_list_id], clnt_addr.sin_addr.s_addr);
        return 1;
    } else {
        return 0;
    }
}

void add_all_clnt(PClntInfoList *all_clnt, int Ins) {
    printf("start = %s end = %s \n", startIP, endIP);
    unsigned int startip = htonl(inet_addr(startIP));
    unsigned int endip = htonl(inet_addr(endIP));
    for(unsigned i = startip; i <= endip; i++) {
        struct in_addr in;
        in.s_addr = ntohl(i);
        if (i << 24 >> 24  == 0 || i << 24 >> 24 == 255) { continue; }
        printf("%s\n", inet_ntoa(in));
        int min_list_id = get_min_list_id(all_clnt, Ins);
        all_clnt[min_list_id] = List_add(all_clnt[min_list_id], ntohl(i));
    }
    printf("%s~%s 插入完毕\n", startIP, endIP);
    for (int i = 0; i < Ins; i++) {
        printf("List %d : \n", i);
        show_list(all_clnt[i]);
    }
}

void *do_work (void *arg) {

    int epollfd;
    struct epoll_event events[atoi(MAX_WORK_EVENTS) / Ins + 10];
    epollfd = epoll_create(1);
    
    ClntInfoList *all_clnt = (ClntInfoList *)arg;
    clntnode *c = all_clnt->head;
    int my_id = all_clnt->my_id;
    int cnt = 0;
    while(1) {
        cnt++;
        while(c->next != NULL) {
            int sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock < 0) { perror("socket in do_work"); break;}

            struct sockaddr_in serv_addr;
            memset(&serv_addr, 0, sizeof(serv_addr));
            serv_addr.sin_family = AF_INET;
            char *cnext_ip_str = get_ip_str(c->next);
            serv_addr.sin_addr.s_addr = inet_addr(cnext_ip_str);
            serv_addr.sin_port = htons(atoi(clntPORT));
            unsigned long ul = 1;
            ioctl(sock, FIONBIO, &ul);
            int con_ret = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
            add_event(epollfd, sock, EPOLLOUT);
        }
        printf("链表【%d】 全部注册完毕! from [FUNC: do_work]\n", my_id);
        printf("开始进行第[%d]次收发:\n",cnt);
    }
   //写到这了！ 
    
    
    while(1) {
        //printf("正在 epollwait\n");
        printf("------------------------------\n");
        int nfds = epoll_wait(epollfd, events, atoi(MAX_WORK_EVENTS), 10000);
        printf("nfds = %d\n", nfds);
        if (nfds == -1) { 
            perror("epoll_wait");
        } else if(nfds == 0) {
            printf("epoll wait Master心跳 超时\n");
            printf("断线超时, 准备重连。\n");
            if(kill(pid, 10) == -1) {
                perror("kill");
            }
        } else {
            int master_socket = accept_clnt(listen_socket);
            if (master_socket != -1) {
                printf("收到心跳\n");
            } else {
                printf("心跳失败\n");
            }
            close(master_socket);
            printf("已经关闭mastersocket\n");
        }
    }
    close(epollfd);

    

    
    
}

int main() {
    do_master_config();
    Ins = atoi(INS);
    printf("Ins = %d\n", Ins);
    PClntInfoList *clnt_list;
    clnt_list=all_init(Ins);
    
    add_all_clnt(clnt_list, Ins);
    pthread_t pthread_id[Ins + 1];
	pthread_create(&pthread_id[Ins], NULL, heartbeat, clnt_list);
    for(int i = 0; i < Ins; i++) {
	    pthread_create(&pthread_id[i], NULL, do_work, clnt_list[i]);
    }
    int listen_socket = get_listen_socket(masterIP, atoi(masterPORT));
    if (listen_socket < 0) exit(1);
    int epollfd;
    struct epoll_event events[atoi(MAX_EVENTS)];
    epollfd = epoll_create(1);
    add_event(epollfd, listen_socket, EPOLLIN);
    while(1) {
        printf("正在 epollwait\n");
        int nfds = epoll_wait(epollfd, events, atoi(MAX_EVENTS), -1);
        printf("nfds = %d\n", nfds);
        if (nfds == -1) {
            perror("epoll_wait");
        }
        if (add_clnt(listen_socket, clnt_list) == 1) {
            printf("加入成功\n");
        } else {
            printf("该客户端已存在\n");
        }
    }
    close(epollfd);
	return 0;	
}


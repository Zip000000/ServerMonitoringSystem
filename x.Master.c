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
pthread_mutex_t mutex;

int do_heartbeat(clntnode *c) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    char *c_ip_str = get_ip_str(c);
    serv_addr.sin_addr.s_addr = inet_addr(c_ip_str);
    serv_addr.sin_port = htons(atoi(clntHPORT));
    unsigned long ul = 1;
    ioctl(sock, FIONBIO, &ul);
    int con_ret = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
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
    sleep(5);
    long long cnt = 0;
    while(1) {
        printf("子线程进行第%lld次心跳遍历检测\n", cnt);
        for (int i = 0; i < Ins; i++) {
            ClntInfoList *all_clnt = clntlist[i];
            
            clntnode *c = all_clnt->head;
            while(c->next) {
                if (!do_heartbeat(c->next)){
                    //printf("[No. %d] 删除该服务器\n", all_clnt->my_id);
                    pthread_mutex_lock(&mutex);
                    clntnode *d = c->next;
                    c->next = c->next->next;
                    clntlist[i]->clnt_num--;
                    free(d);
                    pthread_mutex_unlock(&mutex);
                    continue;
                    //List_delete(all_clnt, now_id);
                }
                c = c->next;
            }    
        }
        //show_list(all_clnt);
        printf("--------------------------------\n");
        cnt++;
        usleep(atoi(HeartbeatTimeout));
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
        pthread_mutex_lock(&mutex);
        int min_list_id = get_min_list_id(all_clnt, Ins);
        List_add(all_clnt[min_list_id], clnt_addr.sin_addr.s_addr);
        pthread_mutex_unlock(&mutex);
        close(clnt_socket);
        return 1;
    } else {
        close(clnt_socket);
        return 0;
    }
}

void add_all_clnt(PClntInfoList *all_clnt, int Ins) {
    printf("start = %s end = %s \n", startIP, endIP);
    unsigned int startip = htonl(inet_addr(startIP));
    unsigned int endip = htonl(inet_addr(endIP));
    pthread_mutex_lock(&mutex);
    for(unsigned i = startip; i <= endip; i++) {
        struct in_addr in;
        in.s_addr = ntohl(i);
        if (i << 24 >> 24  == 0 || i << 24 >> 24 == 255) { continue; }
        printf("%s\n", inet_ntoa(in));
        int min_list_id = get_min_list_id(all_clnt, Ins);
        all_clnt[min_list_id] = List_add(all_clnt[min_list_id], ntohl(i));
    }
    pthread_mutex_unlock(&mutex);
    printf("%s~%s 插入完毕\n", startIP, endIP);
    for (int i = 0; i < Ins; i++) {
        printf("List %d : \n", i);
        show_list(all_clnt[i]);
    }
}

int do_save_log_file(int now_fd) {
    char buff[1024];
    memset(buff, 0, sizeof(buff));
    int recv_ret = recv(now_fd, buff, sizeof(buff), 0);
    if (recv_ret < 0) { perror("recv"); return -1;}
    if (recv_ret == 0) { printf("recv_ret == 0\n"); return -1;}
    char filename[1000] = "./Master_LogInfo/LogTest";
    FILE *fp = fopen(filename, "a+");
    if (fp == NULL) {perror("fopen"); return -1;}
    int fwrite_ret = fwrite(buff, 1, strlen(buff), fp);
    if (fwrite_ret > 0) {
        printf("Log: 写入[%d]字节成功\n", fwrite_ret);
    } else {
        perror("fwrite");
    }
    fclose(fp);
    return 0;
}

void *do_work (void *arg) {
    struct epoll_event events[atoi(MAX_WORK_EVENTS) / Ins + 10];
    
    ClntInfoList *all_clnt = (ClntInfoList *)arg;
    long long cnt = 0;
    int epollfd = epoll_create(1);
    while(1) {
        cnt++;
        pthread_mutex_lock(&mutex);
        ClntInfoList *tmp_list = copy_List(all_clnt);
        int clnt_num = all_clnt->clnt_num;
        int my_id = all_clnt->my_id;
        pthread_mutex_unlock(&mutex);

        printf("[send & recv] 开始进行第[%lld]次收发: 链表[%d]共%d 个客户.\n", cnt, my_id, clnt_num);
        if(clnt_num == 0) {
            printf("[send & recv]当前没有用户，休息1s\n");
            sleep(1);
            continue;
        }
        clntnode *c = tmp_list->head;
        while(1) {
            char cnext_ip_str[100];
            memset(cnext_ip_str, 0, sizeof(cnext_ip_str));
            if(c->next != NULL) {
                printf("id = %d, ip = %s\n", c->next->id, get_ip_str(c->next));
                strcpy(cnext_ip_str, get_ip_str(c->next));
            } else {
                printf("c->next == NULL  break\n");
                break;
            }
            
            int sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock < 0) { perror("socket in do_work"); break;}
            struct sockaddr_in serv_addr;
            make_sockaddr_in(&serv_addr, cnext_ip_str, clntPORT);
            unsigned long ul = 1;
            ioctl(sock, FIONBIO, &ul);
            int con_ret = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
            add_event(epollfd, sock, EPOLLOUT);
            while(1) {
                int nfds = epoll_wait(epollfd, events, atoi(MAX_WORK_EVENTS), 1000);
                if(nfds == -1) {
                    perror("epoll_wait in do_work");
                } else if(nfds == 0) {
                    printf("[send & recv] <%s>%d ：epoll wait do_work 超时,跳过该客户。\n",cnext_ip_str, sock);
                } else {
                    if (events[0].events & EPOLLOUT) {
                        char sendstr[] = "hello--from_master";
                        int send_ret = send(sock, sendstr, sizeof(sendstr), 0);
                        if (send_ret <= 0) {
                            if (send_ret < 0)perror("send"); else { printf("send_ret==0\n"); } 
                            delete_event(epollfd, sock, 0);
                            close(sock);
                            sleep(1);
                            break;
                        }
                        printf("[send & recv] <%s>%d ： send success！\n",cnext_ip_str, sock);
                        modify_event(epollfd, sock, EPOLLIN);
                        continue;
                    } else if(events[0].events & EPOLLIN) {
                        printf("准备recv！！！！\n");
                        if (do_save_log_file(sock) < 0) {
                            delete_event(epollfd, sock, 0);
                            close(sock);
                            sleep(1);
                            break;
                        }
                        printf("[send & recv] <%s>%d ： save success！\n",cnext_ip_str, sock);
                    }
                }
                delete_event(epollfd, sock, 0);
                close(sock);
                printf("[send & recv] <%s>%d ： clear success！\n",cnext_ip_str, sock);
                sleep(1);  //1s收发一次
                break;
            }
            c = c->next;
        }
    clear_List(tmp_list);
    //这里不注释吧？？？？？？
    }
}
int main() {
    pthread_mutex_init(&mutex, NULL);
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
    if (listen_socket < 0) { perror("get_listen_socket in main"); exit(1); } 
    struct epoll_event events[atoi(MAX_EVENTS)];
    int epollfd = epoll_create(1);
    add_event(epollfd, listen_socket, EPOLLIN);
    while(1) {
        int nfds = epoll_wait(epollfd, events, atoi(MAX_EVENTS), -1);
        printf("nfds = %d\n", nfds);
        if (nfds == -1) {
            perror("epoll_wait");
        }
        if (add_clnt(listen_socket, clnt_list) == 1) {
            printf("[main] 新客户端加入成功\n");
        } else {
            printf("[main] 该客户端已存在\n");
        }
    }

    close(epollfd);
    for(int i = 0; i <= Ins; i++) {
        pthread_join(pthread_id[i], NULL);
    }
    pthread_mutex_destroy(&mutex);
	return 0;	
}

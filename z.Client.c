/*************************************************************************
	> File Name: c.Client.c
	> Author: Zip 
	> Mail: 307110017@qq.com 
	> Created Time: 2019年07月20日 星期六 14时26分40秒
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
#include <sys/epoll.h>
#include <signal.h>
#include "Sock.c"
#include "Common.c"
#include "Epoll.c"

#define MAX_EVENTS 10

int  accept_wait(int listen_socket) {
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    printf("正在accept\n");
    unsigned long ul = 1;
     
    int clnt_socket = accept(listen_socket, (struct sockaddr *)&client_addr, &len);
    printf("accept over\n");
    if(clnt_socket < 0) {
        perror("accept");
        return -1;
    }
    getpeername(clnt_socket, (struct sockaddr *)&client_addr, &len);
    printf("<%s> : Login    fd = %d \n",inet_ntoa(client_addr.sin_addr), clnt_socket);
    return clnt_socket;
}



void grandson_lazy_connect(int tmp) {
    printf("in grandson_lazy_connect()\n");
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {perror("socket");exit(1);}
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(masterIP);
    serv_addr.sin_port = htons(atoi(masterPORT));
    int flag = 0;
    for (int i = 1;; i++) {
        printf("[son] 第%d次尝试连接Master\n", i);
        int ret = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        if(ret == -1) {
            printf("[son] 失败, 1s后进行下次尝试。。。。\n");
            sleep(1);
        } else if(ret == 0) {
            printf("[son] 链接成功\n");
            flag = 1;
            break;
        }
    }
    return;
} 

int main() {
    do_clnt_config();
    printf("master IP = %s Master PORT =  %s\n", masterIP, masterPORT);
    printf("clnt IP = %s clnt PORT =  %s\n", clntIP, clntHPORT);
    printf("主进程：永远等待master发送心跳。\n");
    printf("子进程：若孙子线程十次失败，子线程父承子业。成功则kill孙子进程。\n");
    printf("孙子进程：主动链接master上限十次。之后无论成功失败都执行client本职任务。\n");
    

	pid_t pid;
	int my_id = 0;
    pid = fork();
    if (pid == 0) {
 		my_id++;
        pid = fork();
	}
    if (pid == 0) my_id++;
    
    if (my_id == 0) {
        int listen_socket = get_listen_socket(clntIP, atoi(clntHPORT));
        if(listen_socket < 0) {perror("getlistensock"); exit(1);}
        while(1) {
            printf("我是老大\n");
            
			int epollfd;
			struct epoll_event events[MAX_EVENTS];
			epollfd = epoll_create(1);
			add_event(epollfd, listen_socket, EPOLLIN);
			while(1) {
				//printf("正在 epollwait\n");
                printf("------------------------------\n");
				int nfds = epoll_wait(epollfd, events, MAX_EVENTS, 10000);
				printf("nfds = %d\n", nfds);
				if (nfds == -1) { 
                    perror("epoll_wait");
                } else if(nfds == 0) {
                    printf("epoll wait Master心跳 超时\n");
                    printf("断线超时, 启动重连功能。\n");
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

            /*
            char tmp[20];
            int recv_ret = 0;
            while((recv_ret = recv(listen_socket, tmp, sizeof(tmp), 0)) <= 0) {
                printf("recv_ret = %d\n", recv_ret);
            }
            printf("对方关闭链接\n");
            */
        }
    }
    if(my_id == 1) {
        printf("我是子进程, 在等待孙子进程告知结果\n");
        signal(10, grandson_lazy_connect);
        while(1) pause();
    }
    
    if (my_id == 2) {
        printf("我是孙子进程\n");
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1) {perror("socket");exit(1);}
        struct sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr(masterIP);
        serv_addr.sin_port = htons(atoi(masterPORT));
        int flag = 0;
        for (int i = 1; i <= 10; i++) {
            printf("第%d次尝试连接Master\n", i);
            int ret = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
            if(ret == -1) {
                printf("失败, 1s后进行下次尝试。。。。\n");
                sleep(1);
            } else if(ret == 0) {
                printf("链接成功\n");
                flag = 1;
                break;
            }
        }
        if (flag == 1) {
            printf("***孙子进程进行本职任务***\n");
        } else {
            printf("让子进程继续尝试链接master\n");
            close(sock);
            if(kill(getppid(), 10) == -1) {
                perror("kill");
            }
        }

        printf("***孙子进程正在工作。。。。。。。***\n");
        int listen_socket = -1;
        listen_socket = get_listen_socket(clntIP, atoi(clntPORT));
        if(listen_socket < 0) {perror("[work] getlistensock");}

        while(1) {
			int epollfd;
			struct epoll_event events[MAX_EVENTS];
			epollfd = epoll_create(1);
			add_event(epollfd, listen_socket, EPOLLIN);
			while(1) {
				//printf("正在 epollwait\n");
                printf("------------------------------\n");
				int nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
				printf("nfds = %d\n", nfds);
				if (nfds == -1) { 
                    perror("epoll_wait in work");
                } else if (nfds == 0) {
                    printf("WTF!in working epollwait nfds == 0\n");
                } else {
                    for (int i = 0; i < nfds; i++) {
                        int now_fd = events[i].data.fd;
                        if (now_fd == listen_socket) {
                            int master_socket = accept_clnt(listen_socket);
                            if (master_socket == -1) {
                                printf("[work] 数据请求 连接失败\n");
                                break;
                            } else {
                                printf("[work] 数据请求链接成功\n");
                                add_event(epollfd, master_socket, EPOLLIN);
                            }
                        } else if (events[i].events & EPOLLIN) {
                            printf("准备接受数据\n");
                            char buff[1000];
                            memset(buff, 0, sizeof(buff));
                            int recv_ret = recv(now_fd, buff, sizeof(buff), 0);
                            if(recv_ret < 0) {perror("recv");}
                            else printf("**************recvstr : %s \n", buff);
                            modify_event(epollfd, now_fd, EPOLLOUT);
                        } else if(events[i].events & EPOLLOUT) {
                            printf("准备发送数据\n");
                            char send_str[1000];
                            memset(send_str, 0, sizeof(send_str));
                            sprintf(send_str, "HAHAHA,SEND_SUCCESS\n");
                            int send_ret = send(now_fd, send_str, sizeof(send_str), 0);
                            if (send_ret < 0) { perror("send"); }
                            else {
                                printf("send success!!\n");
                                delete_event(epollfd, now_fd, 0);
                                close(now_fd);
                                printf("now_fd closed!\n");

                            }
                        } else {
                            printf("不应该存在的情况\n");

                        }

                    }
                }
			}
			close(epollfd);
        }
    }
    
    
    
    return 0;
}



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

#include "Sock.c"
#include "Common.c"

#define MAX_EVENTS 10

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("192.168.2.90");
    serv_addr.sin_port = htons(8888);
	pid_t pid;

	int my_id = 0;
    pid = fork();
    if (pid == 0) {
 		my_id++;
        pid = fork();
	}
    if (pid == 0) my_id++;

    if (my_id == 0) {
        printf("我是老大\n");

		char masterIP[100];
		char masterPORT[100];
		memset(masterIP, 0, sizeof(masterIP));
		memset(masterPORT, 0, sizeof(masterPORT));
		get_conf_value("config", "MasterIP", masterIP);
		get_conf_value("config", "MasterPORT", masterPORT);

		char clntIP[100];
		char clntPORT[100];
		memset(clntIP, 0, sizeof(clntIP));
		memset(clntPORT, 0, sizeof(clntPORT));
		get_conf_value("config", "ClntHIP", clntIP);
		get_conf_value("config", "ClntHPORT", clntPORT);

        int listen_socket = get_listen_socket(clntIP, atoi(clntPORT));
		accept_clnt(listen_socket);
    }
    
    if (my_id == 1) {
        printf("我是子进程\n");
        for(int i = 1; i <= 10; i++) {
            printf("第%d次尝试连接Master\n", i);
            int ret = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
            if(ret == -1) {
                printf("失败\n");
                sleep(1);
            } else if(ret == 0) {
                printf("链接成功\n");
                break;
            }
        }
        printf("子进程 尝试链接10次失败\n");
    }
    
    if(my_id == 2) {
        printf("我是孙子进程\n");
        
    }
    
    
    return 0;
}



/*
            int ret = epoll_wait(epollfd, events, MAX_EVENTS, 1000000);
            if(ret == 0) continue;
            if(ret < 0) {perror("epoll_wait");}
            if(ret > 0) printf(" ret = %d epoll 收到链接\n", ret);
 */


        /*
        int epollfd;
        struct epoll_event events[MAX_EVENTS];
        epollfd = epoll_create(1);
        add_event(epollfd, sock, EPOLLIN);
        */

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

#define MAX_EVENTS 10

char clntIP[100];
char clntHPORT[100];
char masterIP[100];
char masterPORT[100];

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
    sleep(50);
} 

int main() {
    do_clnt_config();
    printf("master IP = %s Master PORT =  %s\n", masterIP, masterPORT);
    printf("clnt IP = %s clnt PORT =  %s\n", clntIP, clntHPORT);
    /*
    int sock = get_socket(masterIP, atoi(masterPORT));
    if(sock == -1) exit(1);
    */
    printf("主进程：永远等待master发送心跳。\n");
    printf("子进程：若孙子线程十次失败，子线程父承子业。成功则kill孙子进程。\n");
    printf("孙子进程：主动链接master上限十次。之后无论成功失败都执行client本职任务。\n");
    
    int listen_socket = get_listen_socket(clntIP, atoi(clntHPORT));
    if(listen_socket < 0) {perror("getlistensock"); exit(1);}

	pid_t pid;
	int my_id = 0;
    pid = fork();
    if (pid == 0) {
 		my_id++;
        pid = fork();
	}
    if (pid == 0) my_id++;
    
    if (my_id == 0) {
        while(1) {
            printf("我是老大\n");
            int master_socket = accept_clnt(listen_socket);
            printf("收到心跳\n");
            printf("------------------------------\n");
            /*
            char tmp[20];
            int recv_ret = 0;
            while((recv_ret = recv(listen_socket, tmp, sizeof(tmp), 0)) <= 0) {
                printf("recv_ret = %d\n", recv_ret);
            }
            printf("对方关闭链接\n");
            */
            close(master_socket);
            printf("已经关闭mastersocket\n");
        }
    }
    if(my_id == 1) {
        printf("我是子进程, 在等待孙子进程告知结果\n");
        signal(10, grandson_lazy_connect);
        pause();
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
        sleep(100);
    }
    
    
    
    return 0;
}



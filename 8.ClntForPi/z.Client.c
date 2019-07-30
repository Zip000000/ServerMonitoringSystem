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

#include <sys/ioctl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <signal.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "Common.c"
#include "Sock.c"
#include "Epoll.c"

#define MAX_DATA 1024
#define MAX_EVENTS 10
#define ReconnTimes 10

struct sm_msg {
    int heartbeat_flag;
    int makeinfo_times;
    double para_for_Mem;
    int Reconn_pid;
} *msg;

int select_conn(int sock) {
    int flag = -1;
    fd_set wfds; FD_ZERO(&wfds); FD_SET(sock, &wfds);
    struct timeval tv; tv.tv_sec = 0; tv.tv_usec = 1000000;
    int sel_ret = select(sock + 1, NULL, &wfds, NULL, &tv);
    if(sel_ret == 0) {
        flag = -1;
    } else if (sel_ret > 0) {
        int error = -1;
        int len = sizeof(error);
        if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len) < 0) {
            perror("getsockopt");
        } else if (error == 0) {
            flag = 1;
        } else {
            flag = -1;
        }
    }
    return flag;
}
void grandson_lazy_connect(int n) {
    printf("断线重连功能开启！\n");
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {perror("socket in lazy_conn");  return;}
    struct sockaddr_in serv_addr;
    make_sockaddr_in(&serv_addr, masterIP, masterPORT);
    unsigned long ul = 1;
    ioctl(sock, FIONBIO, &ul);
    for (int i = 1;; i++) {
        printf("[子][断线重连] 第%d次尝试连接Master\n", i);
        int ret = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        if (ret < 0 && errno == EINPROGRESS) {
            int flag = select_conn(sock);
            if (flag == -1) {
                printf("[子][断线重连] 失败, 1s后进行下次尝试。。。。\n");  
            } else {
                printf("[子][断线重连] 链接成功\n");
                close(sock);   
                break;
            }
        } 
        sleep(1);
    }
    return;
} 
typedef struct singleData {
    char buf[1024];
} singleData;
typedef struct Data {
    char cmd[256];
    char savePath[256];
    singleData sd[5];
    int cnt;
} Data;

void make_single_log(struct Data *data) {
    //printf("start make single log cnt = %d\n", data->cnt);
    FILE *pp = popen(data->cmd, "r");
    if(pp == NULL) { perror("popen"); return ;}
    char *dt_buf = data->sd[data->cnt].buf;
    fread(dt_buf, 1, MAX_DATA, pp);
    pclose(pp);
    //printf("cnt = %d : %s\n", data->cnt, data->sd[data->cnt].buf);
    data->cnt++;
    if(data->cnt < 5) return;
    printf("累计五次信息，准备写入\n");
    FILE *p_tmp = fopen(data->savePath, "a+");
    if(p_tmp == NULL) { perror("fopen");return; }
    for (int i = 0; i < 5; i++) {
        //printf("i = %d, strlen = %ld\n", i, strlen(data->sd[i].buf));
        fwrite(data->sd[i].buf, 1, strlen(data->sd[i].buf), p_tmp);
    }
    memset(data->sd, 0, 5 * sizeof(singleData));
    data->cnt = 0;
    fclose(p_tmp);
}
void *do_make_log_info(void *arg) {
    Data Cpu, Mem, User, Disk, Dete, Stat;
    memset(&Cpu, 0, sizeof(Cpu));
    memset(&Mem, 0, sizeof(Cpu));
    memset(&User, 0, sizeof(Cpu));
    memset(&Disk, 0, sizeof(Cpu));
    memset(&Dete, 0, sizeof(Cpu));
    memset(&Stat, 0, sizeof(Cpu));
    strcpy(Cpu.cmd, "bash ./1.ShellStuff/1.CpuLog.sh");
    strcpy(Mem.cmd, "bash ./1.ShellStuff/2.MemLog.sh");
    strcpy(User.cmd, "bash ./1.ShellStuff/3.User.sh");
    strcpy(Disk.cmd, "bash ./1.ShellStuff/4.Disk.sh");
    strcpy(Dete.cmd, "bash ./1.ShellStuff/5.Detection.sh");
    strcpy(Stat.cmd, "bash ./1.ShellStuff/6.State.sh");
    strcpy(Cpu.savePath,"./Clnt_LogInfo/Log_Cpu");
    strcpy(Mem.savePath, "./Clnt_LogInfo/Log_Mem");
    strcpy(User.savePath, "./Clnt_LogInfo/Log_User");
    strcpy(Disk.savePath, "./Clnt_LogInfo/Log_Disk");
    strcpy(Dete.savePath, "./Clnt_LogInfo/Log_Dete");
    strcpy(Stat.savePath, "./Clnt_LogInfo/Log_Stat");
    Cpu.cnt = Mem.cnt = User.cnt = Disk.cnt = Dete.cnt = Stat.cnt = 0;

    while (1) {
        printf("1\n");
        make_single_log(&Cpu);
        printf("2\n");
        make_single_log(&Mem);
        printf("3\n");
        make_single_log(&User);
        printf("4\n");
        make_single_log(&Disk);
        printf("5\n");
        make_single_log(&Dete);
        printf("6\n");
        make_single_log(&Stat);
        //printf("运行了 %d 次脚本\n", msg->makeinfo_times);
        if (msg->heartbeat_flag != 0) {
            msg->makeinfo_times++;
            if (msg->makeinfo_times >= ReconnTimes) {
                printf("断线超过 自检十次的时间！Reconnect!\n");
                if(kill(msg->Reconn_pid, 10) == -1) { perror("kill");}
                msg->makeinfo_times = 0;
            }
        }
        sleep(5);
        /*
        if (cnt == 6) {
            make_single_log(Dete, Log_Dete);
        } else if (cnt == 12) {
            make_single_log(Stat, Log_Stat);
            make_single_log(User, Log_User);
            make_single_log(Disk, Log_Disk);
            cnt = 0;
        }
        make_single_log(Cpu, Log_Cpu);
        make_single_log(Mem, Log_Mem);
        cnt++;
        sleep(5);
    */
    }
}
int do_send(int now_fd, int epollfd) {
    //do_make_log_info();
    char send_str[1024];
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
}

int do_recv(int now_fd, int epollfd) {
    char buff[1024];
    memset(buff, 0, sizeof(buff));
    int recv_ret = recv(now_fd, buff, sizeof(buff), 0);
    if(recv_ret < 0) {perror("recv");}
    else printf("**************recvstr : %s \n", buff);
    modify_event(epollfd, now_fd, EPOLLOUT);
}

void do_events(int epollfd, int nfds, int listen_socket, struct epoll_event *events) {
    for (int i = 0; i < nfds; i++) {
        int now_fd = events[i].data.fd;
        if (now_fd == listen_socket) {
            int master_socket = accept_clnt(listen_socket);
            if (master_socket == -1) {
                printf("[work] 数据请求 连接失败\n");
            } else {
                printf("[work] 数据请求链接成功\n");
                add_event(epollfd, master_socket, EPOLLIN);
            }
        } else if (events[i].events & EPOLLIN) {
            printf("准备接受数据\n");
            do_recv(now_fd, epollfd);
        } else if(events[i].events & EPOLLOUT) {
            printf("准备发送数据\n");
            do_send(now_fd, epollfd);
        } else {
            printf("不应该存在的情况\n");
        }
    }
}

void first_try_connect() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {perror("socket");exit(1);}
    struct sockaddr_in serv_addr;
    make_sockaddr_in(&serv_addr, masterIP, masterPORT);
    int flag = -1;
    unsigned long ul = 1;
    ioctl(sock, FIONBIO, &ul);
    for (int i = 1; i <= 10; i++) {
        printf("第%d次尝试连接Master\n", i);
        int ret = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        if (ret < 0 && errno == EINPROGRESS) {
            flag = select_conn(sock);
            if (flag == -1) {
                printf("[孙子][初试连接] 失败, 1s后进行下次尝试。。。。\n");  
                sleep(1);
            } else {
                printf("[孙子][初试连接] 链接成功 flag = %d\n", flag);
                close(sock);   
                break;
            }
        }
    }
    if (flag != 1) {
        //printf("让子进程继续尝试链接master flag = %d\n", flag);
        //if(kill(getppid(), 10) == -1) { perror("kill"); }
    }
    return ;
}

int heartbeat_recv(int son_pid) {
    int listen_socket = get_listen_socket(clntIP, atoi(clntHPORT));
    if(listen_socket < 0) {perror("getlistensock"); return -1;}
    struct epoll_event events[MAX_EVENTS];
    int epollfd = epoll_create(1);
    add_event(epollfd, listen_socket, EPOLLIN);
    while(1) {
        printf("------------------------------\n");
        int nfds = epoll_wait(epollfd, events, MAX_EVENTS, 1000);
        if (nfds == -1) { 
            perror("epoll_wait");
        } else if (nfds == 0) {
            msg->heartbeat_flag = 1;
            /*
            if (kill(son_pid, 10) == -1) {
                perror("kill");
            }
            */
        } else {
            int master_socket = accept_clnt(listen_socket);
            if (master_socket > 0) {
                printf("收到心跳\n");
                close(master_socket);
                msg->heartbeat_flag = 0;
            } else {
                printf("心跳失败\n");
                msg->heartbeat_flag = 1;
            }
        }
    }
    close(epollfd);
    return 0;
}

void do_data_recv_send() {
        int listen_socket = get_listen_socket(clntIP, atoi(clntPORT));
        if(listen_socket < 0) { perror("[work] getlistensock"); }
        int epollfd;
        struct epoll_event events[MAX_EVENTS];
        epollfd = epoll_create(1);
        add_event(epollfd, listen_socket, EPOLLIN);
        while(1) {
            printf("------------------------------\n");
            int nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
            if (nfds == -1) { 
                perror("epoll_wait in work");
            } else if (nfds == 0) {
                printf("WTF!in working epollwait nfds == 0\n");
            } else {
                do_events(epollfd, nfds, listen_socket, events);
            }
        }
        close(epollfd);
}
    
int main() {
    do_clnt_config();
    printf("master IP = %s Master PORT =  %s\n", masterIP, masterPORT);
    printf("clnt IP = %s clnt PORT =  %s\n", clntIP, clntHPORT);
    printf("主进程：永远等待master发送心跳。\n");
    printf("子进程：若孙子线程十次失败，子线程父承子业。成功则kill孙子进程。\n");
    printf("孙子进程：主动链接master上限十次。之后无论成功失败都执行client本职任务。\n");
    
    int shmid = shmget(IPC_PRIVATE, sizeof(struct sm_msg), IPC_CREAT | 0666);
    if (shmid < 0) { perror("shmget"); exit(1); }
    msg = shmat(shmid, NULL, 0);
    msg->heartbeat_flag = 0;
    msg->makeinfo_times = 0;
    msg->para_for_Mem = 0;

	pid_t pid;
	int my_id = 0;
    pid = fork();
    if (pid == 0) {
 		my_id++;
        pid = fork();
	}
    if (pid == 0) my_id++;
    
    if (my_id == 0) {
        msg->Reconn_pid = pid;
        printf("[主] pid = %d ：监听心跳信号\n", getpid());
        pthread_t pthread_id;
        pthread_create(&pthread_id, NULL, do_make_log_info, NULL);
        heartbeat_recv(pid);
    } else if (my_id == 1) {
        printf("[子] pid = %d ：我是子进程, 在等待接受信号，断线重连时刻就绪\n", getpid());
        signal(10, grandson_lazy_connect);
        while(1) pause();
    } else if (my_id == 2) {
        printf("[孙] pid = %d ：主动连接master\n", getpid());
        first_try_connect();
        printf("***孙子进程开始进行数据收发工作。。。。。。。***\n");
        do_data_recv_send();         
    }
    return 0;
}

        /*
        int ret = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        if(ret == -1) {
            printf("[子][断线重连] 失败, 1s后进行下次尝试。。。。\n");
            sleep(1);
        } else if(ret == 0) {
            printf("[子][断线重连] 链接成功\n");
            close(sock);   
            break;
        }
        */




/*
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
*/


        /*
        int ret = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        if (ret < 0 && errno == EINPROGRESS) {
            int flag = select_conn(sock);
            if (flag == -1) {
                printf("[子][断线重连] 失败, 1s后进行下次尝试。。。。\n");  
            } else {
                printf("[子][断线重连] 链接成功\n");
                close(sock);   
                break;
            }
        } 
        sleep(1);
        */

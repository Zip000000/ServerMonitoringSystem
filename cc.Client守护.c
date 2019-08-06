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
#include <sys/file.h>
#include "Common.c"
#include "Sock.c"
#include "Epoll.c"

#define MAX_DATA 1024
#define MAX_EVENTS 10
//#define __DEBUG__
#ifdef __DEBUG__
#define DBG(format, ...) printf (format, ##__VA_ARGS__)
#else
#define DBG(format, ...)
#endif

struct sm_msg {
    int heartbeat_flag;
    int makeinfo_times;
    double para_for_Mem;
    int Reconn_pid;
} *msg;

typedef struct singleData {
    char buf[1024];
} singleData;
typedef struct Data {
    int my_id;
    char cmd[256];
    char savePath[256];
    singleData sd[5];
    int cnt;
} Data;

typedef struct LogInfo {
    int flag;
    int buflen;
    char buf[1025];
} LogInfo;

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
    DBG("断线重连功能开启！\n");
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {perror("socket in lazy_conn");  return;}
    struct sockaddr_in serv_addr;
    make_sockaddr_in(&serv_addr, masterIP, masterPORT);
    unsigned long ul = 1;
    ioctl(sock, FIONBIO, &ul);
    for (int i = 1;; i++) {
        DBG("[子][断线重连] 第%d次尝试连接Master\n", i);
        int ret = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        if (ret < 0 && errno == EINPROGRESS) {
            int flag = select_conn(sock);
            if (flag == -1) {
                DBG("[子][断线重连] 失败, 1s后进行下次尝试。。。。\n");  
            } else {
                DBG("[子][断线重连] 链接成功\n");
                close(sock);   
                break;
            }
        } 
        sleep(1);
    }
    return;
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
        DBG("第%d次尝试连接Master\n", i);
        int ret = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        if (ret < 0 && errno == EINPROGRESS) {
            flag = select_conn(sock);
            if (flag == -1) {
                DBG("[孙子][初试连接] 失败[1], 1s后进行下次尝试。。。。\n");  
                perror("connect 1");
                sleep(2);
            } else {
                DBG("[孙子][初试连接] 链接成功 flag = %d\n", flag);
                close(sock);   
                break;
            }
        } else {
            DBG("[孙子][初试连接] 失败[2], 1s后进行下次尝试。。。。\n");;  
            perror("connect 2");
            sleep(2);
        }
    }
    if (flag != 1) {
        DBG("让子进程继续尝试链接master flag = %d\n", flag);
        if(kill(getppid(), 10) == -1) { perror("kill"); }
    }
    return ;
}

int heartbeat_recv() {
    int listen_socket = get_listen_socket(clntIP, atoi(clntHPORT));
    if(listen_socket < 0) {perror("getlistensock"); return -1;}
    struct epoll_event events[MAX_EVENTS];
    int epollfd = epoll_create(1);
    add_event(epollfd, listen_socket, EPOLLIN);
    while(1) {
        DBG("------------------------------\n");
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
                DBG("收到心跳\n");
                close(master_socket);
                msg->heartbeat_flag = 0;
                msg->makeinfo_times=0;
            } else {
                DBG("心跳失败\n");
                msg->heartbeat_flag = 1;
            }
        }
    }
    close(epollfd);
    return 0;
}
int warnning_master() {
    struct sockaddr_in serv_warn_addr;
    
    make_sockaddr_in(&serv_warn_addr, masterIP, masterWPORT);

    int udpsock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpsock < 0) {
        perror("socket in warnning");
        return -1;
    }
    
    for (int i = 0; i < 10; i++) {
        int warn = 1;
        int sendto_ret = sendto(udpsock, &warn, sizeof(int), 0, (struct sockaddr *)&serv_warn_addr, sizeof(struct sockaddr));
        if (sendto_ret < 0) {
            perror("sendto in warnning");
            return -1;
        }
    }
    DBG("\033[31m警告成功\033[0m\n");
    close(udpsock);
    return 0;
}

void make_single_log(struct Data *data) {
    //DBG("start make single log cnt = %d\n", data->cnt);
    FILE *pp = popen(data->cmd, "r");
    if(pp == NULL) { perror("popen"); return ;}
    char *dt_buf = data->sd[data->cnt].buf;
    char tmp_para[100];
    if (data->my_id == 2) {
        size_t len = 1024;
        getline(&dt_buf, &len, pp);
        size_t n = 100;
        fread(tmp_para, 1, MAX_DATA, pp);
        //DBG("得到的参数是：%lf\n", atof(tmp_para));
        msg->para_for_Mem = atof(tmp_para);
    } else {
        fread(dt_buf, 1, MAX_DATA, pp);
    }
    pclose(pp);
    //DBG("cnt = %d : %s\n", data->cnt, data->sd[data->cnt].buf);
    data->cnt++;
    if(data->cnt < 5) return;
    //DBG("累计五次信息，准备写入\n");
    FILE *p_tmp = fopen(data->savePath, "a+");
    flock(p_tmp->_fileno, LOCK_EX);
    if(p_tmp == NULL) { perror("fopen");return; }
    for (int i = 0; i < 5; i++) {
        //DBG("i = %d, strlen = %ld\n", i, strlen(data->sd[i].buf));
        fwrite(data->sd[i].buf, 1, strlen(data->sd[i].buf), p_tmp);
    }
    memset(data->sd, 0, 5 * sizeof(singleData));
    data->cnt = 0;
    fclose(p_tmp);
    flock(p_tmp->_fileno, LOCK_UN);

    //warnning_master();
}
void *do_make_log_info(void *arg) {
    Data Cpu, Mem, User, Disk, Dete, Stat;
    memset(&Cpu, 0, sizeof(Cpu));
    memset(&Mem, 0, sizeof(Cpu));
    memset(&User, 0, sizeof(Cpu));
    memset(&Disk, 0, sizeof(Cpu));
    memset(&Dete, 0, sizeof(Cpu));
    memset(&Stat, 0, sizeof(Cpu));
    Cpu.my_id = 1; Mem.my_id = 2; User.my_id = 3;
    Disk.my_id = 4; Dete.my_id = 5; Stat.my_id = 6;
    strcpy(Cpu.cmd, "bash ./1.ShellStuff/1.CpuLog.sh");
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
    int cnt = 0;
    while (1) {
        sprintf(Mem.cmd, "bash ./1.ShellStuff/2.MemLog.sh %lf", msg->para_for_Mem);
       /* 
        if (cnt == 6) {
            make_single_log(&Dete);
        } else if (cnt == 12) {
            make_single_log(&User);
            make_single_log(&Disk);
            make_single_log(&Stat);
            cnt = 0;
        }
        make_single_log(&Cpu);
        make_single_log(&Mem);
        cnt++;
        sleep(5);
*/
        make_single_log(&Cpu);
        make_single_log(&Mem);
        make_single_log(&User);
        make_single_log(&Disk);
        make_single_log(&Dete);
        make_single_log(&Stat);
        if (msg->heartbeat_flag != 0) {
            msg->makeinfo_times++;
            DBG("断线 %d 次 满 %d 开启断线重连\n", msg->makeinfo_times, atoi(ReconnTimes));
            if (msg->makeinfo_times >= atoi(ReconnTimes)) {
                DBG("断线超过 规定自检次数！Reconnect!\n");
                if(kill(msg->Reconn_pid, 10) == -1) { perror("kill");}
                msg->makeinfo_times = 0;
            }
        }
    }
}
int do_send(int now_fd, int epollfd) {
    LogInfo loginfo;

    char filename[6][256];
    memset(filename, 0, sizeof(filename));
    strcpy(filename[0], "./Clnt_LogInfo/Log_Cpu");
    strcpy(filename[1], "./Clnt_LogInfo/Log_Mem");
    strcpy(filename[2], "./Clnt_LogInfo/Log_User");
    strcpy(filename[3], "./Clnt_LogInfo/Log_Disk");
    strcpy(filename[4], "./Clnt_LogInfo/Log_Dete");
    strcpy(filename[5], "./Clnt_LogInfo/Log_Stat");
    int ret = 0;
    for (int i = 0; i < 6; i++) {
        DBG("文件 %d 开始发送！\n", i);
        FILE *fp = fopen(filename[i], "r");
        if (fp == NULL) perror("fopen i");
        flock(fp->_fileno, LOCK_EX);
        int cnt = 0;
        while(1) {
            memset(&loginfo, 0, sizeof(loginfo));
            int read_ret = fread(loginfo.buf, 1, 1024, fp);
            if (read_ret < 0) perror("fread in while1");
            if (read_ret == 0 && cnt == 0) {DBG("空文件跳过\n");break;}
            else if (read_ret == 0 && cnt > 0) { break; }
            loginfo.flag = i;
            loginfo.buflen = strlen(loginfo.buf);
            //DBG("包：\nflag = %d, stdlen = %d buf = \n%s\n", loginfo.flag, loginfo.buflen, loginfo.buf);
            int send_ret = send(now_fd, &(loginfo), sizeof(loginfo), 0);
            if (send_ret < 0) perror("send in while1");
            if (send_ret == 0) {
                DBG("对方关闭链接\n");
                ret = -1;
                break;
            }
            DBG("[文件%d] %d号包 | ", i, cnt++);
            //usleep(500000);
        }
        fclose(fp);
        DBG("文件 %d 处理成功！\n\n", i);
        if (ret != -1) {
            FILE *clear = fopen(filename[i], "w");
            if (fp == NULL) perror("fopen clear");
            fclose(clear);
        } else {
            flock(fp->_fileno, LOCK_UN);
            return -1;
        }
        flock(fp->_fileno, LOCK_UN);
    }
    return 0;
}
int do_recv(int now_fd, int epollfd) {
    char buff[1024];
    memset(buff, 0, sizeof(buff));
    int recv_ret = recv(now_fd, buff, sizeof(buff), 0);
    if(recv_ret < 0) {perror("recv in do recv"); return -1;}
    else DBG("**************recvstr : %s \n", buff);
    modify_event(epollfd, now_fd, EPOLLOUT);
    return 0;
}

void do_events(int epollfd, int nfds, int listen_socket, struct epoll_event *events) {
    for (int i = 0; i < nfds; i++) {
        int now_fd = events[i].data.fd;
        if (now_fd == listen_socket) {
            int master_socket = accept_clnt(listen_socket);
            if (master_socket == -1) {
                DBG("[work] 数据请求 连接失败\n");
            } else {
                DBG("[work] 数据请求链接成功\n");
                add_event(epollfd, master_socket, EPOLLIN);
            }
        } else if (now_fd != listen_socket && events[i].events & EPOLLIN) {
            DBG("准备接受数据\n");
            unsigned long un_ul = 0;
            ioctl(now_fd, FIONBIO, &un_ul);
            if (do_recv(now_fd, epollfd) == -1) {
                delete_event(epollfd, now_fd, 0);
                close(now_fd);
                return;
            }
            unsigned long bl_ul = 1;
            ioctl(now_fd, FIONBIO, &bl_ul);
        } else if(now_fd != listen_socket && events[i].events & EPOLLOUT) {
            DBG("准备发送数据\n");
            unsigned long un_ul = 0;
            ioctl(now_fd, FIONBIO, &un_ul);
            do_send(now_fd, epollfd);
            unsigned long bl_ul = 1;
            ioctl(now_fd, FIONBIO, &bl_ul);

            DBG("send success!!\n");
            delete_event(epollfd, now_fd, 0);
            close(now_fd);
            DBG("now_fd closed!\n");
        
        } else {
            DBG("不应该存在的情况\n");
        }
    }
}

void do_data_recv_send() {
    int listen_socket = get_listen_socket(clntIP, atoi(clntPORT));
    if(listen_socket < 0) { perror("[work] getlistensock"); exit(1); }
    int epollfd;
    struct epoll_event events[MAX_EVENTS];
    epollfd = epoll_create(1);
    add_event(epollfd, listen_socket, EPOLLIN);
    while(1) {
        DBG("-------------send N recv-----------------\n");
        int nfds = epoll_wait(epollfd, events, MAX_EVENTS, 3000);
        if (nfds == -1) { 
            perror("epoll_wait in do_data_send_recv");
        } else if (nfds == 0) {
            DBG("send N recv Timeout!\n");
        } else {
            do_events(epollfd, nfds, listen_socket, events);
            msg->heartbeat_flag = 0;
            msg->makeinfo_times=0;

        }
    }
    close(listen_socket);
    close(epollfd);
}

int real_main() {
    
    do_clnt_config();
    DBG("master IP = %s Master PORT =  %s\n", masterIP, masterPORT);
    DBG("clnt IP = %s clnt PORT =  %s\n", clntIP, clntHPORT);
    DBG("主进程：永远等待master发送心跳。\n");
    DBG("子进程：若孙子线程十次失败，子线程父承子业。成功则kill孙子进程。\n");
    DBG("孙子进程：主动链接master上限十次。之后无论成功失败都执行client本职任务。\n");
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
        DBG("[主] pid = %d ：监听心跳信号\n", getpid());
        pthread_t pthread_id;
        pthread_create(&pthread_id, NULL, do_make_log_info, NULL);
        heartbeat_recv();
    } else if (my_id == 1) {
        DBG("[子] pid = %d ：我是子进程, 在等待接受信号，断线重连时刻就绪\n", getpid());
        signal(10, grandson_lazy_connect);
        while(1) pause();
    } else if (my_id == 2) {
        DBG("[孙] pid = %d ：主动连接master\n", getpid());
        first_try_connect();
        DBG("***孙子进程开始进行数据收发工作。。。。。。。***\n");
        do_data_recv_send();         
    }
    return 0;
}

int main() {
    pid_t pid = fork();
    if (pid == 0) real_main();
    exit(0);
}

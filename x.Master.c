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
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <stdarg.h>
#include <sys/file.h>

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
    long long cnt = 0;
    while(1) {
        printf("子线程进行第%lld次心跳遍历检测\n", cnt);
        for (int i = 0; i < Ins; i++) {
            ClntInfoList *all_clnt = clntlist[i];
            
            clntnode *c = all_clnt->head;
            while(c->next) {
                if (!do_heartbeat(c->next)){
                    printf("[No. %d] 删除该服务器\n", all_clnt->my_id);
                    pthread_mutex_lock(&mutex);
                    clntnode *d = c->next;
                    c->next = c->next->next;
                    clntlist[i]->clnt_num--;
                    write_running_log(SysLog, "Delete client <%s>\n", get_ip_str(d));
                    free(d);
                    pthread_mutex_unlock(&mutex);
                    continue;
                    //List_delete(all_clnt, now_id);
                }
                c = c->next;
            }    
        }
        //show_list(all_clnt);
        //printf("--------------------------------\n");
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
        clntnode *tmp = List_add(all_clnt[min_list_id], clnt_addr.sin_addr.s_addr);
        write_running_log(SysLog, "New client : <%s> add success!\n", get_ip_str(tmp));
        pthread_mutex_unlock(&mutex);
        close(clnt_socket);
        return 1;
    } else {
        write_running_log(SysLog, "Same client : <%s> add failed! Already in List.\n", inet_ntoa(clnt_addr.sin_addr));
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
        List_add(all_clnt[min_list_id], ntohl(i));
    }
    pthread_mutex_unlock(&mutex);
    printf("%s~%s 插入完毕\n", startIP, endIP);
    for (int i = 0; i < Ins; i++) {
        printf("List %d : \n", i);
        show_list(all_clnt[i]);
    }
}

int do_save_log_file(int now_fd) {

    typedef struct LogInfo {
        int flag;
        int buflen;
        char buf[1025];
    } LogInfo;
    LogInfo loginfo;
    char tmp[1024]={0};

    int ret = 0;
    
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    getpeername(now_fd, (struct sockaddr *)&addr, &addrlen);
    char *ipPath = inet_ntoa(addr.sin_addr);
    char logDir[100];
    sprintf(logDir, "./Master_LogInfo/%s", ipPath);
    mkdir(logDir, 0777);
    mkdir("./Master_Sys_Log/", 0777);
    while (1) {
        memset(&loginfo, 0, sizeof(loginfo));
        int recv_ret = recv(now_fd, &loginfo, sizeof(loginfo), 0);
        /*
        while (recv_ret == -1 && errno == EAGAIN) {
            printf("\033[31m非阻塞： \033[0m strlen = %ld\n", strlen(loginfo.buf));
            recv_ret = recv(now_fd, &loginfo, sizeof(loginfo), 0);
            printf("\033[31m尝试recv!------------------------------\033[0m\n");
        }
        */
        if (recv_ret < 0) { perror("recv after judge"); ret = -1; break;}
        int recv_tmp = recv_ret;
        while (recv_tmp < sizeof(loginfo)) {
            printf("\033[31mflag=%d 拼凑字符串呢！ recvret = %d strlen = %ld 标准：%d \033[0m\n", loginfo.flag, recv_ret, strlen(loginfo.buf), loginfo.buflen);
            printf("\033[31m当前字符串：\n%s\033[0m\n", loginfo.buf);
            char tail_part[1024] = {0};
            recv_ret = recv(now_fd, tail_part, sizeof(loginfo)-recv_tmp, 0);
            recv_tmp += recv_ret;
            if (recv_ret == -1) {perror("recvc in 拼凑"); sleep(5);}
            if (recv_ret == 0) {printf("in 拼凑 对方关闭sock\n"); break;}
            strcat(loginfo.buf, tail_part);
        }
        char filename[1000] = {0};
        switch (loginfo.flag) {
            case 0:sprintf(filename, "%s/Save_Log_Cpu", logDir); break;
            case 1:sprintf(filename, "%s/Save_Log_Mem", logDir); break;
            case 2:sprintf(filename, "%s/Save_Log_User",logDir); break;
            case 3:sprintf(filename, "%s/Save_Log_Disk",logDir); break;
            case 4:sprintf(filename, "%s/Save_Log_Dete",logDir); break;
            case 5:sprintf(filename, "%s/Save_Log_Stat",logDir); break;
            case 100:printf("收到结束包\n"); return 0;
            default: 
            printf("\033[31m收到异常包\033[0m\n");
            strcpy(filename, "./Master_LogInfo/tmp"); break;
        }
        FILE *fp = fopen(filename, "a+");
        if (fp == NULL) { perror("fopen a+"); fclose(fp); return -1; }
        printf("包：flag = %d stdlen = %d buflen = %ld \n buf = %s\n", loginfo.flag, loginfo.buflen, strlen(loginfo.buf), loginfo.buf);
        if (recv_ret == 0) { printf("recv_ret == 0\n"); fclose(fp); break;}
        if (recv_ret != 0 ) {
            int writenum = (strlen(loginfo.buf) > 1024 ? 1024 : strlen(loginfo.buf));
            int fwrite_ret = fwrite(loginfo.buf, 1, writenum, fp);
            if (fwrite_ret >= 0) {
                printf("Log: 写入[%d]字节成功\n", fwrite_ret);
            } else {
                perror("fwrite");
            }
        } else {
            printf("空文件跳过\n");
        }
        fclose(fp);
    }
    return ret;
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
        for (clntnode *c = tmp_list->head; c->next != NULL; c = c->next) {
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
            printf("我connect 的是 ： ip ： %s  port : %s\n", cnext_ip_str, clntPORT);
            if (con_ret < 0 && errno == EINPROGRESS) {
                perror("connect in send : normal");
                int error = -1;
                int len = sizeof(error);
                if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len) < 0) {
                    perror("getsockopt");
                } else if (error == 0) {
                    printf("\033[31mgetsockopt 正常！\033[0m\n");
                } else {
                    printf("\033[31mgetsockopt 异常\033[0m\n");
                    close(sock);
                    sleep(1);
                    continue;
                }
            } else {
                perror("connect in send");
                close(sock);
                sleep(1);
                continue;
            }
            add_event(epollfd, sock, EPOLLOUT);
            while (1) {
                int nfds = epoll_wait(epollfd, events, atoi(MAX_WORK_EVENTS), 1000);
                if(nfds == -1) {
                    perror("epoll_wait in do_work");
                } else if(nfds == 0) {
                    printf("[send & recv] <%s>%d ：epoll wait do_work 超时,跳过该客户。\n",cnext_ip_str, sock);
                } else {
                    if (events[0].events & EPOLLOUT) {
                        char sendstr[] = "I want you!!!give it to me!!!";
                        printf("开始准备发送！\n");
                        int send_ret = send(sock, sendstr, strlen(sendstr), 0);
                        while (send_ret == -1 && errno == EAGAIN) {
                            send_ret = send(sock, sendstr, strlen(sendstr), 0);
                            printf("\033[31msend 阻塞的锅！！！\033[0m\n");
                        }

                        printf("发送结束\n");
                        if (send_ret <= 0) {
                            if (send_ret < 0) perror("send in do work"); else { printf("send_ret==0\n"); } 
                            printf("delete_event 111\n");
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

                        unsigned long new_ul = 0;
                        ioctl(sock, FIONBIO, &new_ul);
                        int do_save_log_file_ret = do_save_log_file(sock);
                        unsigned long new_ul2 = 1;
                        ioctl(sock, FIONBIO, &new_ul2);
                        if (do_save_log_file_ret < 0) {
                            printf("\n\nPlease dont be here!!!!!!!!!!!!!!!!!!!!\n\n");
                            write_running_log(SysLog, "<%s> Save Log Error!\n", cnext_ip_str);
                            unsigned long ul = 1;
                            ioctl(sock, FIONBIO, &ul);

                            printf("delete_event 222\n");
                            delete_event(epollfd, sock, 0);
                            close(sock);
                            sleep(1);
                            break;
                        }
                        printf("[send & recv] <%s>%d ： save success！\n",cnext_ip_str, sock);
                    }
                }
                printf("delete_event 333\n");
                delete_event(epollfd, sock, 0);
                close(sock);
                printf("[send & recv] <%s>%d ： clear success！\n",cnext_ip_str, sock);
                sleep(atoi(Send_Recv_Time));  //10s收发一次
                break;
            }
            //c = c->next;
        }
    clear_List(tmp_list);
    }
}

/*
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
        for (clntnode *c = tmp_list->head; c->next != NULL; c = c->next) {
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
            printf("before connect: port = %s  ip = %s\n", clntPORT, cnext_ip_str);
            int con_ret = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
            if (con_ret < 0 && errno == EINPROGRESS) {
                printf("connect in send : normal\n");
            } else {
                perror("connect in send");
            }
            if (con_ret >= 0) add_event(epollfd, sock, EPOLLOUT);

            while (1) {
                int nfds = epoll_wait(epollfd, events, atoi(MAX_WORK_EVENTS), 1000);
                if(nfds == -1) {
                    perror("epoll_wait in do_work");
                } else if(nfds == 0) {
                    printf("[send & recv] <%s>%d ：epoll wait do_work 超时,跳过该客户。\n",cnext_ip_str, sock);
                } else {
                    if (events[0].events & EPOLLOUT) {
                        char sendstr[] = "I want you!!!give it to me!!!";
                        printf("开始准备发送！\n");
                        int send_ret = send(sock, sendstr, strlen(sendstr), 0);
                        while (send_ret == -1 && errno == EAGAIN) {
                            send_ret = send(sock, sendstr, strlen(sendstr), 0);
                            printf("\033[31msend 阻塞的锅！！！\033[0m\n");
                        }

                        printf("发送结束\n");
                        if (send_ret <= 0) {
                            if (send_ret < 0) perror("send in do work"); else { printf("send_ret==0\n"); } 
                            printf("delete_event 111\n");
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

                        unsigned long new_ul = 0;
                        ioctl(sock, FIONBIO, &new_ul);
                        int do_save_log_file_ret = do_save_log_file(sock);
                        unsigned long new_ul2 = 1;
                        ioctl(sock, FIONBIO, &new_ul2);
                        if (do_save_log_file_ret < 0) {
                            printf("\n\nPlease dont be here!!!!!!!!!!!!!!!!!!!!\n\n");
                            
                            unsigned long ul = 1;
                            ioctl(sock, FIONBIO, &ul);

                            printf("delete_event 222\n");
                            delete_event(epollfd, sock, 0);
                            close(sock);
                            sleep(1);
                            break;
                        }
                        printf("[send & recv] <%s>%d ： save success！\n",cnext_ip_str, sock);
                    }
                }
                printf("delete_event 333\n");
                delete_event(epollfd, sock, 0);
                close(sock);
                printf("[send & recv] <%s>%d ： clear success！\n",cnext_ip_str, sock);
                sleep(Send_Recv_Time);  //10s收发一次
                break;
            }
            //c = c->next;
        }
    clear_List(tmp_list);
    //这里不注释吧？？？？？？
    }
}
*/
void *warnning_recv() {
    int flag = 0;
    int udpsock;
    struct sockaddr_in serv_addr;
    
    while (flag == 0) {
        flag = 1;
        make_sockaddr_in(&serv_addr, masterIP, masterWPORT);

        udpsock = socket(AF_INET, SOCK_DGRAM, 0);
        if (udpsock < 0) {
            perror("socket in warnning");
            flag = 0;
        }
        int bind_ret = bind(udpsock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
        if (bind_ret < 0) {
            perror("bind");
            flag = 0;
        }
    }
    int len = sizeof(struct sockaddr);
    while (1) {
        int warn = 0;
        int recvfrom_ret = recvfrom(udpsock, &warn, sizeof(warn), 0, (struct sockaddr *)&serv_addr, &len);
        if (recvfrom_ret < 0) {
            perror("recvfrom");
        } else if (warn == 1) {
            printf("\033[31m收到客户端警报!!!!!!!!!!!!!!!!!!\033[0m\n");
            sleep(5);
        }
    }
}

int main() {
    write_running_log(SysLog, "Master Start!\n");
    printf("pid = %d\n", getpid());
    pthread_mutex_init(&mutex, NULL);
    do_master_config();
    Ins = atoi(INS);
    printf("Ins = %d\n", Ins);
    PClntInfoList *clnt_list;
    clnt_list=all_init(Ins);
    
    add_all_clnt(clnt_list, Ins);
    write_running_log(SysLog, "From <%s> to <%s> added success!\n", startIP, endIP);
    pthread_t pthread_id[Ins + 5];
	pthread_create(&pthread_id[Ins], NULL, heartbeat, clnt_list);
    write_running_log(SysLog, "HeartBeat Ready!\n");
	//pthread_create(&pthread_id[Ins + 1], NULL, warnning_recv, NULL);
    for(int i = 0; i < Ins; i++) {
	    pthread_create(&pthread_id[i], NULL, do_work, clnt_list[i]);
        write_running_log(SysLog, "do_work[%d] Ready!\n", i);
    }
    int listen_socket = get_listen_socket(masterIP, atoi(masterPORT));
    if (listen_socket < 0) {
        perror("get_listen_socket in main"); 
        write_running_log(SysLog, "getlistensocket: %s\n", strerror(errno));
        exit(1);
    } 
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
        /*
        int recv_tmp = recv_ret;
        while (recv_tmp < sizeof(loginfo)) {
            printf("\033[31mflag=%d 拼凑字符串呢！ recvret = %d strlen = %ld 标准：%d \033[0m\n", loginfo.flag, recv_ret, strlen(loginfo.buf), loginfo.buflen);
            printf("\033[31m当前字符串：\n%s\033[0m\n", loginfo.buf);
            char tail_part[1024] = {0};
            recv_ret = recv(now_fd, , sizeof(tail_part), 0);
            if (recv_ret == 0) break;
            strcat(loginfo.buf, tail_part);
            
        }
        */
/*
        for (clntnode *c = tmp_list->head; c->next != NULL; c = c->next) {
            char cnext_ip_str[100];
            memset(cnext_ip_str, 0, sizeof(cnext_ip_str));
            //if(c->next != NULL) {
            //    printf("id = %d, ip = %s\n", c->next->id, get_ip_str(c->next));
            //    strcpy(cnext_ip_str, get_ip_str(c->next));
            //} else {
            //    printf("c->next == NULL  break\n");
            //    break;
            //}

            int sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock < 0) { perror("socket in do_work"); break;}
            struct sockaddr_in serv_addr;
            make_sockaddr_in(&serv_addr, cnext_ip_str, clntPORT);
            //unsigned long ul = 1;
            //ioctl(sock, FIONBIO, &ul);
            int con_ret = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
            //if (con_ret < 0 && errno == EINPROGRESS) {
            //    printf("connect in send : normal\n");
            //} else {
            //    perror("connect in send");
            //}
            perror("connect important");
            if (con_ret < 0 && errno == EINPROGRESS) {
                printf("connect in send : normal\n");
                int error = -1;
                int len = sizeof(error);
                if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len) < 0) {
                    perror("getsockopt");
                } else if (error == 0) {
                    printf("\033[31m应该能发过去啊a！\033[0m\n");
                } else {
                    printf("\033[31m怪不得发不过\033[0m\n");
                }
            } else { 
                perror("connect in send");
                sleep(1);
                continue;
            }
            add_event(epollfd, sock, EPOLLOUT);
*/


            /*
            while(1) {
                int nfds = epoll_wait(epollfd, events, atoi(MAX_WORK_EVENTS), 1000);
                if(nfds == -1) {
                    perror("epoll_wait in do_work");
                } else if(nfds == 0) {
                    printf("[send & recv] <%s>%d ：epoll wait do_work 超时,跳过该客户。\n",cnext_ip_str, sock);
                } else {
                    if (events[0].events & EPOLLOUT) {
                        char sendstr[] = "I want you!!!give it to me!!!";
                        printf("开始准备发送！\n");
                        int send_ret = send(sock, sendstr, strlen(sendstr), 0);
                        printf("发送结束\n");
                        if (send_ret <= 0) {
                            if (send_ret < 0) perror("send in do work"); else { printf("send_ret==0\n"); } 
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
                            printf("\n\nPlease dont be here!!!!!!!!!!!!!!!!!!!!\n\n");
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
                sleep(Send_Recv_Time);  //1s收发一次
                break;
            }
            */


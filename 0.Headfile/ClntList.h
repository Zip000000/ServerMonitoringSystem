/*************************************************************************
	> File Name: ClntList.h
	> Author: Zip 
	> Mail: 307110017@qq.com 
	> Created Time: 2019年08月08日 星期四 19时42分11秒
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
 #include <errno.h>
 #include <time.h>
 #include <stdarg.h>
 #include <sys/stat.h>




#ifndef _CLNTLIST_H
#define _CLNTLIST_H


typedef struct clnt_node {
    unsigned int ip;
    int id;
    char logPath[32];
    struct clnt_node *next;
} clntnode;

typedef struct ClntInfoList {
    clntnode *head;
    int clnt_num;
    int maxid;
    int my_id;
} ClntInfoList, *PClntInfoList;

ClntInfoList *Clnt_Info_list_init(int id) ;
char *get_ip_str(clntnode *);
PClntInfoList *all_init(int Ins) ;
clntnode *List_add(ClntInfoList *all_clnt, unsigned int ip) ;
int get_min_list_id(PClntInfoList *clnt_list, int Ins) ;
int List_delete(ClntInfoList *all_clnt, int id) ;
int is_in_list(PClntInfoList *all_clnt, unsigned int ip, int Ins) ;
void show_list(ClntInfoList *l) ;
char *get_ip_str(clntnode *n) ;
ClntInfoList *copy_List(const ClntInfoList *l) ;
void clear_List(ClntInfoList *a) ;

#endif

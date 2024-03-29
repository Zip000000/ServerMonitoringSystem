/*************************************************************************
	> File Name: Epoll.h
	> Author: Zip 
	> Mail: 307110017@qq.com 
	> Created Time: 2019年08月08日 星期四 19时39分32秒
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


#ifndef _EPOLL_H
#define _EPOLL_H


int add_event(int epollfd,int fd,int state);
int delete_event(int epollfd,int fd,int state);
int modify_event(int epollfd,int fd,int state);

#endif


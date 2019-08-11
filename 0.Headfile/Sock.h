/*************************************************************************
	> File Name: Sock.h
	> Author: Zip 
	> Mail: 307110017@qq.com 
	> Created Time: 2019年08月08日 星期四 19时51分30秒
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


#ifndef _SOCK_H
#define _SOCK_H
void make_sockaddr_in(struct sockaddr_in *addr, char *ip, char *port) ;
int get_listen_socket(char *ip, int port) ;
int  accept_clnt(int listen_socket) ;
#endif






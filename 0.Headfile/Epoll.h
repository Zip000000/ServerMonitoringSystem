/*************************************************************************
	> File Name: Epoll.h
	> Author: Zip 
	> Mail: 307110017@qq.com 
	> Created Time: 2019年08月08日 星期四 19时39分32秒
 ************************************************************************/

#ifndef _EPOLL_H
#define _EPOLL_H


int add_event(int epollfd,int fd,int state);
int delete_event(int epollfd,int fd,int state);
int modify_event(int epollfd,int fd,int state);

#endif


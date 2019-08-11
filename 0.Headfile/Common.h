/*************************************************************************
	> File Name: Common.h
	> Author: Zip 
	> Mail: 307110017@qq.com 
	> Created Time: 2019年08月08日 星期四 19时40分29秒
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


#ifndef _COMMON_H
#define _COMMON_H
extern char startIP[100];
extern char endIP[100];
extern char masterIP[100];
extern char masterPORT[100];
extern char masterWPORT[100];
extern char clntHPORT[100];
extern char clntPORT[100];
extern char clntIP[100];
extern char INS[100];
extern char MAX_EVENTS[100];
extern char MAX_WORK_EVENTS[100];
extern char HeartbeatTimeout[100];
extern char ReconnTimes[100];
extern char Send_Recv_Time[100];
extern char SysLog[100];
extern char ShellDir[100];
extern char LoginfoDir[100];
extern char MasterLogDir[100];

int get_conf_value(const char *file, const char *key, char *val) ;
void do_master_config() ;
void do_clnt_config() ;
void write_running_log(char *filename, char *format, ...) ;

#endif



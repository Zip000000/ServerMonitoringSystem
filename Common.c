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


#ifndef Common_Define
#define Common_Define
int get_conf_value(const char *file, const char *key, char *val) {
    if (key == NULL || val == NULL) {printf("Wrong parameters\n"); return -1;}
    
    FILE *fp = fopen(file, "r");
    if (fp == NULL) {perror("fopen"); return -1;}
    
    int readnum;
    char *line= (char *)malloc(sizeof(char) * 100);
    size_t n;

    while ((readnum = getline(&line, &n, fp)) != -1) {
        char *p = strstr(line, key);
        if (p == NULL) continue;
        int len = strlen(key);
        if(p[len] != '=') continue;
        strncpy(val, p+len+1, (int)readnum - len - 2);
        break;
    }
    if(readnum == 0) {
        printf("%s Not Found!\n", key);
        free(line);
        return 1;
    }
    return 0;
}

char startIP[100];
char endIP[100];
char masterIP[100];
char masterPORT[100];
char masterWPORT[100];
char clntHPORT[100];
char clntPORT[100];
char clntIP[100];
char INS[100];
char MAX_EVENTS[100];
char MAX_WORK_EVENTS[100];
char HeartbeatTimeout[100];
char ReconnTimes[100];
char Send_Recv_Time[100];
char SysLog[100];

void do_master_config() {
    memset(clntPORT, 0, sizeof(clntPORT));
    get_conf_value("config", "ClntPORT", clntPORT);
    memset(clntHPORT, 0, sizeof(clntHPORT));
    get_conf_value("config", "ClntHPORT", clntHPORT);
    memset(masterIP, 0, sizeof(masterIP));
    memset(masterPORT, 0, sizeof(masterPORT));
    get_conf_value("config", "MasterIP", masterIP);
    get_conf_value("config", "MasterPORT", masterPORT);
    memset(startIP, 0, sizeof(startIP));
    memset(endIP, 0, sizeof(endIP));
    get_conf_value("config", "startIP", startIP);
    get_conf_value("config", "endIP", endIP);
    memset(INS, 0, sizeof(INS));
    get_conf_value("config", "Ins", INS);
    memset(MAX_EVENTS, 0, sizeof(MAX_EVENTS));
    get_conf_value("config", "Master_MAX_EVENTS", MAX_EVENTS);
    memset(MAX_WORK_EVENTS, 0, sizeof(MAX_WORK_EVENTS));
    get_conf_value("config", "Master_MAX_WORK_EVENTS", MAX_WORK_EVENTS);
    memset(HeartbeatTimeout, 0, sizeof(HeartbeatTimeout));
    get_conf_value("config", "HeartbeatTimeout",HeartbeatTimeout);

    memset(masterWPORT, 0, sizeof(masterWPORT));
    get_conf_value("config", "MasterWPORT", masterWPORT);

    memset(Send_Recv_Time, 0, sizeof(Send_Recv_Time));
    get_conf_value("config", "Send_Recv_Time", Send_Recv_Time);
    
    memset(SysLog, 0, sizeof(SysLog));
    get_conf_value("config", "Master_Sys_Log", SysLog);
}


void do_clnt_config() {
    memset(clntPORT, 0, sizeof(clntPORT));
    get_conf_value("config", "ClntPORT", clntPORT);
    memset(clntIP, 0, sizeof(clntIP));
    memset(clntHPORT, 0, sizeof(clntHPORT));
    get_conf_value("config", "ClntIP", clntIP);
    get_conf_value("config", "ClntHPORT", clntHPORT);
    memset(masterIP, 0, sizeof(masterIP));
    memset(masterPORT, 0, sizeof(masterPORT));
    get_conf_value("config", "MasterIP", masterIP);
    get_conf_value("config", "MasterPORT", masterPORT);
    memset(MAX_EVENTS, 0, sizeof(MAX_EVENTS));
    get_conf_value("config", "Clnt_MAX_EVENTS", MAX_EVENTS);
    memset(ReconnTimes, 0, sizeof(ReconnTimes));
    get_conf_value("config", "ReconnTimes", ReconnTimes);

    memset(masterWPORT, 0, sizeof(masterWPORT));
    get_conf_value("config", "MasterWPORT", masterWPORT);
    
    memset(SysLog, 0, sizeof(SysLog));
    get_conf_value("config", "Clnt_Sys_Log", SysLog);
}


void write_running_log(char *filename, char *format, ...) {
    va_list vl;
    va_start(vl, format);
    printf("in WRIT RUNNINT LOG\n");
    FILE *fp = fopen(filename, "a+");
    if (fp == NULL) {return; }
    time_t timer = time(NULL);
    char *ct = ctime(&timer);
    ct[strlen(ct)-1] = 0;
    char *str_err = NULL;
    flock(fp->_fileno, LOCK_EX);
    fprintf(fp, "[%s] ", ct);
    vfprintf(fp, format, vl);
    flock(fp->_fileno, LOCK_UN);
    fclose(fp);
    va_end(vl);
}



#endif
/*
void write_running_log(char *format, ...) {
    va_list vl;
    va_start(vl, format);
    printf("in WRIT RUNNINT LOG\n");
    FILE *fp = fopen("./Clnt_Running_Log/1.Log", "a+");
    if (fp == NULL) {return; }
    time_t timer = time(NULL);
    char *ct = ctime(&timer);
    ct[strlen(ct)-1] = 0;
    char *str_err = NULL;
    flock(fp->_fileno, LOCK_EX);
    fprintf(fp, "[%s]", ct);
    vfprintf(fp, format, vl);
    flock(fp->_fileno, LOCK_UN);
    fclose(fp);
    va_end(vl);
}
*/

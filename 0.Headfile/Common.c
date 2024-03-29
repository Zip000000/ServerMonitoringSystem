
#include "Common.h"

#ifdef __DEBUG__
#define DBG(format, ...) printf (format, ##__VA_ARGS__)
#define PERR(format) perror(format)
#else
#define DBG(format, ...)
#define PERR(format) 
#endif
#define CONFIG_PATH "/etc/Zip.pihealth.config"


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
char ShellDir[100];
char LoginfoDir[100];
char MasterLogDir[100];

int get_conf_value(const char *file, const char *key, char *val) {
    if (key == NULL || val == NULL) {DBG("Wrong parameters\n"); return -1;}
    
    FILE *fp = fopen(file, "r");
    if (fp == NULL) {
        //perror("fopen");
        return -1;
    }
    
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
void do_master_config() {
    memset(clntPORT, 0, sizeof(clntPORT));
    get_conf_value(CONFIG_PATH, "ClntPORT", clntPORT);
    memset(clntHPORT, 0, sizeof(clntHPORT));
    get_conf_value(CONFIG_PATH, "ClntHPORT", clntHPORT);
    memset(masterIP, 0, sizeof(masterIP));
    memset(masterPORT, 0, sizeof(masterPORT));
    get_conf_value(CONFIG_PATH, "MasterIP", masterIP);
    get_conf_value(CONFIG_PATH, "MasterPORT", masterPORT);
    memset(startIP, 0, sizeof(startIP));
    memset(endIP, 0, sizeof(endIP));
    get_conf_value(CONFIG_PATH, "startIP", startIP);
    get_conf_value(CONFIG_PATH, "endIP", endIP);
    memset(INS, 0, sizeof(INS));
    get_conf_value(CONFIG_PATH, "Ins", INS);
    memset(MAX_EVENTS, 0, sizeof(MAX_EVENTS));
    get_conf_value(CONFIG_PATH, "Master_MAX_EVENTS", MAX_EVENTS);
    memset(MAX_WORK_EVENTS, 0, sizeof(MAX_WORK_EVENTS));
    get_conf_value(CONFIG_PATH, "Master_MAX_WORK_EVENTS", MAX_WORK_EVENTS);
    memset(HeartbeatTimeout, 0, sizeof(HeartbeatTimeout));
    get_conf_value(CONFIG_PATH, "HeartbeatTimeout",HeartbeatTimeout);

    memset(masterWPORT, 0, sizeof(masterWPORT));
    get_conf_value(CONFIG_PATH, "MasterWPORT", masterWPORT);

    memset(Send_Recv_Time, 0, sizeof(Send_Recv_Time));
    get_conf_value(CONFIG_PATH, "Send_Recv_Time", Send_Recv_Time);
    
    memset(SysLog, 0, sizeof(SysLog));
    get_conf_value(CONFIG_PATH, "Master_Sys_Log", SysLog);
    memset(MasterLogDir, 0, sizeof(MasterLogDir));
    get_conf_value(CONFIG_PATH, "MasterLogDir", MasterLogDir);
}


void do_clnt_config() {
    memset(clntPORT, 0, sizeof(clntPORT));
    get_conf_value(CONFIG_PATH, "ClntPORT", clntPORT);
    memset(clntIP, 0, sizeof(clntIP));
    memset(clntHPORT, 0, sizeof(clntHPORT));
    get_conf_value(CONFIG_PATH, "ClntIP", clntIP);
    get_conf_value(CONFIG_PATH, "ClntHPORT", clntHPORT);
    memset(masterIP, 0, sizeof(masterIP));
    memset(masterPORT, 0, sizeof(masterPORT));
    get_conf_value(CONFIG_PATH, "MasterIP", masterIP);
    get_conf_value(CONFIG_PATH, "MasterPORT", masterPORT);
    memset(MAX_EVENTS, 0, sizeof(MAX_EVENTS));
    get_conf_value(CONFIG_PATH, "Clnt_MAX_EVENTS", MAX_EVENTS);
    memset(ReconnTimes, 0, sizeof(ReconnTimes));
    get_conf_value(CONFIG_PATH, "ReconnTimes", ReconnTimes);

    memset(masterWPORT, 0, sizeof(masterWPORT));
    get_conf_value(CONFIG_PATH, "MasterWPORT", masterWPORT);
    
    memset(SysLog, 0, sizeof(SysLog));
    get_conf_value(CONFIG_PATH, "Clnt_Sys_Log", SysLog);
    memset(ShellDir, 0, sizeof(ShellDir));
    get_conf_value(CONFIG_PATH, "ShellDir", ShellDir);
    memset(LoginfoDir, 0, sizeof(LoginfoDir));
    get_conf_value(CONFIG_PATH, "LoginfoDir", LoginfoDir);
}


void write_running_log(char *filename, char *format, ...) {
    va_list vl;
    va_start(vl, format);
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



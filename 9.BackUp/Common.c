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
char clntHPORT[100];
char clntIP[100];
char INS[100];
char MAX_EVENTS[100];

void do_master_config() {
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
}


void do_clnt_config() {
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

}

#endif

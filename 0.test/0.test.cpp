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
#include "Common.c"
#include "Sock.c"
#include "Epoll.c"

#define MAX_DATA 1024
typedef struct singleData {
    char buf[MAX_DATA];
} singleData;
typedef struct Data {
    char cmd[256];
    char savePath[256];
    singleData sd[5];
    int cnt;
} Data;

void make_single_log(struct Data *data) {
    printf("start make single log cnt = %d\n", data->cnt);
    FILE *pp = popen(data->cmd, "r");
    if(pp == NULL) { perror("popen"); return ;}
    char *dt_buf = data->sd[data->cnt].buf;
    fread(dt_buf, 1, MAX_DATA, pp);
    pclose(pp);
    printf("cnt = %d : %s\n", data->cnt, data->sd[data->cnt].buf);
    data->cnt++;
    printf("before judge\n");
    if(data->cnt < 5) return;
    printf("after judge\n");
    FILE *p_tmp = fopen(data->savePath, "a+");
    if(p_tmp == NULL) { perror("fopen");return; }
    for (int i = 0; i < 5; i++) {
        printf("i = %d, strlen = %ld\n", i, strlen(data->sd[i].buf));
        fwrite(data->sd[i].buf, 1, strlen(data->sd[i].buf), p_tmp);
    }
    memset(data->sd, 0, 5 * sizeof(singleData));
    data->cnt = 0;
    fclose(p_tmp);
}
void *do_make_log_info(void *arg) {
    Data Cpu, Mem, User, Disk, Dete, Stat;
    memset(&Cpu, 0, sizeof(Cpu));
    memset(&Mem, 0, sizeof(Cpu));
    memset(&User, 0, sizeof(Cpu));
    memset(&Disk, 0, sizeof(Cpu));
    memset(&Dete, 0, sizeof(Cpu));
    memset(&Stat, 0, sizeof(Cpu));
    strcpy(Cpu.cmd, "bash ./1.ShellStuff/1.CpuLog.sh");
    strcpy(Mem.cmd, "bash ./1.ShellStuff/2.MemLog.sh");
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
        make_single_log(&Cpu);
        make_single_log(&Mem);
        make_single_log(&User);
        make_single_log(&Disk);
        make_single_log(&Dete);
        make_single_log(&Stat);
        sleep(2);
        /*
        if (cnt == 6) {
            make_single_log(Dete, Log_Dete);
        } else if (cnt == 12) {
            make_single_log(Stat, Log_Stat);
            make_single_log(User, Log_User);
            make_single_log(Disk, Log_Disk);
            cnt = 0;
        }
        make_single_log(Cpu, Log_Cpu);
        make_single_log(Mem, Log_Mem);
        cnt++;
        sleep(5);
    */
    }
}
int main() {
        pthread_t pthread_id;
        pthread_create(&pthread_id, NULL, do_make_log_info, NULL);
        pthread_join(pthread_id, NULL);
    return 0;
}

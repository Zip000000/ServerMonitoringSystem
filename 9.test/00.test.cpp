/*************************************************************************
	> File Name: 00.test.c
	> Author: Zip 
	> Mail: 307110017@qq.com 
	> Created Time: 2019年08月07日 星期三 14时17分11秒
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



int main() {
    /*
    */
    close(0);
    close(1);
    close(2);
    while (1) {
        FILE *pp = popen("ls", "r");
        if (pp == NULL) { perror("popen"); return 1; }
        sleep(1);
    }
    return 0;
}

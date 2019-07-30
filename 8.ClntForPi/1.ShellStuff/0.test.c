/*************************************************************************
	> File Name: 0.test.c
	> Author: Zip 
	> Mail: 307110017@qq.com 
	> Created Time: 2019年07月30日 星期二 20时22分34秒
 ************************************************************************/

#include<stdio.h>

int main() {

    FILE *p = popen("bash ./4.Disk.sh", "r");
    char buf[1000];
    fread(buf, 1, sizeof(buf), p);
    printf("%s\n", buf);
    perror("popen");
    return 0;
}

/*************************************************************************
	> File Name: 7.get_conf.c
	> Author: Zip 
	> Mail: 307110017@qq.com 
	> Created Time: 2019年07月18日 星期四 19时34分10秒
 ************************************************************************/

#include<stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CONF_FILE "config"
#define MAX_LINE 1000

int get_conf_value(const char *file, char *key, char *val) {
    if (key == NULL || val == NULL) {printf("Wrong parameters\n"); return -1;}
    
    FILE *fp = fopen(file, "r");
    if (fp == NULL) {perror("fopen"); return -1;}
    
    int readnum;
    char *line= (char *)malloc(sizeof(char) * MAX_LINE);
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


int main(int argc, char *argv[]) {
    /*
    if(argc != 3) {
        printf("正确输入格式：file, key, val\n")
        exit(1);
    }
    char *file = argv[1];
    */
    char age[100]="Hi";
    char key[100]="name";
    get_conf_value(CONF_FILE, key, age);
    printf("age = %s\n", age);
}

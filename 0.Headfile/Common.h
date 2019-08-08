/*************************************************************************
	> File Name: Common.h
	> Author: Zip 
	> Mail: 307110017@qq.com 
	> Created Time: 2019年08月08日 星期四 19时40分29秒
 ************************************************************************/

#ifndef _COMMON_H
#define _COMMON_H
int get_conf_value(const char *file, const char *key, char *val) ;
void do_master_config() ;
void do_clnt_config() ;
void write_running_log(char *filename, char *format, ...) ;

#endif

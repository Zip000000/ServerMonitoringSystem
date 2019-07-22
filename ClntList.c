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


#ifndef ClntList
#define ClntList
typedef struct clnt_node {
        unsigned int ip;
        int id;
        struct clnt_node *next;
} clntnode;

typedef struct ClntInfoList {
        clntnode *head;
        int clnt_num;
        int maxid ;

} ClntInfoList;

ClntInfoList *all_clnt;

ClntInfoList *Clnt_Info_list_init() {
        all_clnt = (ClntInfoList *)malloc(sizeof(ClntInfoList));
        all_clnt->clnt_num = 0;
        all_clnt->maxid = 0;
        all_clnt->head = (clntnode *)malloc(sizeof(clntnode));
        all_clnt->head->ip = 0;
        all_clnt->head->id = 0;
        all_clnt->head->next = NULL;
        return all_clnt;

}

ClntInfoList *List_add(ClntInfoList *all_clnt, unsigned int ip) {
        clntnode *n = (clntnode *)malloc(sizeof(clntnode));
        all_clnt->clnt_num += 1;
        all_clnt->maxid += 1;
        n->ip = ip;
        n->id = all_clnt->maxid;
        
        n->next = all_clnt->head->next;
        all_clnt->head->next = n;   //头插
        return all_clnt;
}

int List_delete(ClntInfoList *all_clnt, int id) {
        if(id > all_clnt->maxid) return 0;
        clntnode *p, *q;
        p = all_clnt->head;
        q = p->next;
    while(q) {
        if(q->id == id) {
                        p->next = q->next;
            all_clnt->clnt_num--;
                        free(q);
                        return 1;
                    
        }
                p = q;
                q = q->next;
            
    }
        return 0;
}

int is_in_list(ClntInfoList *all_clnt, unsigned int ip) {
        if(all_clnt->clnt_num == 0) return 0;
        clntnode *p;
        p = all_clnt->head->next;
    while(p) {
        if(p->ip ==ip) return 1;
        p = p->next;
    }
        return 0;
}

void show_list(ClntInfoList *l) {
    clntnode *c;
    c = l->head->next;
    printf("clntnum = %d maxid = %d\n", l->clnt_num, l->maxid);
    while(c != NULL) {
        struct in_addr in;
        in.s_addr = c->ip;
        printf("[No. %d] %s\n", c->id, inet_ntoa(in));
        c = c->next;
    }

}

char *get_ip_str(clntnode *n) {
    struct in_addr in;
    in.s_addr = n->ip;
    return inet_ntoa(in);
}

#endif

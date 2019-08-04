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
    char logPath[32];
    struct clnt_node *next;
} clntnode;

typedef struct ClntInfoList {
    clntnode *head;
    int clnt_num;
    int maxid;
    int my_id;
} ClntInfoList, *PClntInfoList;
//PClntInfoList *clnt_list;
//ClntInfoList *all_clnt;
// all = all_init(Ins);

ClntInfoList *Clnt_Info_list_init(int id) {
    ClntInfoList *all_clnt;
    all_clnt = (ClntInfoList *)malloc(sizeof(ClntInfoList));
    all_clnt->clnt_num = 0;
    all_clnt->maxid = 0;
    all_clnt->my_id = id;
    all_clnt->head = (clntnode *)malloc(sizeof(clntnode));
    all_clnt->head->ip = 0;
    all_clnt->head->id = 0;
    all_clnt->head->next = NULL;
    return all_clnt;

}
char *get_ip_str(clntnode *);

PClntInfoList *all_init(int Ins) {
    PClntInfoList *a = (PClntInfoList *)malloc(sizeof(PClntInfoList) * Ins);
    for(int i = 0; i < Ins; i++) {
        a[i] = Clnt_Info_list_init(i);
    }
    return a;
}

ClntInfoList *List_add(ClntInfoList *all_clnt, unsigned int ip) {
    clntnode *n = (clntnode *)malloc(sizeof(clntnode));
    all_clnt->clnt_num += 1;
    all_clnt->maxid += 1;
    n->ip = ip;
    n->id = all_clnt->maxid;
    n->next = all_clnt->head->next;
    memset(n->logPath, 0, 32);
    strcpy(n->logPath, get_ip_str(n));
    all_clnt->head->next = n;   //头插
    return all_clnt;
}
int get_min_list_id(PClntInfoList *clnt_list, int Ins) {
    int min_id = 0;
    for (int i = 0; i < Ins; i++) {
        if(clnt_list[i]->clnt_num < clnt_list[min_id]->clnt_num) min_id = i;
    }
    return min_id;
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

int is_in_list(PClntInfoList *all_clnt, unsigned int ip, int Ins) {
    for (int i = 0; i < Ins; i++) {
        if(all_clnt[i]->clnt_num == 0) continue;
        clntnode *p;
        p = all_clnt[i]->head->next;
        while(p) {
            if(p->ip ==ip) return 1;
            p = p->next;
        }
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
ClntInfoList *copy_List(const ClntInfoList *l) {
    ClntInfoList *ret = Clnt_Info_list_init(0);  //参数无所谓，后面被修改了
    clntnode *p = l->head;
    ret->clnt_num = l->clnt_num;
    ret->maxid = l->maxid;
    ret->my_id = l->my_id;
    clntnode *ret_p = ret->head;
    while (p->next) {
        p = p->next;
        clntnode *n = (clntnode *)malloc(sizeof(clntnode));
        n->ip = p->ip;
        n->id = p->id;
        n->next = ret_p->next;
        ret_p->next = n;
        ret_p = n;
    }
    return ret;
    /*
    ClntInfoList *ret = Clnt_Info_list_init(0);  //参数无所谓，后面被修改了
    clntnode *o = l->head;
    ret->clnt_num = l->clnt_num;
    ret->maxid = l->maxid;
    ret->my_id = l->my_id;
    clntnode *ret_p = ret->head;
    while (o->next != NULL) {
        o = o->next;
        clntnode *n = (clntnode *)malloc(sizeof(clntnode));
        n->id = o->ip;
        n->id = o->id;
        n->next = ret_p->next;
        ret_p->next = n;

        ret_p = ret_p->next;
    }
    ret_p->next = NULL; //感觉这句多余了
    return ret;
    */
}
void clear_List(ClntInfoList *a) {
    if (a == NULL) return;
    clntnode *d = a->head;
    while(d != NULL) {
        a->head = d->next;
        free(d);
        d = a->head;
    }
    free(a);
    return ;
}

#endif

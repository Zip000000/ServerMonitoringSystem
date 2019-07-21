#ifndef My_Epoll
#define My_Epoll
static void add_event(int epollfd,int fd,int state)
{
        struct epoll_event ev;
        ev.events = state;
        ev.data.fd = fd;
        epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&ev);

}

static void delete_event(int epollfd,int fd,int state)
{
        struct epoll_event ev;
        ev.events = state;
        ev.data.fd = fd;
        epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,&ev);

}

static void modify_event(int epollfd,int fd,int state)
{
        struct epoll_event ev;
        ev.events = state;
        ev.data.fd = fd;
        epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&ev);

}
#endif

#ifndef My_Epoll
#define My_Epoll
static void add_event(int epollfd,int fd,int state)
{
        struct epoll_event ev;
        ev.events = state;
        ev.data.fd = fd;
        int ret = epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&ev);
        if (ret < 0) perror("add_events");

}

static void delete_event(int epollfd,int fd,int state)
{
        struct epoll_event ev;
        ev.events = state;
        ev.data.fd = fd;
        int ret = epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,&ev);
        if (ret < 0) perror("delete_event");

}

static void modify_event(int epollfd,int fd,int state)
{
        struct epoll_event ev;
        ev.events = state;
        ev.data.fd = fd;
        int ret = epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&ev);
        if (ret < 0) perror("modify_events");

}
#endif

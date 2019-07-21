#ifndef MySock
#define MySock
int get_listen_socket(char *ip, int port) {
    int listen_socket;
    if ((listen_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket() error");
        return -1;
    }
    struct sockaddr_in my_addr;
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = inet_addr(ip);
    if (bind(listen_socket, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0) {
        perror("bind");
        return -1;
    }
    if (listen(listen_socket, 20) < 0) {
        perror("listen");
        return -1;
    }
    return listen_socket;
}

int  accept_clnt(int listen_socket) {
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    printf("正在accept\n");
    int clnt_socket = accept(listen_socket, (struct sockaddr *)&client_addr, &len);
    printf("accept over\n");
    if(clnt_socket < 0) {
        perror("accept");
        return -1;
    }
    getpeername(clnt_socket, (struct sockaddr *)&client_addr, &len);
    printf("<%s> : Login    fd = %d \n",inet_ntoa(client_addr.sin_addr), clnt_socket);
    return clnt_socket;
}
#endif

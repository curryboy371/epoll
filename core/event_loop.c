#include "event_loop.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>

static int listen_fd = -1;
static int epoll_fd = -1;

int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int event_loop_init(int port) {

    // server listen socket 생성
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    set_nonblocking(listen_fd);

    // listen socket 주소 설정
    struct sockaddr_in serv_addr = {0};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    // bind and listen
    bind(listen_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    listen(listen_fd, SOMAXCONN);

    // epoll 인스턴스 생성
    epoll_fd = epoll_create1(0);

    // epoll에 서버 리슨 소켓 등록
    // 리슨 소켓은 클라이언트 접속 요청이 들어왔는가만 감시하면 됨
    // 또한 리슨 소켓은 1:1 구조가 아니기 때문에 동시에 여러 클라이언트가 접속할 가능성이 있음
    // 따라서 동시 발생시 계속해서 리슨 소켓으로 이벤트를 발생시키기 위해 EPOLLIN 사용

    struct epoll_event ev = { .events = EPOLLIN, .data.fd = listen_fd };
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev);

    printf("Server running on port %d\n", port);
    return 0;
}

int event_loop_register(int fd) {
    struct epoll_event ev = { .events = EPOLLIN | EPOLLET, .data.fd = fd };
    return epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev);
}

int event_loop_unregister(int fd) {
    return epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
}

int event_loop_wait(struct epoll_event* events, int max_events) {
    return epoll_wait(epoll_fd, events, max_events, -1);
}

int get_listen_fd() { return listen_fd; }
int get_epoll_fd() { return epoll_fd; }

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

    // 상태가 계속 준비(Ready)이면 계속해서 Evnet가 발생
    // listen socket은 event 발생시 한번에 하나의 client socket을 accept
    // 버퍼에 남을 수도 있는 다른 클라이언트 fd는 다음 이벤트에서 처리
    struct epoll_event ev = { .events = EPOLLIN, .data.fd = listen_fd }; // Level Triggered
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev); // 

    printf("Server running on port %d\n", port);
    return 0;
}

int event_loop_register(int fd) {

    // 상태가 READY로 변경될 때 딱 한 번만 이벤트를 발생
    // 클라이언트 소켓 read는 loop에서 수신 버퍼가 비어질 때까지 계속 read ( syscall 호출 최소화를 위함 )
    struct epoll_event ev = { .events = EPOLLIN | EPOLLET, .data.fd = fd }; // Edge Triggered
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

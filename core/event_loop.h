#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

#include "define.h"
#include <sys/epoll.h>

int event_loop_init(int port);
int event_loop_wait(struct epoll_event* events, int max_events);
int event_loop_register(int fd);
int event_loop_unregister(int fd);
int set_nonblocking(int fd);

int get_listen_fd();
int get_epoll_fd();

#endif // EVENT_LOOP_H
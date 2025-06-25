#include "define.h"
#include "event_loop.h"
#include "connection_manager.h"
#include "thread_manager.h"

#include "task/system_task.h"
#include "task/worker_task.h"


#include "server_context.h"
#include "system_broadcast.h"
#include "db_manager.h"

#define DB_URI "mongodb://172.19.192.1:27017"
#define DB_NAME "epoll"

#define PORT 9000
#define MAX_EVENTS 10

// 워커는 4개, 시스템은 1개
#define WORKER_THREAD_COUNT  4
#define SYSTEM_THREAD_COUNT  1

ServerContext server_ctx;

void init_server() {
    
    // 초기화
    session_init(&server_ctx.session);
    db_init(&server_ctx.db, DB_URI, DB_NAME);
    user_manager_init(&server_ctx.user);

    // task queue 초기화
    system_task_init();
    worker_task_init();


    // thread start
    thread_start(WORKER_THREAD_COUNT, worker_task_main);
    thread_start(SYSTEM_THREAD_COUNT, system_task_main);


    // epoll init
    event_loop_init(PORT);
}

int main() {

    struct epoll_event events[MAX_EVENTS];

    init_server();

    while (TRUE) {
        // 발생한 epoll 이벤트만큼 반복
        int event_count = event_loop_wait(events, MAX_EVENTS);
        for (int i = 0; i < event_count; i++) {

            int fd = events[i].data.fd;
            if (fd == get_listen_fd())  {
                // 새로운 클라이언트 accept - event fd가 listen socket인 경우 
                connection_handle_accept();
                //system_broadcast_notice("hi hi hi\n");
                
            }   
            else {
                // 기존 클라이언트에서 event가 발생한 경우
                connection_handle_read(fd);
            }                            
        }
    }
}

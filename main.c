#include "define.h"
#include "event_loop.h"
#include "connection_manager.h"
#include "thread_manager.h"

#include "task/system_task.h"
#include "task/worker_task.h"


#include "server_context.h"
#include "system_broadcast.h"
#include "db_manager.h"

#include <signal.h>

//#define DB_URI_BACKUP "mongodb://172.17.208.1:27017"
#define DB_URI "mongodb://10.10.16.8:27017"
#define DB_URI_BACKUP "mongodb://172.19.192.1:27017"
#define DB_NAME "epoll"

#define PORT 9000
#define MAX_EVENTS 10

// 워커는 4개, 시스템은 1개
#define WORKER_THREAD_COUNT  4
#define SYSTEM_THREAD_COUNT  1

ServerContext server_ctx;

// 종료 시그널 핸들러
void handle_shutdown(int sig) {
    printf("Signal %d received. Shutting down server gracefully...\n", sig);

    stop_flag = 1;

    event_loop_unregister(get_listen_fd());
    close(get_listen_fd());
    
    // 모든 대기중 스레드 깨우기
    worker_task_awaik();
    system_task_awaik();

    //sleep(1); // 잠시 대기


    system_task_release();
    worker_task_release();

    user_manager_release(&server_ctx.user);
    session_release(&server_ctx.session);
    db_release(&server_ctx.db);
    driver_manager_release(&server_ctx.driver);

    printf("Signal %d finished\n", sig);

}   

void init_server() {
    
    // 시그널 등록
    signal(SIGINT, handle_shutdown);
    signal(SIGTERM, handle_shutdown);
    signal(SIGKILL, handle_shutdown);

    // 초기화
    session_init(&server_ctx.session);
    db_init(&server_ctx.db, DB_URI, DB_NAME);
    driver_manager_init(&server_ctx.driver);
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

    while (!stop_flag) {
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

#include "connection_manager.h"

#include "server_context.h"

#include "event_loop.h"
#include "recv_buffer.h"
#include "thread_manager.h"

#include "task/task_queue.h"
#include "task/worker_task.h"


#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>


void connection_handle_accept() {

    // 클라이언트 소켓 accept

    // epoll에 클라이언트 소켓 등록
    // 클라이언트 소켓의 이벤트튼 한 클라이언트에게서만 받음
    // 동시에 발생한다면, 한번에 받아 경계를 나눠주면 됨
    // EPOLLET : Edge-Triggered 모드

    int listen_fd = get_listen_fd();
    int client_fd = accept(listen_fd, NULL, NULL);
    set_nonblocking(client_fd);

    
    event_loop_register(client_fd);
    session_add(&server_ctx.session, client_fd);

    printf("New client fd=%d\n", client_fd);
}

void connection_handle_read(int client_fd) {

    while (TRUE) {

        // 클라 전송 data read
        uint8_t tmp_buf[BUFFER_SIZE];
        int n = read(client_fd, tmp_buf, sizeof(tmp_buf));
        if (n > 0) {
            // 읽은 데이터를 클라이언트마다 가지고 있는 누적 버퍼에 추가
            Session* session = session_get(&server_ctx.session, client_fd);
            if(!session) {
                // 세션 없음
                break;
            }

            recv_buffer_append(&session->recv_buffer, tmp_buf, n);

            while (1) {
                uint8_t packet[BUFFER_SIZE];
                size_t packet_len = 0;

                // 버퍼에서 패킷이 완성되는가 체크
                ParseResult ret = recv_buffer_extract_packet(&session->recv_buffer, packet, &packet_len);
                if (ret == PARSE_COMPLETE) {
                    // 패킷 완성, 스레드로 Task 전달
                    Task task;
                    task.target_fd = client_fd;
                    memcpy(task.data, packet, packet_len);
                    task.len = packet_len;
                    worker_task_enqueue(task);
                } 
                else if (ret == PARSE_INCOMPLETE) { 
                    break; 
                }
                else {
                    printf("Invalid packet from fd=%d\n", client_fd);
                    close(client_fd);
                    return;
                }
            }
        } else if (n == 0) {
            // 종료 처리
            user_manager_logout(&server_ctx.user, client_fd);
            session_remove(&server_ctx.session, client_fd);

            printf("Client disconnected fd=%d\n", client_fd);
            close(client_fd);
            break;
        } else {
            if (errno != EAGAIN)
                perror("read error");
            break;
        }
    }
}


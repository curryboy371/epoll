#include "packet_dispatcher.h"
#include "packet.h"

#include "packet_handler.h"

#include <string.h>
#include <arpa/inet.h>

// worker가 받은 packet을 dispatch
// 들어오는 패킷을 파싱 및 역직렬화한 후 패킷 내용 처리
void worker_dispatcher(int client_fd, const uint8_t* data, size_t len) {
    
    if (len < 4) return; // 최소 패킷 ( lenth(2) + cmd(2))

    // header 파싱 ( len, cmd )
    uint16_t net_len, net_cmd;
    memcpy(&net_len, data, 2);
    memcpy(&net_cmd, data + 2, 2);
    uint16_t total_len = ntohs(net_len);
    uint16_t cmd = ntohs(net_cmd);

    // 길이 체크
    if (total_len != len) {
        // log?
        return;   
    }
    printf("receive packet cmd %d\n", cmd);
    // dispatcher
    switch(cmd) {
        case CMD_CHAT_MESSAGE:
            handle_chat_message(client_fd, data + PACKET_HEADER_SIZE, len - PACKET_HEADER_SIZE);
            break;

        case CMD_LOGIN_REQUEST:
            handle_login_request(client_fd, data + PACKET_HEADER_SIZE, len - PACKET_HEADER_SIZE);
            break;

        case CMD_LOGIN_RESPONSE:
            handle_login_response(client_fd, data + PACKET_HEADER_SIZE, len - PACKET_HEADER_SIZE);
            break;

        case CMD_JOIN_REQUEST:
            handle_join_request(client_fd, data + PACKET_HEADER_SIZE, len - PACKET_HEADER_SIZE);
            break;
        
        case CMD_ADMIN_BROADCAST:
            handle_admin_message(client_fd, data + PACKET_HEADER_SIZE, len - PACKET_HEADER_SIZE);
            break;
        
        default:
            printf("invaild packet cmd %d (%d)\n", cmd, client_fd);
            break;
    }
}


// system task가 전송할 packet을 dispatch
// 보낼 패킷을 파싱 및 역직렬화한 후 패킷 내용 처리
void system_dispatcher(int client_fd, const uint8_t* data, size_t len) {
    if (len < 4) return; // 최소 패킷 ( lenth(2) + cmd(2))

    // header 파싱 ( len, cmd )
    uint16_t net_len, net_cmd;
    memcpy(&net_len, data, 2);
    memcpy(&net_cmd, data + 2, 2);
    uint16_t total_len = ntohs(net_len);
    uint16_t cmd = ntohs(net_cmd);

    // 길이 체크
    if (total_len != len) {
        // log?
        return;   
    }

    // dispatcher
    switch(cmd) {
        case CMD_CHAT_MESSAGE:
            handle_chat_message(client_fd, data + PACKET_HEADER_SIZE, len - PACKET_HEADER_SIZE);
            break;

        case CMD_LOGIN_REQUEST:
            //handle_login_request(client_fd, data + PACKET_HEADER_SIZE, len - PACKET_HEADER_SIZE);
            break;

        case CMD_ADMIN_BROADCAST:
            //handle_login_request(client_fd, data + PACKET_HEADER_SIZE, len - PACKET_HEADER_SIZE);
            break;


        default:
            printf("invaild packet cmd %d (%d)\n", cmd, client_fd);
            break;
    }
}
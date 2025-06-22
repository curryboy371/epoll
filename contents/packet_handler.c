#include "packet_handler.h"

#include "server_context.h"

#include "packet_sender.h"
#include "packet.h"
#include "chat.pb-c.h"
#include "admin.pb-c.h"
#include "login.pb-c.h"
#include "join.pb-c.h"

void handle_chat_message(int client_fd, const uint8_t* body, size_t body_len) {

    // 역직렬화
    Chat__ChatMessage* msg = chat__chat_message__unpack(NULL, body_len, body);
    if (!msg) {
        printf("Protobuf unpack fail\n");
        return;
    }

    printf("[Chat] %s: %s\n", msg->name, msg->message);

    // broadcast
    Chat__ChatMessage chat_msg = CHAT__CHAT_MESSAGE__INIT;
    chat_msg.name = msg->name;
    chat_msg.message = msg->message;
    broadcast_user_packet(CMD_CHAT_MESSAGE, &chat_msg);


    // 메모리 정리
    chat__chat_message__free_unpacked(msg, NULL);

}


void handle_login_request(int client_fd, const uint8_t* body, size_t body_len) {

    Login__LoginRequest* msg = login__login_request__unpack(NULL, body_len, body);
    if (!msg) {
        printf("Protobuf unpack fail\n");
        return;
    }

    // char password_buf[64];
    // if(db_find_user(&server_ctx.db, msg->id, msg->password, sizeof(password_buf))) {
    //     printf("success id :%s, pw : %s \n", msg->id, password_buf);
    // }
    // else {
    //     printf("failed id :%s, pw : %s \n", msg->id, msg->password);
    // }

    // 메모리 해제
    login__login_request__free_unpacked(msg, NULL);
}

void handle_login_response(int client_fd, const uint8_t* body, size_t body_len) {

}

void handle_join_request(int client_fd, const uint8_t* body, size_t body_len) {

    Join__JoinRequest* msg = join__join_request__unpack(NULL, body_len, body);
    if (!msg) {
        printf("Protobuf unpack fail\n");
        return;
    }

    Join__JoinResponse join_msg = JOIN__JOIN_RESPONSE__INIT;
    // 값 검증 
    if (!msg->id || strlen(msg->id) == 0 ||
        !msg->password || strlen(msg->password) == 0 ||
        !msg->name || strlen(msg->name) == 0)
    {
        join_msg.success = FALSE;
        join_msg.message = "Invalid input (empty id/password/name)";
        send_packet(client_fd, CMD_JOIN_RESPONSE, &join_msg);
        join__join_request__free_unpacked(msg, NULL);
        return;
    }

    // user mgr
    if (user_manager_add(&server_ctx.user, msg->id, msg->password, msg->name) == FALSE) {
        join_msg.success = FALSE;
        join_msg.message = "User Max...";
        send_packet(client_fd, CMD_JOIN_RESPONSE, &join_msg);
        join__join_request__free_unpacked(msg, NULL);
        return;
    }

    if (user_manager_login(&server_ctx.user, msg->id, msg->password, client_fd) == FALSE) {
        join_msg.success = FALSE;
        join_msg.message = "Faild user login";
        send_packet(client_fd, CMD_JOIN_RESPONSE, &join_msg);
        join__join_request__free_unpacked(msg, NULL);
        return;
    }

    join_msg.success = TRUE;
    send_packet(client_fd, CMD_JOIN_RESPONSE, &join_msg);

    // 입장
    Chat__ChatMessage chat_msg = CHAT__CHAT_MESSAGE__INIT;
    chat_msg.name = msg->name;
    chat_msg.message = "님이 입장하였음";
    broadcast_user_packet(CMD_CHAT_MESSAGE, &chat_msg);

    // // 2. 중복 검사 및 DB insert
    // if (db_insert_user(&server_ctx.db, msg->id, msg->password, msg->name)) {
    //     join_msg.success = TRUE;
    //     send_packet(client_fd, CMD_JOIN_RESPONSE, &join_msg);
    // }
    // else {
    //     join_msg.success = FALSE;
    //     join_msg.message = "ID duplication";
    //     send_packet(client_fd, CMD_JOIN_RESPONSE, &join_msg);
    // }
    // 메모리 해제
    join__join_request__free_unpacked(msg, NULL);
}

void handle_admin_message(int client_fd, const uint8_t* body, size_t body_len) {

    // 역직렬화
    Admin__AdminMessage* msg = admin__admin_message__unpack(NULL, body_len, body);
    if (!msg) {
        printf("Protobuf unpack fail\n");
        return;
    }

    printf("[Admin] Message: %s\n", msg->message);

    // 메모리 해제
    admin__admin_message__free_unpacked(msg, NULL);


    // test
    Chat__ChatMessage chat_msg = CHAT__CHAT_MESSAGE__INIT;
    chat_msg.name = "kekek";
    chat_msg.message = "nice to meet you!!!";
    send_packet(client_fd, CMD_CHAT_MESSAGE, &chat_msg);
    
}
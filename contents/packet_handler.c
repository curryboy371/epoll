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

    printf("handle_login_request\n");

    Login__LoginRequest* msg = login__login_request__unpack(NULL, body_len, body);
    if (!msg) {
        printf("Protobuf unpack fail\n");
        return;
    }

    Login__LoginResponse response = LOGIN__LOGIN_RESPONSE__INIT;
    response.sender = NULL;
    response.users = NULL;
    response.n_users = 0;

    User_Data user;
    if(db_find_user_by_pw(&server_ctx.db, msg->id, msg->password, &user) == FALSE) {
        printf("failed id :%s, pw : %s \n", msg->id, msg->password);
        response.success = FALSE;
    }
    else {

        // user add
        if (user_manager_add(&server_ctx.user, msg->id, msg->password, user.user_name, client_fd) == FALSE) {
            printf("접속중인 유저 %s", msg->id);
            response.success = FALSE;
        }
        else {

            response.success = TRUE;

            printf("1\n");
            //ChatCommon__User sender = CHAT_COMMON__USER__INIT;

            // sender 설정
            response.sender = malloc(sizeof(ChatCommon__User));
            chat_common__user__init(response.sender);
            response.sender->id = strdup(user.user_id);
            response.sender->name = strdup(user.user_name);

            // sender.id = user.user_id;
            // sender.name = user.user_name;
            // response.sender = &sender;


            int user_len = 0;
            User* user_list = user_manager_get_all(&server_ctx.user, &user_len);
            if(user_list) {

                response.n_users = user_len;
                response.users = malloc(sizeof(ChatCommon__User*) * user_len);

                for (int i = 0; i < user_len; i++) {
                    response.users[i] = malloc(sizeof(ChatCommon__User));
                    chat_common__user__init(response.users[i]);
                    response.users[i]->id = strdup(user_list[i].id);
                    response.users[i]->name = strdup(user_list[i].name);
                }
            }
        }
    }

    printf("2\n");

    send_packet(client_fd, CMD_LOGIN_RESPONSE, &response);

    printf("3\n");

    // 메모리 정리
    if (response.users) {
        for (int i = 0; i < response.n_users; i++) {
            if (response.users[i]) {
                free(response.users[i]->id);
                free(response.users[i]->name);
                free(response.users[i]);
            }
        }
        free(response.users);
    }

    if (response.sender) {
        free(response.sender->id);
        free(response.sender->name);
        free(response.sender);
    }

    printf("4\n");

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

    Join__JoinResponse response = JOIN__JOIN_RESPONSE__INIT;
    response.n_users = 0;

    // 값 검증 
    if (!msg->id || strlen(msg->id) == 0 ||
        !msg->password || strlen(msg->password) == 0 ||
        !msg->name || strlen(msg->name) == 0)
    {
        response.success = FALSE;
        response.message = "Invalid input (empty id/password/name)";
        send_packet(client_fd, CMD_JOIN_RESPONSE, &response);
        join__join_request__free_unpacked(msg, NULL);
        return;
    }

     // 2. 중복 검사 및 DB insert
     if (db_insert_user(&server_ctx.db, msg->id, msg->password, msg->name) == FALSE) {
         response.success = FALSE;
         response.message = "ID duplication";
         send_packet(client_fd, CMD_JOIN_RESPONSE, &response);
         return;
     }

    // user mgr
    if (user_manager_add(&server_ctx.user, msg->id, msg->password, msg->name, client_fd) == FALSE) {
        response.success = FALSE;
        response.message = "Faild login...";
        send_packet(client_fd, CMD_JOIN_RESPONSE, &response);
        join__join_request__free_unpacked(msg, NULL);
        return;
    }

    int user_len = 0;
    User* user_list = user_manager_get_all(&server_ctx.user, &user_len);
    if(user_list) {

        response.n_users = user_len;
        response.users = malloc(sizeof(ChatCommon__User*) * response.n_users);
        
        for (size_t i = 0; i < response.n_users; i++) {
            response.users[i] = malloc(sizeof(ChatCommon__User));
            chat_common__user__init(response.users[i]);

            // 널문자 user_list에 이미 존재함
            response.users[i]->id = strdup(user_list[i].id);
            response.users[i]->name = strdup(user_list[i].name);
        }
    }

    response.success = TRUE;

    send_packet(client_fd, CMD_JOIN_RESPONSE, &response);

    // 입장 메시지
    Chat__ChatMessage chat_msg = CHAT__CHAT_MESSAGE__INIT;
    chat_msg.name = msg->name;
    chat_msg.message = "님이 입장하였음";
    broadcast_user_packet_exept(CMD_CHAT_MESSAGE, &chat_msg, client_fd); // 자기 자신은 입장메시지 필요 없음


    // 메모리 정리
    for (size_t i = 0; i < response.n_users; i++) {
        free(response.users[i]->id);
        free(response.users[i]->name);
        free(response.users[i]);
    }

    if(response.users) {
        free(response.users);
    }

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
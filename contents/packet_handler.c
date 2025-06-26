#include "packet_handler.h"

#include "server_context.h"

#include "packet_sender.h"
#include "packet.h"
#include "chat.pb-c.h"
#include "admin.pb-c.h"
#include "login.pb-c.h"
#include "join.pb-c.h"

#include "leave.pb-c.h"


Common__User* create_chat_user(User* user_info)
{
    Common__User* user = malloc(sizeof(Common__User));
    common__user__init(user);

    if (user_info->id) {
        user->id = malloc(strlen(user_info->id) + 1);
        strcpy(user->id, user_info->id);
    } else {
        user->id = NULL;
    }

    if (user_info->name) {
        user->name = malloc(strlen(user_info->name) + 1);
        strcpy(user->name, user_info->name);
    } else {
        user->name = NULL;
    }

    user->uid = user_info->uid;
   
    return user;
}

Common__User* create_chat_user_by_id_name(const char* id, const char* name)
{
    Common__User* user = malloc(sizeof(Common__User));
    if (!user)
        return NULL;

    common__user__init(user);

    // id 복사
    if (id) {
        size_t id_len = strlen(id) + 1;
        user->id = malloc(id_len);
        if (user->id) {
            memcpy(user->id, id, id_len);
        }
    } 
    else {
        user->id = NULL;
    }

    // name 복사
    if (name) {
        size_t name_len = strlen(name) + 1;
        user->name = malloc(name_len);
        if (user->name) {
            memcpy(user->name, name, name_len);
        }
    } 
    else {
        user->name = NULL;
    }

    return user;
}

char* alloc_and_copy_string(const char* src)
{
    if (src == NULL) return NULL;

    size_t len = strlen(src) + 1;
    char* dest = malloc(len);
    if (dest)
        memcpy(dest, src, len);

    return dest;
}


void handle_chat_message(int client_fd, const uint8_t* body, size_t body_len) {

    // 역직렬화
    Chat__ChatMessage* msg = chat__chat_message__unpack(NULL, body_len, body);
    if (!msg) {
        printf("Protobuf unpack fail\n");
        return;
    }

    // broadcast
    Chat__ChatMessage chat_msg = CHAT__CHAT_MESSAGE__INIT;
    chat_msg.name = msg->name;
    chat_msg.message = msg->message;
    broadcast_user_packet(CMD_CHAT_MESSAGE, &chat_msg);

    // 메모리 정리
    chat__chat_message__free_unpacked(msg, NULL);
}

void handle_chat_command(int client_fd, const uint8_t* body, size_t body_len) {

    // 역직렬화
    Chat__ChatCommand* msg = chat__chat_command__unpack(NULL, body_len, body);
    if (!msg) {
        printf("Protobuf unpack fail\n");
        return;
    }

    // 여기서 command
    size_t cmd_len = strlen(msg->message);
    if(strncmp(msg->message, CMD_BMP, cmd_len)) { // bmp180

        
    }
    else if(strncmp(msg->message, CMD_BH, cmd_len)) {

    }

    // broadcast
    Chat__ChatMessage chat_msg = CHAT__CHAT_MESSAGE__INIT;
    chat_msg.name = CMD_NAME;
    chat_msg.message = "wait...";
    broadcast_user_packet(CMD_CHAT_MESSAGE, &chat_msg);

    // 메모리 정리
    chat__chat_command__free_unpacked(msg, NULL);
}


void handle_login_request(int client_fd, const uint8_t* body, size_t body_len) {

    Login__LoginRequest* msg = login__login_request__unpack(NULL, body_len, body);
    if (!msg) {
        printf("Protobuf unpack fail\n");
        return;
    }

    bool success = FALSE;

    Login__LoginResponse* response = malloc(sizeof(Login__LoginResponse));
    login__login_response__init(response);
    response->success = FALSE;

    User client_user;
    if (db_find_user_by_pw(&server_ctx.db, msg->id, msg->password, &client_user)) {
        if (user_manager_add(&server_ctx.user, &client_user, client_fd)) {

            response->success = TRUE;

            response->sender = create_chat_user(&client_user);

            // users 리스트 설정
            int user_len = 0;
            User** user_list = user_manager_get_all(&server_ctx.user, &user_len);

            response->n_users = user_len;

            response->users = malloc(sizeof(Common__User*) * user_len);

            for (int i = 0; i < user_len; i++) {

                response->users[i] = create_chat_user(user_list[i]);
            }

            free(user_list);
        }
        else {
            response->message = alloc_and_copy_string("중벅 로그인..");
        }
    }
    else{
        response->message = alloc_and_copy_string("db 인증 실패..");
    }
    
    // send_packet
    send_packet(client_fd, CMD_LOGIN_RESPONSE, response);

    // 입장 메시지
    if(response->success) {

        // join한 사람 제외, join user 뿌려줌
        send_join_notice(&client_user, client_fd);
    }


    login__login_response__free_unpacked(response, NULL);
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

    Join__JoinResponse* response = malloc(sizeof(Join__JoinResponse));
    join__join_response__init(response);
    response->success = FALSE;

    // 값 검증 
    if (!msg->id || strlen(msg->id) == 0 ||
        !msg->password || strlen(msg->password) == 0 ||
        !msg->name || strlen(msg->name) == 0)
    {
        response->message = alloc_and_copy_string("Invalid input (empty id/password/name)");
        send_packet(client_fd, CMD_JOIN_RESPONSE, response);
        join__join_response__free_unpacked(response, NULL);
        join__join_request__free_unpacked(msg, NULL);
        return;
    }

     // 2. 중복 검사 및 DB insert
     if (db_insert_user(&server_ctx.db, msg->id, msg->password, msg->name) == FALSE) {

        response->message = alloc_and_copy_string("ID duplication");
        send_packet(client_fd, CMD_JOIN_RESPONSE, response);
        join__join_response__free_unpacked(response, NULL);
        join__join_request__free_unpacked(msg, NULL);
        return;
     }

    // user mgr
    User find_user;
    if (db_find_user_by_pw(&server_ctx.db, msg->id, msg->password, &find_user) == FALSE){
        response->message = alloc_and_copy_string("User Find Failed");
        send_packet(client_fd, CMD_JOIN_RESPONSE, response);
        join__join_response__free_unpacked(response, NULL);
        join__join_request__free_unpacked(msg, NULL);
        return;
    }

    if (user_manager_add(&server_ctx.user, &find_user, client_fd) == FALSE) {
        response->message = alloc_and_copy_string("Faild login");
        send_packet(client_fd, CMD_JOIN_RESPONSE, response);
        join__join_response__free_unpacked(response, NULL);
        join__join_request__free_unpacked(msg, NULL);
        return;
    }

    // sender 정보 전달
    response->sender = create_chat_user_by_id_name(msg->id, msg->name);

    // 접속 유저 리스트 전달
    int user_len = 0;
    User** user_list = user_manager_get_all(&server_ctx.user, &user_len);

    response->n_users = user_len;
    response->users = malloc(sizeof(Common__User*) * user_len);

    for (int i = 0; i < user_len; i++) {
        response->users[i] = create_chat_user(user_list[i]);
    }

    free(user_list);

    response->success = TRUE;
    send_packet(client_fd, CMD_JOIN_RESPONSE, response);

    if(response->success) {
        // join한 사람 제외, join user 뿌려줌
        send_join_notice(&find_user, client_fd);
    }

    // 메모리 해제
    join__join_response__free_unpacked(response, NULL);
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


void handle_change_name_request(int client_fd, const uint8_t* body, size_t body_len)
{
    Chat__ChangeNameRequest* req = chat__change_name_request__unpack(NULL, body_len, body);
    if (!req) {
        printf("Protobuf unpack fail (ChangeNameRequest)\n");
        return;
    }

    // 유저 찾기
    User* user = user_manager_find_by_session(&server_ctx.user, client_fd);
    if (!user) {
        printf("User not found for session %d\n", client_fd);
        chat__change_name_request__free_unpacked(req, NULL);
        return;
    }

    // 닉네임 변경 전 기존 이름 백업
    char old_name[MAX_NAME_LEN];
    strncpy(old_name, user->name, sizeof(old_name) - 1);
    old_name[sizeof(old_name) - 1] = '\0';

    // DB Name 변경
    if (db_update_user_name(&server_ctx.db, user->uid, req->new_name) == FALSE) {
        printf("change name db failed %d\n", client_fd);
        chat__change_name_request__free_unpacked(req, NULL);
        return;
    }

    // 닉네임 변경
    strncpy(user->name, req->new_name, sizeof(user->name) - 1);
    user->name[sizeof(user->name) - 1] = '\0';

    // 응답 전송
    Chat__ChangeNameResponse resp = CHAT__CHANGE_NAME_RESPONSE__INIT;
    resp.success = TRUE;
    resp.new_name = user->name;

    send_packet(client_fd, CMD_CHANGE_NAME_RESPONSE, &resp);

    // 닉네임 변경 브로드캐스트 알림
    Chat__ChangeNameNotice notice = CHAT__CHANGE_NAME_NOTICE__INIT;
    Common__User sender = COMMON__USER__INIT;
    sender.uid = user->uid;
    sender.id = user->id;
    sender.name = user->name;
    notice.sender = &sender;
    notice.old_name = old_name;

    notice.success = TRUE;

    broadcast_user_packet(CMD_CHANGE_NAME_NOTIFY, &notice);

    chat__change_name_request__free_unpacked(req, NULL);
}


///////////////////////////////////////////////////////////////////

void send_join_notice(User* user_info, int exept_fd)
{
    Join__JoinNotice notify = JOIN__JOIN_NOTICE__INIT;
    notify.success = TRUE;

    Common__User sender = COMMON__USER__INIT;
    sender.uid = user_info->uid;
    sender.id = user_info->id;
    sender.name = user_info->name;

    notify.sender = &sender;

    broadcast_user_packet_exept(CMD_JOIN_NOTIFY, &notify, exept_fd);
}

void send_leave_notify(User* user_info)
{
    Leave__LeaveNotice  notify = LEAVE__LEAVE_NOTICE__INIT;

    notify.success = TRUE;


    Common__User sender = COMMON__USER__INIT;
    sender.uid = user_info->uid;
    sender.id = user_info->id;
    sender.name = user_info->name;
    notify.sender = &sender;

    broadcast_user_packet(CMD_LEAVE_NOTIFY, &notify);
}
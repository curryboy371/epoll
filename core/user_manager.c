#include "user_manager.h"
#include "system_broadcast.h"
#include <sys/socket.h>
void user_manager_init(UserInfo* info) {
    info->count = 0;
    pthread_mutex_init(&info->mutex, NULL);
}

Boolean user_manager_add(UserInfo* info, const char* id, const char* pw, const char* name, const int session_fd) {

    pthread_mutex_lock(&info->mutex);

    // 유저 초과
    if (info->count >= MAX_USER) {
        pthread_mutex_unlock(&info->mutex);
        return FALSE; 
    }

    // 중복 id 검사
    int check_count = 0;
    for (int i = 0; i < MAX_USER; i++) {

        // 유효하지 않은 유저
        if(info->users[i].session_fd != -1) {
            continue;
        }

        if (strcmp(info->users[i].id, id) == 0) {
            pthread_mutex_unlock(&info->mutex);
            return FALSE; // 중복
        }

        check_count++;

        if(info->count == check_count) {
            break;
        }
    }


    // 먼저 빈 슬롯 찾기
    for (int i = 0; i < info->count; i++) {
        if (info->users[i].session_fd == -1) {
            User* user = &info->users[i];
            strncpy(user->id, id, sizeof(user->id) - 1);
            user->id[sizeof(user->id) - 1] = '\0';

            strncpy(user->password, pw, sizeof(user->password) - 1);
            user->password[sizeof(user->password) - 1] = '\0';

            strncpy(user->name, name, sizeof(user->name) - 1);
            user->name[sizeof(user->name) - 1] = '\0';

            user->session_fd = session_fd;
            info->count++;

            pthread_mutex_unlock(&info->mutex);
            return TRUE;
        }
    }

    User* user = &info->users[info->count];
    strncpy(user->id, id, sizeof(user->id) - 1);
    user->id[sizeof(user->id) - 1] = '\0';

    strncpy(user->password, pw, sizeof(user->password) - 1);
    user->password[sizeof(user->password) - 1] = '\0';

    strncpy(user->name, name, sizeof(user->name) - 1);
    user->name[sizeof(user->name) - 1] = '\0';

    user->session_fd = session_fd; 

    info->count++;

    pthread_mutex_unlock(&info->mutex);
    return TRUE;
}

User* user_manager_find(UserInfo* info, const char* id) {
    for (int i = 0; i < info->count; i++) {
        if (strcmp(info->users[i].id, id) == 0) {
            return &info->users[i];
        }
    }
    return NULL;
}

User* user_manager_get_all(UserInfo* info, int* out_count) {
    *out_count = info->count;
    return info->users;
}

void user_manager_logout(UserInfo* info, int session_fd) {

    char* username = NULL;

    pthread_mutex_lock(&info->mutex);

    for (int i = 0; i < info->count; i++) {
        if (info->users[i].session_fd == session_fd) {
            info->users[i].session_fd= -1;
            username = info->users[i].name;
            break;
        }
    }

    pthread_mutex_unlock(&info->mutex);


    // 퇴장메시지
    if(username) {
        char msg[128];
        sprintf(msg, "%s 님이 퇴장", username);
        system_broadcast_notice(msg);
    }
}

void user_manager_broadcast(UserInfo* info, SessionInfo* session_info, const uint8_t* data, size_t len) {
    for (int i = 0; i < info->count; i++) {
        User* user = &info->users[i];

        printf("user name %s\n", user->name);

        if (user->session_fd != -1) {

            // 세션이 실제로 살아있는지 추가 확인
            Session* session = session_get(session_info, user->session_fd);
            if (session) {
                send(user->session_fd, data, len, 0);
            }
        }
    }
}

void user_manager_broadcast_except(UserInfo* info, const int except_fd, SessionInfo* session_info, const uint8_t* data, size_t len) {
    for (int i = 0; i < info->count; i++) {
        User* user = &info->users[i];
        if (user->session_fd != -1 && user->session_fd != except_fd) {
            // 세션이 실제로 살아있는지 추가 확인
            Session* session = session_get(session_info, user->session_fd);
            if (session) {
                send(user->session_fd, data, len, 0);
            }
        }
    }
}
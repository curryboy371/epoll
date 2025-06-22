#include "user_manager.h"
#include "system_broadcast.h"
#include <sys/socket.h>
void user_manager_init(UserInfo* info) {
    info->count = 0;
    pthread_mutex_init(&info->mutex, NULL);
}

Boolean user_manager_add(UserInfo* info, const char* id, const char* pw, const char* name) {
    // 중복 ID 검사
    for (int i = 0; i < info->count; i++) {
        if (strcmp(info->users[i].id, id) == 0) {
            return FALSE; // 중복
        }
    }

    if (info->count >= MAX_USER) {
        return FALSE; // 용량 초과
    }

    pthread_mutex_lock(&info->mutex);
    User* user = &info->users[info->count];
    strncpy(user->id, id, sizeof(user->id));
    strncpy(user->password, pw, sizeof(user->password));
    strncpy(user->name, name, sizeof(user->name));
    user->session_fd = -1;  // 아직 접속 안 됨

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

Boolean user_manager_login(UserInfo* info, const char* id, const char* pw, int session_fd) {

    pthread_mutex_lock(&info->mutex);

    User* user = user_manager_find(info, id);
    if (!user) {
        pthread_mutex_unlock(&info->mutex);
        return FALSE;
    }

    if (strcmp(user->password, pw) != 0) {
        pthread_mutex_unlock(&info->mutex);
        return FALSE;
    }

    user->session_fd = session_fd;
    pthread_mutex_unlock(&info->mutex);

    return TRUE;
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
        if (user->session_fd != -1) {

            // 세션이 실제로 살아있는지 추가 확인
            Session* session = session_get(session_info, user->session_fd);
            if (session) {
                send(user->session_fd, data, len, 0);
            }
        }
    }
}
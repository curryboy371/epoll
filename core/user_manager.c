#include "user_manager.h"
#include "system_broadcast.h"
#include <sys/socket.h>

#include "server_context.h"
#include "task/system_task.h"



#include "packet_handler.h"

void user_manager_init(UserInfo* info) {
    info->count = 0;
    info->user_map = NULL;

    pthread_mutex_init(&info->mutex, NULL);
}

Boolean user_manager_add(UserInfo* info, User* user_data, const int session_fd) {

    
    pthread_mutex_lock(&info->mutex);

    // 유저 초과
    if (info->count >= MAX_USER) {
        pthread_mutex_unlock(&info->mutex);
        return FALSE; 
    }

    // 중복 uid
    User* found = NULL;
    HASH_FIND_INT(info->user_map, &user_data->uid, found);
    if (found) {
        pthread_mutex_unlock(&info->mutex);
        return FALSE;
    }

    User* new_user = malloc(sizeof(User));
    if (!new_user) {
        pthread_mutex_unlock(&info->mutex);
        return FALSE;
    }

    new_user->uid = user_data->uid;

    strncpy(new_user->id, user_data->id, sizeof(new_user->id) - 1);
    new_user->id[sizeof(new_user->id) - 1] = '\0';

    strncpy(new_user->password, user_data->password, sizeof(new_user->password) - 1);
    new_user->password[sizeof(new_user->password) - 1] = '\0';

    strncpy(new_user->name, user_data->name, sizeof(new_user->name) - 1);
    new_user->name[sizeof(new_user->name) - 1] = '\0';

    new_user->session_fd = session_fd;

    HASH_ADD_INT(info->user_map, uid, new_user);

    info->count++;
    pthread_mutex_unlock(&info->mutex);
    return TRUE;
}

User* user_manager_find(UserInfo* info, const char* id) 
{
    pthread_mutex_lock(&info->mutex);
    
    User* user = NULL;
    HASH_FIND_STR(info->user_map, id, user);
    
    pthread_mutex_unlock(&info->mutex);
    return user;
}

User* user_manager_find_by_session(UserInfo* info, int session_fd)
{
    pthread_mutex_lock(&info->mutex);

    User* user = NULL;
    User* tmp;

    HASH_ITER(hh, info->user_map, user, tmp) {
        if (user->session_fd == session_fd) {
            pthread_mutex_unlock(&info->mutex);
            return user;
        }
    }

    pthread_mutex_unlock(&info->mutex);
    return NULL;
}

User* user_manager_find_by_uid(UserInfo* info, int uid) 
{
    pthread_mutex_lock(&info->mutex);
    
    User* user = NULL;
    HASH_FIND_INT(info->user_map, &uid, user);
    
    pthread_mutex_unlock(&info->mutex);
    return user;
}


User** user_manager_get_all(UserInfo* info, int* out_count)
{
    pthread_mutex_lock(&info->mutex);

    int count = HASH_COUNT(info->user_map);
    User** user_array = malloc(sizeof(User*) * count);
    if (!user_array) {
        pthread_mutex_unlock(&info->mutex);
        *out_count = 0;
        return NULL;
    }

    int idx = 0;
    User* user;
    User* tmp;
    HASH_ITER(hh, info->user_map, user, tmp) {
        user_array[idx++] = user; // shallow copy (포인터만)
    }

    *out_count = count;
    pthread_mutex_unlock(&info->mutex);
    return user_array;
}

User** user_manager_get_all_unsafe(UserInfo* info, int* out_count)
{
    int count = HASH_COUNT(info->user_map);
    User** user_array = malloc(sizeof(User*) * count);
    if (!user_array) {
        *out_count = 0;
        return NULL;
    }

    int idx = 0;
    User* user;
    User* tmp;
    HASH_ITER(hh, info->user_map, user, tmp) {
        user_array[idx++] = user;
    }

    *out_count = count;
    return user_array;
}


void user_manager_logout(UserInfo* info, int session_fd) {

    User backup_user;
    Boolean found = FALSE;

    pthread_mutex_lock(&info->mutex);

    User* user;
    User* tmp;

    HASH_ITER(hh, info->user_map, user, tmp) {
        if (user->session_fd == session_fd) {

            backup_user.uid = user->uid;
            strncpy(backup_user.id, user->id, sizeof(backup_user.id) - 1);
            strncpy(backup_user.name, user->name, sizeof(backup_user.name) - 1);

            // 해시에서 삭제
            HASH_DEL(info->user_map, user);
            free(user);
            info->count--;
            found = TRUE;
            break;
        }
    }


    pthread_mutex_unlock(&info->mutex);

    if(found) {
        send_leave_notify(&backup_user);
    }

}

void user_manager_release(UserInfo* info) {

    pthread_mutex_lock(&info->mutex);

    User* user;
    User* tmp;

    HASH_ITER(hh, info->user_map, user, tmp) {
        HASH_DEL(info->user_map, user);
        free(user);
    }

    info->count = 0;
    pthread_mutex_unlock(&info->mutex);

    pthread_mutex_destroy(&info->mutex);
}

void user_manager_broadcast(UserInfo* info, SessionInfo* session_info, const uint8_t* data, size_t len) {

    User** user_list = NULL;
    int user_len = 0;

    // snapshot broadcasting
    pthread_mutex_lock(&info->mutex);
    user_list = user_manager_get_all_unsafe(info, &user_len);
    pthread_mutex_unlock(&info->mutex);

    for (int i = 0; i < user_len; i++) {
        User* user = user_list[i];
        if (user->session_fd != -1) {
            Session* session = session_get(session_info, user->session_fd);
            if (session && session->fd != -1) {

                // 바로 send 하지 말고 system task로 enqueue
                Task task;
                task.target_fd = session->fd;
                memcpy(task.data, data, len);
                task.len = len;

                system_task_enqueue(task);
            }
        }
    }

    free(user_list);

}

void user_manager_broadcast_except(UserInfo* info, const int except_fd, SessionInfo* session_info, const uint8_t* data, size_t len) {

    User** user_list = NULL;
    int user_len = 0;

    // snapshot broadcasting
    pthread_mutex_lock(&info->mutex);
    user_list = user_manager_get_all_unsafe(info, &user_len);
    pthread_mutex_unlock(&info->mutex);

    for (int i = 0; i < user_len; i++) {
        User* user = user_list[i];
        if (user->session_fd != -1 && user->session_fd != except_fd) {
            Session* session = session_get(session_info, user->session_fd);
            if (session && session->fd != -1) {

                Task task;
                task.target_fd = user->session_fd;
                memcpy(task.data, data, len);
                task.len = len;
                system_task_enqueue(task);
            }
        }
    }

    free(user_list);
}
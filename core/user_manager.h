
#ifndef USER_MANAGER_H
#define USER_MANAGER_H


#include "define.h"
#include "session_manager.h"
#include "uthash.h"

#include <pthread.h>

#define MAX_USER 1024


// TODO 동적할당 방식으로
typedef struct {
    pthread_mutex_t mutex;  
    User*user_map;
    int count;
} UserInfo;

void user_manager_init(UserInfo* info);
Boolean user_manager_add(UserInfo* info, User* user, const int session_fd);
User* user_manager_find_by_uid(UserInfo* info, int uid);

User* user_manager_find(UserInfo* info, const char* id);
User* user_manager_find_by_session(UserInfo* info, int session_fd);

User** user_manager_get_all(UserInfo* info, int* out_count);
User** user_manager_get_all_unsafe(UserInfo* info, int* out_count);

void user_manager_logout(UserInfo* info, int session_fd);

void user_manager_broadcast(UserInfo* info, SessionInfo* session_info, const uint8_t* data, size_t len);
void user_manager_broadcast_except(UserInfo* info, const int except_fd, SessionInfo* session_info, const uint8_t* data, size_t len);

#endif // USER_MANAGER_H
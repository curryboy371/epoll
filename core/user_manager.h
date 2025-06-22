
#ifndef USER_MANAGER_H
#define USER_MANAGER_H


#include "define.h"
#include "session_manager.h"
#include <pthread.h>

#define MAX_USER 1024

typedef struct {
    char id[64];
    char password[64];
    char name[64];
    int session_fd;  // 연결된 클라이언트 fd (미연결시 -1)
} User;

// TODO 동적할당 방식으로
typedef struct {
    pthread_mutex_t mutex;  
    User users[MAX_USER];
    int count;
} UserInfo;

void user_manager_init(UserInfo* info);
Boolean user_manager_add(UserInfo* info, const char* id, const char* pw, const char* name);
User* user_manager_find(UserInfo* info, const char* id);
Boolean user_manager_login(UserInfo* info, const char* id, const char* pw, int session_fd);
void user_manager_logout(UserInfo* info, int session_fd);

void user_manager_broadcast(UserInfo* info, SessionInfo* session_info, const uint8_t* data, size_t len);

#endif // USER_MANAGER_H
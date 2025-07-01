#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

#include "define.h"
#include "recv_buffer.h"
#include <pthread.h>

#define MAX_SESSIONS 1024

typedef struct {
    int fd;
    RecvBuffer recv_buffer;
    int active;
} Session;

// TODO 동적할당 방식으로
typedef struct {
    pthread_mutex_t mutex;  
    Session sessions[MAX_SESSIONS];
} SessionInfo;


void session_init(SessionInfo* info);
void session_release(SessionInfo* info);
int session_add(SessionInfo* info, int fd);
void session_remove(SessionInfo* info, int fd);


Session* session_get(SessionInfo* info, int fd);
int session_send(SessionInfo* info, int client_fd, const uint8_t* data, size_t len);
void session_broadcast(SessionInfo* info, const uint8_t* data, size_t len);


#endif // SESSION_MANAGER_H
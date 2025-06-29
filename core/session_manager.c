#include "session_manager.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>

void session_init(SessionInfo* info) {
    memset(info->sessions, 0, sizeof(info->sessions));
    pthread_mutex_init(&info->mutex, NULL);
}

int session_add(SessionInfo* info, int fd) {

    if (fd >= MAX_SESSIONS) {
        return -1;
    }

    pthread_mutex_lock(&info->mutex);

    Session* session = &info->sessions[fd];
    session->fd = fd;
    session->active = 1;
    recv_buffer_init(&session->recv_buffer);

    pthread_mutex_unlock(&info->mutex);
    return 0;
}

void session_remove(SessionInfo* info, int fd) {
    if (fd >= MAX_SESSIONS) {
        return;
    }
    pthread_mutex_lock(&info->mutex);
    Session* session = &info->sessions[fd];
    if (session->active) {
        close(session->fd);
        session->active = 0;
    }
    pthread_mutex_unlock(&info->mutex);
    
    printf("Client disconnected fd=%d\n", fd);
    close(fd);
}

Session* session_get(SessionInfo* info, int fd) {
    if (fd >= MAX_SESSIONS) {
        return NULL;
    }

    Session* session = &info->sessions[fd];
    if (!session->active) {
        return NULL;
    }
    return session;
}

int session_send(SessionInfo* info, int client_fd, const uint8_t* data, size_t len) {
    Session* session = session_get(info, client_fd);
    if (!session) {
        return -1;
    }

    if( session->fd == -1) {
        return -1;
    }

    int sent = send(client_fd, data, len, 0);

    printf("send %d \n", len);
    return (sent == len) ? 0 : -1;
}

void session_broadcast(SessionInfo* info, const uint8_t* data, size_t len) {
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (info->sessions[i].active) {

            if( info->sessions[i].fd == -1) {
                return;
            }

            send(info->sessions[i].fd, data, len, 0);
        }
    }
}
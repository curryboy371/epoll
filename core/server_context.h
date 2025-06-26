#ifndef SERVER_CONTEXT_H
#define SERVER_CONTEXT_H

#include "define.h"
#include "db_manager.h"
#include "user_manager.h"
#include "driver_manager.h"
#include "session_manager.h"
#include "task/task_queue.h"

typedef struct {
    DBInfo db;
    UserInfo user;
    SessionInfo session;
    DriverInfo driver;
    TaskQueue system_queue;
    TaskQueue worker_queue;
} ServerContext;


// 선언만
extern ServerContext server_ctx;

#endif // SERVER_CONTEXT_H
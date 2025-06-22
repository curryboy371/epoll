#include "system_task.h"

#include "server_context.h"

#include <pthread.h>

#include <sys/socket.h>

#include "packet_dispatcher.h"


//static TaskQueue system_queue;
static pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;

void system_task_init() {

    

    task_queue_init(&server_ctx.system_queue);
}

void system_task_enqueue(Task task) {
    pthread_mutex_lock(&queue_mutex);

    if (!task_queue_enqueue(&server_ctx.system_queue, task)) {
        printf("System queue full!\n");
    }

    pthread_cond_signal(&queue_cond);
    pthread_mutex_unlock(&queue_mutex);
}

Boolean system_task_dequeue(Task* out_task) {
    pthread_mutex_lock(&queue_mutex);
    while (task_queue_is_empty(&server_ctx.system_queue)) {
        pthread_cond_wait(&queue_cond, &queue_mutex);

        if( task_queue_dequeue(&server_ctx.system_queue, out_task) == FALSE) {
            continue;
        }
    }

    pthread_mutex_unlock(&queue_mutex);


    return TRUE;
}

void* system_task_main(void* arg) {

    while (TRUE) {
        pthread_mutex_lock(&queue_mutex);

        while (TRUE) {
            Task task;
            if (task_queue_dequeue(&server_ctx.system_queue, &task)) {
                pthread_mutex_unlock(&queue_mutex);
                worker_dispatcher(task.target_fd, (uint8_t*)task.data, task.len);
                break;
            }
            pthread_cond_wait(&queue_cond, &queue_mutex);
        }

    }
    return NULL;
}
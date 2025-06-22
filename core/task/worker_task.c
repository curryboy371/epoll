#include "worker_task.h"

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "server_context.h"
#include "packet_dispatcher.h"


// 내부 큐 및 락
//static TaskQueue worker_queue;
static pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;

void worker_task_init() {
    
    task_queue_init(&server_ctx.worker_queue);
}

void worker_task_enqueue(Task task) {
    pthread_mutex_lock(&queue_mutex);

    if (!task_queue_enqueue(&server_ctx.worker_queue, task)) {
        printf("Worker queue full!\n");
    }

    pthread_cond_signal(&queue_cond);
    pthread_mutex_unlock(&queue_mutex);
}

void* worker_task_main(void* arg) {

    while (TRUE) {
        pthread_mutex_lock(&queue_mutex);

        while (TRUE) {
            Task task;
            if (task_queue_dequeue(&server_ctx.worker_queue, &task)) {
                pthread_mutex_unlock(&queue_mutex);
                worker_dispatcher(task.target_fd, (uint8_t*)task.data, task.len);
                break;
            }
            pthread_cond_wait(&queue_cond, &queue_mutex);
        }
    }
    return NULL;
}
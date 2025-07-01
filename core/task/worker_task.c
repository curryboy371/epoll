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

void worker_task_release() {

    pthread_cond_broadcast(&queue_cond); // 모든 wait 깨우기


    pthread_mutex_lock(&queue_mutex);
    task_queue_release(&server_ctx.worker_queue);
    pthread_mutex_unlock(&queue_mutex);

    pthread_mutex_destroy(&queue_mutex);
    pthread_cond_destroy(&queue_cond);
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

    while (stop_flag) {
        pthread_mutex_lock(&queue_mutex);

        while (stop_flag) {
            Task task;
            if (task_queue_dequeue(&server_ctx.worker_queue, &task)) {
                pthread_mutex_unlock(&queue_mutex);
                worker_dispatcher(task.target_fd, (uint8_t*)task.data, task.len);
                break;
            }

            // wait 내부에서 queue_mutex를 unlock 하고, signal 받을 때 다시 lock 후 리턴
            pthread_cond_wait(&queue_cond, &queue_mutex);
        }
    }
    return NULL;
}

void worker_task_awaik() {
    pthread_mutex_lock(&queue_mutex);
    pthread_cond_broadcast(&queue_cond); // 대기중인 스레드 깨우기
    pthread_mutex_unlock(&queue_mutex);
}
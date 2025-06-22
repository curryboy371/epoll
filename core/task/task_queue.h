#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include "define.h"

#define TASK_QUEUE_SIZE 128

typedef struct {
    int target_fd;
    char data[BUFFER_SIZE];
    size_t len;
} Task;

typedef struct {
    Task queue[TASK_QUEUE_SIZE];
    int front;
    int rear;
} TaskQueue;

void task_queue_init(TaskQueue* q);
Boolean task_queue_is_empty(TaskQueue* q);
Boolean task_queue_is_full(TaskQueue* q);
Boolean task_queue_enqueue(TaskQueue* q, Task task);
Boolean task_queue_dequeue(TaskQueue* q, Task* out_task);

#endif // TASK_QUEUE_H
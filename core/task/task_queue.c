#include "task_queue.h"

void task_queue_init(TaskQueue* q) {
    q->front = q->rear = 0;
}

void task_queue_release(TaskQueue* q) {
    
    q->front = q->rear = 0;
}

Boolean task_queue_is_empty(TaskQueue* q) {
    return q->front == q->rear;
}

Boolean task_queue_is_full(TaskQueue* q) {
    return ((q->rear + 1) % TASK_QUEUE_SIZE) == q->front;
}

Boolean task_queue_enqueue(TaskQueue* q, Task task) {
    if (task_queue_is_full(q)) {
        return FALSE;
    }
    q->queue[q->rear] = task;
    q->rear = (q->rear + 1) % TASK_QUEUE_SIZE;
    return TRUE;
}

Boolean task_queue_dequeue(TaskQueue* q, Task* out_task) {
    if (task_queue_is_empty(q)) {
        return FALSE;
    }
    *out_task = q->queue[q->front];
    q->front = (q->front + 1) % TASK_QUEUE_SIZE;
    return TRUE;
}
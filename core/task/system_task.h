#ifndef SYSTEM_TASK_H
#define SYSTEM_TASK_H

#include "define.h"
#include "task_queue.h"

// 초기화
void system_task_init();

// enqueue
void system_task_enqueue(Task task);

// dequeue (내부 전용)
Boolean system_task_dequeue(Task* out_task);


void* system_task_main(void* arg);

#endif // SYSTEM_TASK_H

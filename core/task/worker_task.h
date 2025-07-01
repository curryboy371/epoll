#ifndef WORKER_TASK_H
#define WORKER_TASK_H

#include "define.h"
#include "task_queue.h"

void worker_task_init();
void worker_task_release();
void worker_task_enqueue(Task task);
void* worker_task_main(void* arg);

void worker_task_awaik();

#endif // WORKER_TASK_H
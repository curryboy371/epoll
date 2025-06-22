#ifndef THREAD_MANAGER_H
#define THREAD_MANAGER_H

#include "define.h"

typedef void* (*ThreadMain)(void*);

void thread_start(int num_threads, ThreadMain entry);


#endif // THREAD_MANAGER_H
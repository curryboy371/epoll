#include "thread_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>


#define MAX_THREADS 5

static pthread_t threads[MAX_THREADS];
static int thread_count = 0;

void thread_start(int num_threads, ThreadMain entry) {
    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&threads[thread_count], NULL, entry, NULL) != 0) {
            perror("pthread_create");
            exit(1);
        }
        pthread_detach(threads[thread_count]); // 자동 회수
        thread_count++;
    }
}
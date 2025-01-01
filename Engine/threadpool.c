#include "threadpool.h"
#include <stdlib.h>
#include <stdio.h>

// Worker thread function
static void* ThreadPoolWorker(void* arg) {
    ThreadPool* pool = (ThreadPool*)arg;

    while (true) {
        ThreadPoolTask task;

        // Lock the queue to retrieve a task
        pthread_mutex_lock(&pool->queueMutex);

        // Wait until the queue is not empty or stop signal is received
        while (pool->taskCount == 0 && !pool->stop) {
            pthread_cond_wait(&pool->queueNotEmpty, &pool->queueMutex);
        }

        if (pool->stop) {
            pthread_mutex_unlock(&pool->queueMutex);
            break;
        }

        // Get the task from the queue
        task = pool->taskQueue[pool->queueFront];
        pool->queueFront = (pool->queueFront + 1) % pool->queueSize;
        pool->taskCount--;

        // Signal that there is space in the queue
        pthread_cond_signal(&pool->queueNotFull);
        pthread_mutex_unlock(&pool->queueMutex);

        // Execute the task
        task.function(task.argument);
    }

    return NULL;
}

// Create and initialize a thread pool
ThreadPool* ThreadPoolCreate(int threadCount, int queueSize) {
    ThreadPool* pool = safe_malloc(sizeof(ThreadPool));
    pool->threadCount = threadCount;
    pool->queueSize = queueSize;
    pool->queueFront = 0;
    pool->queueBack = 0;
    pool->taskCount = 0;
    pool->stop = false;

    pool->threads = safe_malloc(threadCount * sizeof(pthread_t));
    pool->taskQueue = safe_malloc(queueSize * sizeof(ThreadPoolTask));

    pthread_mutex_init(&pool->queueMutex, NULL);
    pthread_cond_init(&pool->queueNotEmpty, NULL);
    pthread_cond_init(&pool->queueNotFull, NULL);

    // Create worker threads
    for (int i = 0; i < threadCount; i++) {
        pthread_create(&pool->threads[i], NULL, ThreadPoolWorker, pool);
    }

    return pool;
}

// Add a task to the thread pool
bool ThreadPoolAddTask(ThreadPool* pool, void (*function)(void*), void* argument) {
    pthread_mutex_lock(&pool->queueMutex);

    // Wait if the queue is full
    while (pool->taskCount == pool->queueSize && !pool->stop) {
        pthread_cond_wait(&pool->queueNotFull, &pool->queueMutex);
    }

    if (pool->stop) {
        pthread_mutex_unlock(&pool->queueMutex);
        return false;
    }

    // Add the task to the queue
    pool->taskQueue[pool->queueBack].function = function;
    pool->taskQueue[pool->queueBack].argument = argument;
    pool->queueBack = (pool->queueBack + 1) % pool->queueSize;
    pool->taskCount++;

    pthread_cond_signal(&pool->queueNotEmpty);
    pthread_mutex_unlock(&pool->queueMutex);

    return true;
}

// Destroy the thread pool and cleanup resources
void ThreadPoolDestroy(ThreadPool* pool) {
    pthread_mutex_lock(&pool->queueMutex);
    pool->stop = true;
    pthread_cond_broadcast(&pool->queueNotEmpty);
    pthread_mutex_unlock(&pool->queueMutex);

    // Join all threads
    for (int i = 0; i < pool->threadCount; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    SAFE_FREE(pool->threads);
    SAFE_FREE(pool->taskQueue);

    pthread_mutex_destroy(&pool->queueMutex);
    pthread_cond_destroy(&pool->queueNotEmpty);
    pthread_cond_destroy(&pool->queueNotFull);

    SAFE_FREE(pool);
}

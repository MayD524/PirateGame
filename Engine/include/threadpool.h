#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <extended_memory.h>
#include <pthread.h>
#include <stdbool.h>

// Task structure
typedef struct {
    void (*function)(void*); // Function pointer for the task
    void* argument;          // Argument for the task
} ThreadPoolTask;

// Thread pool structure
typedef struct {
    pthread_t* threads;              // Array of threads
    int threadCount;                 // Number of threads
    ThreadPoolTask* taskQueue;       // Circular task queue
    int queueSize;                   // Maximum size of the task queue
    int queueFront;                  // Front index of the queue
    int queueBack;                   // Back index of the queue
    int taskCount;                   // Number of tasks in the queue
    pthread_mutex_t queueMutex;      // Mutex for queue synchronization
    pthread_cond_t queueNotEmpty;    // Condition variable for non-empty queue
    pthread_cond_t queueNotFull;     // Condition variable for non-full queue
    bool stop;                       // Flag to signal thread termination
} ThreadPool;

// Create and initialize a thread pool
ThreadPool* ThreadPoolCreate(int threadCount, int queueSize);

// Add a task to the thread pool
bool ThreadPoolAddTask(ThreadPool* pool, void (*function)(void*), void* argument);

// Destroy the thread pool and cleanup resources
void ThreadPoolDestroy(ThreadPool* pool);

#endif // THREADPOOL_H

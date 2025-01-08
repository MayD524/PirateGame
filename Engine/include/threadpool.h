#ifndef THREADPOOL_H
#define THREADPOOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <extended_memory.h>
#include <logger.h>
#include <pthread.h>
#include <time.h>
#include <stddef.h>

#define DEFAULT_THREADPOOL_SIZE 6
#define KILL_TIMEOUT_SEC 5

/* Forward declaration so we can reference in function pointers */
typedef struct threadpool_event threadpool_event_t;

/* Task function signature */
typedef void (*thread_func_t)(void* arg, threadpool_event_t* event);

/* Optional callback */
typedef void (*task_callback_t)(void* result);

/* Priority levels */
typedef enum {
    TASK_LOW = 0,
    TASK_MEDIUM,
    TASK_HIGH,
    TASK_PRIORITY_COUNT
} task_priority_t;

/* The threadpool_event structure */
struct threadpool_event {
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    int             tasks_remaining;
    int             shutdown;
};

/* Task structure */
typedef struct threadpool_task {
    thread_func_t       function;
    void*               argument;
    task_priority_t     priority;
    int                 cancelled;
    time_t              execute_at;
    time_t              repeat_interval;
    task_callback_t     callback;
    struct threadpool_task* next;
} threadpool_task_t;

/* FIFO queue for immediate tasks */
typedef struct task_queue {
    threadpool_task_t* head;
    threadpool_task_t* tail;
} task_queue_t;

/* Sorted linked list for delayed tasks */
typedef struct delayed_task_queue {
    threadpool_task_t* head;
} delayed_task_queue_t;

/* Main thread pool structure */
typedef struct threadpool {
    pthread_mutex_t lock;
    pthread_cond_t  notify;
    pthread_t*      threads;
    int             thread_count;
    int             started;

    task_queue_t        priority_queues[TASK_PRIORITY_COUNT];
    delayed_task_queue_t delayed_queue;

    int workers_to_remove;
    int shutdown;

    /* Event for waiting on tasks to complete */
    threadpool_event_t event;
} threadpool_t;

/* API */

threadpool_t* threadpool_create(int num_threads);
void threadpool_destroy(threadpool_t* pool);

int threadpool_add_task_ex(
    threadpool_t*   pool,
    thread_func_t   func,
    void*           arg,
    task_priority_t priority,
    time_t          delay,
    time_t          repeat_interval,
    task_callback_t callback
);

int threadpool_cancel_task(threadpool_t* pool, threadpool_task_t* task);
void threadpool_wait_all(threadpool_t* pool);

int threadpool_increase_threads(threadpool_t* pool, int count);
int threadpool_decrease_threads(threadpool_t* pool, int count);

/**
 * Prints detailed debug info about the thread pool:
 *  - The number of threads started
 *  - The number of tasks remaining
 *  - The tasks in each priority queue
 *  - The tasks in the delayed queue
 */
void threadpool_debug_dump(threadpool_t* pool);

#ifdef __cplusplus
}
#endif

#endif /* THREADPOOL_H */

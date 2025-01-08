#include "threadpool.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <errno.h>
#include <signal.h>

#define CLOCK_MONOTONIC 1

typedef struct {
    pthread_t  target_thread;  /* The worker thread we want to join. */
    int        joined;         /* 1 if joined successfully, else 0. */
    int        join_result;    /* Return code from pthread_join (0 if normal). */
} joiner_args_t;

/* The thread function that calls pthread_join on behalf of the caller. */
static void* joiner_thread_func(void* arg) {
    joiner_args_t* ja = (joiner_args_t*)arg;
    ja->join_result = pthread_join(ja->target_thread, NULL);
    ja->joined      = 1;
    return NULL;
}

/**
 * custom_pthread_timed_join - Attempt to join `thread` within `timeout_sec` seconds.
 * If the thread does not exit in time, forcibly cancel it.
 * 
 * @param thread       The worker thread we want to join.
 * @param timeout_sec  The maximum time to wait (in seconds).
 * @return 0 if joined successfully before the timeout, 1 if we had to cancel, or <0 on error.
 */
static int custom_pthread_timed_join(pthread_t thread, unsigned timeout_sec) {
    joiner_args_t ja;
    memset(&ja, 0, sizeof(joiner_args_t));
    ja.target_thread = thread;
    
    /* Create a short-lived joiner thread. */
    pthread_t joiner;
    if (pthread_create(&joiner, NULL, joiner_thread_func, &ja) != 0) {
        perror("custom_pthread_timed_join: pthread_create failed");
        return -1;
    }

    /* Start timing. */
    struct timespec start_ts, now_ts;
    clock_gettime(CLOCK_MONOTONIC, &start_ts);

    int canceled = 0;
    while (1) {
        /* Check if the joiner has finished. */
        if (ja.joined) {
            /* The worker has been joined. */
            break;
        }
        /* Check elapsed time. */
        clock_gettime(CLOCK_MONOTONIC, &now_ts);
        time_t elapsed = now_ts.tv_sec - start_ts.tv_sec;
        if (elapsed >= (time_t)timeout_sec) {
            /* We timed out. Cancel the worker thread forcibly. */
            fprintf(stderr, "[threadpool] custom_pthread_timed_join: cancelling thread.\n");
            pthread_cancel(thread);
            canceled = 1;
            break;
        }
        /* Sleep briefly to avoid busy-waiting. */
        struct timespec short_sleep = {0, 10000000L}; /* 10 ms */
        nanosleep(&short_sleep, NULL);
    }

    /* Regardless of whether we canceled or it ended, we must join the joiner. */
    pthread_join(joiner, NULL);

    if (!ja.joined && canceled) {
        /* The joiner might or might not have succeeded after we canceled.
           Typically, pthread_join will return some error if the thread was canceled. */
        return 1; /* Indicate we had to cancel. */
    }

    /* If we get here, either we joined in time or there's an error code from pthread_join. */
    if (canceled) {
        return 1;
    } else {
        return 0;
    }
}


/* Forward declarations of internal helpers */
static void*              threadpool_worker(void* arg);
static void               enqueue_task(task_queue_t* queue, threadpool_task_t* task);
static threadpool_task_t* dequeue_task(task_queue_t* queue);
static void               enqueue_delayed(delayed_task_queue_t* dqueue, threadpool_task_t* task);
static threadpool_task_t* pop_ready_delayed(delayed_task_queue_t* dqueue);

/* A special sentinel task for removing threads gracefully */
static threadpool_task_t  g_exit_task = {0};

/* 
 * Global pointer used by the signal handler to destroy the pool.
 * Use with caution; in real applications, you might prefer to do this in main
 * or a dedicated shutdown routine.
 */
static threadpool_t* g_signal_pool = NULL;
static struct sigaction old_int;
static struct sigaction old_term;


static void threadpool_signal_handler(int signum) {
    if (g_signal_pool) {
        fprintf(stderr, "[threadpool] Caught signal %d, destroying pool...\n", signum);
        threadpool_destroy(g_signal_pool);
        g_signal_pool = NULL; 
    }
    
    /* Restore the old handler so the process terminates the usual way. */
    if (signum == SIGINT) {
        sigaction(SIGINT, &old_int, NULL);
    } else if (signum == SIGTERM) {
        sigaction(SIGTERM, &old_term, NULL);
    }
    /* Re-raise the signal so the default behavior can continue (exit). */
    raise(signum);
}

/* -------------------------------------------------------------------------
 * threadpool_create
 * ------------------------------------------------------------------------- */
static void install_signal_handlers() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = threadpool_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    /* SIGINT (Ctrl+C) */
    sigaction(SIGINT, NULL, &old_int);
    sigaction(SIGINT, &sa, NULL);

    /* SIGTERM (kill <pid>) */
    sigaction(SIGTERM, NULL, &old_term);
    sigaction(SIGTERM, &sa, NULL);
}

/* --------------------
 * threadpool_create
 * -------------------- */
threadpool_t* threadpool_create(int num_threads) {
    if (num_threads <= 0) {
        fprintf(stderr, "threadpool_create: invalid num_threads=%d\n", num_threads);
        return NULL;
    }

    threadpool_t* pool = (threadpool_t*)calloc(1, sizeof(threadpool_t));
    if (!pool) {
        perror("threadpool_create: calloc failed");
        return NULL;
    }

    /* Initialize locks and conditions */
    if (pthread_mutex_init(&pool->lock, NULL) != 0) {
        perror("pthread_mutex_init (pool->lock)");
        SAFE_FREE(pool);
        return NULL;
    }
    if (pthread_cond_init(&pool->notify, NULL) != 0) {
        perror("pthread_cond_init (pool->notify)");
        pthread_mutex_destroy(&pool->lock);
        SAFE_FREE(pool);
        return NULL;
    }

    /* Initialize event */
    if (pthread_mutex_init(&pool->event.mutex, NULL) != 0) {
        perror("pthread_mutex_init (pool->event.mutex)");
        pthread_cond_destroy(&pool->notify);
        pthread_mutex_destroy(&pool->lock);
        SAFE_FREE(pool);
        return NULL;
    }
    if (pthread_cond_init(&pool->event.cond, NULL) != 0) {
        perror("pthread_cond_init (pool->event.cond)");
        pthread_mutex_destroy(&pool->event.mutex);
        pthread_cond_destroy(&pool->notify);
        pthread_mutex_destroy(&pool->lock);
        SAFE_FREE(pool);
        return NULL;
    }
    pool->event.tasks_remaining = 0;
    pool->event.shutdown        = 0;

    pool->thread_count     = num_threads;
    pool->started          = 0;
    pool->workers_to_remove= 0;
    pool->shutdown         = 0;

    for (int p = 0; p < TASK_PRIORITY_COUNT; ++p) {
        pool->priority_queues[p].head = NULL;
        pool->priority_queues[p].tail = NULL;
    }
    pool->delayed_queue.head = NULL;

    /* Allocate the worker threads array */
    pool->threads = (pthread_t*)safe_malloc(sizeof(pthread_t) * num_threads);
    if (!pool->threads) {
        perror("safe_malloc (pool->threads)");
        pthread_mutex_destroy(&pool->event.mutex);
        pthread_cond_destroy(&pool->event.cond);
        pthread_cond_destroy(&pool->notify);
        pthread_mutex_destroy(&pool->lock);
        SAFE_FREE(pool);
        return NULL;
    }

    /* Create worker threads */
    for (int i = 0; i < num_threads; ++i) {
        if (pthread_create(&pool->threads[i], NULL, threadpool_worker, pool) != 0) {
            perror("pthread_create");
            pthread_mutex_lock(&pool->lock);
            pool->shutdown = 1;
            pthread_cond_broadcast(&pool->notify);
            pthread_mutex_unlock(&pool->lock);

            /* join the threads started so far */
            for (int j = 0; j < i; ++j) {
                pthread_join(pool->threads[j], NULL);
            }
            SAFE_FREE(pool->threads);
            pthread_mutex_destroy(&pool->event.mutex);
            pthread_cond_destroy(&pool->event.cond);
            pthread_cond_destroy(&pool->notify);
            pthread_mutex_destroy(&pool->lock);
            SAFE_FREE(pool);
            return NULL;
        }
        pool->started++;
    }

    /* 
     * ========== SIGNAL HANDLER INTEGRATION ==========
     * If you'd like the pool to "kill threads" on SIGINT / SIGTERM,
     * we install our handlers here. The handlers call threadpool_destroy
     * on a global pointer 'g_signal_pool'.
     */
    g_signal_pool = pool;  /* So the signal handler can destroy it */
    install_signal_handlers();

    return pool;
}

/* -------------------------------------------------------------------------
 * threadpool_destroy
 * ------------------------------------------------------------------------- */
void threadpool_destroy(threadpool_t* pool) {
    if (!pool) return;

    /*  Signal shutdown */
    pthread_mutex_lock(&pool->lock);
    pool->shutdown = 1;
    pool->event.shutdown = 1;  
    /* Wake all threads so they can exit */
    pthread_cond_broadcast(&pool->notify);
    pthread_mutex_unlock(&pool->lock);

    /*  Attempt to join each thread with a timeout */
    struct timespec start_ts, now_ts;
    clock_gettime(CLOCK_MONOTONIC, &start_ts);

    for (int i = 0; i < pool->started; i++) {
        /* We will poll join in a loop with short sleeps until the timeout passes. */

        while (1) {
            int ret = custom_pthread_timed_join(pool->threads[i], KILL_TIMEOUT_SEC);
            if (ret == 0) {
                /* Joined successfully, break out of the loop. */
                break;
            } else if (ret == EBUSY) {
                /* Thread is still running; check how long we've waited. */
                clock_gettime(CLOCK_MONOTONIC, &now_ts);
                time_t elapsed = now_ts.tv_sec - start_ts.tv_sec;
                if (elapsed >= KILL_TIMEOUT_SEC) {
                    /* We've exceeded the kill_timeout_sec. Cancel the thread. */
                    fprintf(stderr, "Threadpool destroy: forcibly canceling thread %d\n", i);
                    pthread_cancel(pool->threads[i]);
                    /* Now we do a normal join to clean up the canceled thread. */
                    pthread_join(pool->threads[i], NULL);
                    break;
                } else {
                    /* Sleep briefly before retrying. This reduces busy-wait CPU usage. */
                    struct timespec short_sleep = {0, 10000000L}; /* 10 ms */
                    nanosleep(&short_sleep, NULL);
                }
            } else {
                /* Some other error from custom_pthread_timed_join (e.g., ESRCH if thread doesn't exist). */
                fprintf(stderr, "custom_pthread_timed_join error (thread %d), code: %d\n", i, ret);
                break;
            }
        }
    }

    /*  Clean up resources in the pool */
    SAFE_FREE(pool->threads);

    /* Cleanup any tasks in the queues, destroy mutexes and cond vars, etc. */
    /* (You'll need to do this similarly to your existing cleanup code...) */

    pthread_mutex_destroy(&pool->event.mutex);
    pthread_cond_destroy(&pool->event.cond);

    pthread_cond_destroy(&pool->notify);
    pthread_mutex_destroy(&pool->lock);

    SAFE_FREE(pool);
}


/* -------------------------------------------------------------------------
 * threadpool_add_task_ex
 * ------------------------------------------------------------------------- */
int threadpool_add_task_ex(
    threadpool_t*   pool,
    thread_func_t   func,
    void*           arg,
    task_priority_t priority,
    time_t          delay,
    time_t          repeat_interval,
    task_callback_t callback
) {
    if (!pool || !func) {
        fprintf(stderr, "threadpool_add_task_ex: invalid arguments\n");
        return -1;
    }
    if (priority < TASK_LOW || priority >= TASK_PRIORITY_COUNT) {
        fprintf(stderr, "threadpool_add_task_ex: invalid priority\n");
        return -1;
    }

    /* Allocate the new task */
    threadpool_task_t* task = (threadpool_task_t*)calloc(1, sizeof(threadpool_task_t));
    if (!task) {
        perror("threadpool_add_task_ex: calloc");
        return -1;
    }

    task->function        = func;
    task->argument        = arg;
    task->priority        = priority;
    task->cancelled       = 0;
    task->execute_at      = (delay > 0) ? time(NULL) + delay : 0;
    task->repeat_interval = repeat_interval;
    task->callback        = callback;
    task->next            = NULL;

    pthread_mutex_lock(&pool->lock);

    /* If there's a delay, we enqueue into the delayed queue; otherwise, immediate */
    if (delay > 0) {
        enqueue_delayed(&pool->delayed_queue, task);
    } else {
        enqueue_task(&pool->priority_queues[priority], task);
        pthread_cond_signal(&pool->notify);
    }

    /* Increment tasks_remaining in the event structure */
    pthread_mutex_lock(&pool->event.mutex);
    pool->event.tasks_remaining++;
    pthread_mutex_unlock(&pool->event.mutex);

    pthread_mutex_unlock(&pool->lock);
    return 0;
}

/* -------------------------------------------------------------------------
 * threadpool_cancel_task
 * ------------------------------------------------------------------------- */
int threadpool_cancel_task(threadpool_t* pool, threadpool_task_t* task) {
    if (!pool || !task) {
        fprintf(stderr, "threadpool_cancel_task: invalid arguments\n");
        return -1;
    }
    pthread_mutex_lock(&pool->lock);
    task->cancelled = 1;
    pthread_mutex_unlock(&pool->lock);
    return 0;
}

/* -------------------------------------------------------------------------
 * threadpool_wait_all
 * ------------------------------------------------------------------------- */
void threadpool_wait_all(threadpool_t* pool) {
    if (!pool) return;
    threadpool_destroy(pool);
}

/* -------------------------------------------------------------------------
 * threadpool_increase_threads
 * ------------------------------------------------------------------------- */
int threadpool_increase_threads(threadpool_t* pool, int count) {
    if (!pool || count <= 0) {
        fprintf(stderr, "threadpool_increase_threads: invalid arguments\n");
        return -1;
    }

    pthread_mutex_lock(&pool->lock);

    int old_total = pool->thread_count;
    int new_total = old_total + count;

    pthread_t* new_array = (pthread_t*)safe_realloc(pool->threads, sizeof(pthread_t) * new_total);
    if (!new_array) {
        perror("threadpool_increase_threads: safe_realloc");
        pthread_mutex_unlock(&pool->lock);
        return -1;
    }

    pool->threads = new_array;
    /* Create the additional threads */
    for (int i = old_total; i < new_total; ++i) {
        if (pthread_create(&pool->threads[i], NULL, threadpool_worker, pool) != 0) {
            perror("threadpool_increase_threads: pthread_create");
            /* If some threads fail to start, we can just reduce new_total to i */
            new_total = i;
            break;
        }
        pool->started++;
    }
    pool->thread_count = new_total;

    pthread_mutex_unlock(&pool->lock);
    return 0;
}

/* -------------------------------------------------------------------------
 * threadpool_decrease_threads
 * ------------------------------------------------------------------------- */
int threadpool_decrease_threads(threadpool_t* pool, int count) {
    if (!pool || count <= 0) {
        fprintf(stderr, "threadpool_decrease_threads: invalid arguments\n");
        return -1;
    }

    pthread_mutex_lock(&pool->lock);

    if (count >= pool->thread_count) {
        /* If removing as many or more threads than exist, just destroy. */
        pthread_mutex_unlock(&pool->lock);
        threadpool_destroy(pool);
        return 0;
    }

    /* We'll enqueue `count` special tasks that cause the threads to exit. */
    for (int i = 0; i < count; ++i) {
        enqueue_task(&pool->priority_queues[TASK_LOW], &g_exit_task);
        pthread_cond_signal(&pool->notify);
    }

    /* We note how many we expect to remove so the worker can check. */
    pool->workers_to_remove += count;

    pthread_mutex_unlock(&pool->lock);

    return 0;
}

static void threadpool_debug_dump_queue(const task_queue_t* queue, const char* queue_name);

void threadpool_debug_dump(threadpool_t* pool) {
    if (!pool) return;

    /* Lock to ensure we read consistent data. */
    pthread_mutex_lock(&pool->lock);

    /* Summarize basic pool info */
    MATO_LOG(MATO_LOG_INFO, 
        "thread_count=%d, started=%d, shutdown=%d, workers_to_remove=%d",
        pool->thread_count, pool->started, pool->shutdown, pool->workers_to_remove);

    /* Summarize tasks_remaining from the event struct */
    pthread_mutex_lock(&pool->event.mutex);
    int remain = pool->event.tasks_remaining;
    pthread_mutex_unlock(&pool->event.mutex);

    MATO_LOG(MATO_LOG_INFO, 
        "tasks_remaining=%d", remain);

    /* Dump each priority queue */
    static const char* prio_names[TASK_PRIORITY_COUNT] = {"LOW", "MEDIUM", "HIGH"};
    for (int p = 0; p < TASK_PRIORITY_COUNT; ++p) {
        char queue_name[32];
        snprintf(queue_name, sizeof(queue_name), "PriorityQueue[%s]", prio_names[p]);
        threadpool_debug_dump_queue(&pool->priority_queues[p], queue_name);
    }

    /* Dump delayed queue */
    MATO_LOG(MATO_LOG_INFO, "DelayedQueue (by execute_at):");
    threadpool_task_t* dtask = pool->delayed_queue.head;
    while (dtask) {
        MATO_LOG(MATO_LOG_INFO, 
            "  Task@%p: priority=%d, cancelled=%d, execute_at=%ld, repeat_interval=%ld",
            (void*)dtask, dtask->priority, dtask->cancelled, (long)dtask->execute_at, (long)dtask->repeat_interval);
        dtask = dtask->next;
    }

    pthread_mutex_unlock(&pool->lock);
}

/* -------------------------------------------------------------------------
 * Worker Thread Function
 * ------------------------------------------------------------------------- */
static void* threadpool_worker(void* arg) {
    threadpool_t* pool = (threadpool_t*)arg;
    threadpool_task_t* task = NULL;

    for (;;) {
        pthread_mutex_lock(&pool->lock);
        /*  Try immediate tasks from high to low priority */
        for (int p = TASK_HIGH; p >= TASK_LOW; --p) {
            if (pool->priority_queues[p].head) {
                MATO_LOG(MATO_LOG_INFO, "Checking to deque task of %d level", p);
                task = dequeue_task(&pool->priority_queues[p]);
                break;
            }
        }

        /*  If no immediate task, check delayed tasks */
        if (!task) {
            time_t now = time(NULL);
            while (pool->delayed_queue.head && pool->delayed_queue.head->execute_at <= now) {
                threadpool_task_t* dtask = pop_ready_delayed(&pool->delayed_queue);
                if (dtask) {
                    /* Promote to high priority once it's ready */
                    dtask->priority = TASK_HIGH;
                    enqueue_task(&pool->priority_queues[TASK_HIGH], dtask);
                }
            }
        }

        /*  If still no task, wait unless shutting down or removing workers */
        while (!task && !pool->shutdown && pool->workers_to_remove == 0) {
            pthread_cond_wait(&pool->notify, &pool->lock);

            /* Re-check for tasks after wake-up */
            for (int p = TASK_HIGH; p >= TASK_LOW; --p) {
                if (pool->priority_queues[p].head) {
                    task = dequeue_task(&pool->priority_queues[p]);
                    break;
                }
            }
            /* Also check if delayed tasks became ready while waiting */
            if (!task) {
                time_t now = time(NULL);
                while (pool->delayed_queue.head && pool->delayed_queue.head->execute_at <= now) {
                    threadpool_task_t* dtask = pop_ready_delayed(&pool->delayed_queue);
                    if (dtask) {
                        dtask->priority = TASK_HIGH;
                        enqueue_task(&pool->priority_queues[TASK_HIGH], dtask);
                    }
                }
                for (int p = TASK_HIGH; p >= TASK_LOW; --p) {
                    if (pool->priority_queues[p].head) {
                        task = dequeue_task(&pool->priority_queues[p]);
                        break;
                    }
                }
            }
        }

        /* If the pool is shutting down, break out */
        if (pool->shutdown) {
            pthread_mutex_unlock(&pool->lock);
            break;
        }

        /* If we need to remove this worker and no task, exit gracefully */
        if (!task && pool->workers_to_remove > 0) {
            pool->workers_to_remove--;
            pthread_mutex_unlock(&pool->lock);
            break;
        }

        pthread_mutex_unlock(&pool->lock);

        if (!task) {
            /* No task found, but not shutting down -> likely removing worker. */
            continue;
        }

        /* Check special "exit task" used for decreasing threads. */
        if (task == &g_exit_task) {
            pthread_mutex_lock(&pool->lock);
            pool->workers_to_remove--;
            pthread_mutex_unlock(&pool->lock);
            break;
        }

        /*  Actually run the task if not cancelled */
        if (!task->cancelled) {
            /* The function pointer expects: (void* arg, threadpool_event_t* event) */
            task->function(task->argument, &pool->event);

            /* Optional callback after the task finishes */
            if (task->callback) {
                task->callback(task->argument);
            }
        }

        /*  If repeating and not cancelled, re-queue as delayed */
        if (task->repeat_interval && !task->cancelled) {
            task->execute_at = time(NULL) + task->repeat_interval;
            pthread_mutex_lock(&pool->lock);
            enqueue_delayed(&pool->delayed_queue, task);
            pthread_mutex_unlock(&pool->lock);
        } else {
            /* Task is done, SAFE_FREE it. */
            SAFE_FREE(task);

            /* Update the global tasks_remaining count */
            pthread_mutex_lock(&pool->event.mutex);
            pool->event.tasks_remaining--;
            if (pool->event.tasks_remaining == 0) {
                pthread_cond_signal(&pool->event.cond);
            }
            pthread_mutex_unlock(&pool->event.mutex);
        }

        task = NULL;
    }

    /* Thread is exiting */
    pthread_mutex_lock(&pool->lock);
    pool->started--;
    pthread_mutex_unlock(&pool->lock);

    return NULL;
}

/* -------------------------------------------------------------------------
 * Immediate Task Queue Functions
 * ------------------------------------------------------------------------- */

static void enqueue_task(task_queue_t* queue, threadpool_task_t* task) {
    task->next = NULL;
    if (!queue->head) {
        queue->head = queue->tail = task;
    } else {
        queue->tail->next = task;
        queue->tail = task;
    }
}

static threadpool_task_t* dequeue_task(task_queue_t* queue) {
    if (!queue->head) return NULL;
    threadpool_task_t* t = queue->head;
    queue->head = t->next;
    if (!queue->head) {
        queue->tail = NULL;
    }
    t->next = NULL;
    return t;
}

/* -------------------------------------------------------------------------
 * Delayed Task Queue Functions
 * ------------------------------------------------------------------------- */

static void enqueue_delayed(delayed_task_queue_t* dqueue, threadpool_task_t* task) {
    /* Insert in ascending order of execute_at */
    if (!dqueue->head || task->execute_at < dqueue->head->execute_at) {
        task->next = dqueue->head;
        dqueue->head = task;
        return;
    }
    threadpool_task_t* current = dqueue->head;
    while (current->next && current->next->execute_at <= task->execute_at) {
        current = current->next;
    }
    task->next = current->next;
    current->next = task;
}

static threadpool_task_t* pop_ready_delayed(delayed_task_queue_t* dqueue) {
    /* Removes the head from the delayed queue (regardless of its time) */
    if (!dqueue->head) return NULL;
    threadpool_task_t* t = dqueue->head;
    dqueue->head = t->next;
    t->next = NULL;
    return t;
}


static void threadpool_debug_dump_queue(const task_queue_t* queue, const char* queue_name) {
    if (!queue) return;

    MATO_LOG(MATO_LOG_INFO, "%s:", queue_name);

    threadpool_task_t* t = queue->head;
    int index = 0;
    while (t) {
        MATO_LOG(MATO_LOG_INFO, 
            "  [%d] Task@%p: cancelled=%d, priority=%d, execute_at=%ld, repeat_interval=%ld",
            index, (void*)t, t->cancelled, t->priority, (long)t->execute_at, (long)t->repeat_interval);
        index++;
        t = t->next;
    }
    if (index == 0) {
        MATO_LOG(MATO_LOG_INFO, "  (empty)");
    }
}
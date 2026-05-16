#include "include/eduos.h"

/* ═══════════════════════════════════════════════
   THREAD POOL IMPLEMENTATION
   ═══════════════════════════════════════════════ */

/* ─── Worker Thread Function ─── */
static void *worker_thread(void *arg) {
    ThreadPool *pool = (ThreadPool *)arg;

    while (1) {
        pthread_mutex_lock(&pool->lock);

        /* Wait until there is a task or shutdown signal */
        while (pool->task_count == 0 && !pool->shutdown) {
            pthread_cond_wait(&pool->notify, &pool->lock);
        }

        /* If shutting down and no tasks left, exit */
        if (pool->shutdown && pool->task_count == 0) {
            pthread_mutex_unlock(&pool->lock);
            pthread_exit(NULL);
        }

        /* Get task from queue */
        Task *task = pool->task_queue;
        if (task != NULL) {
            pool->task_queue = task->next;
            pool->task_count--;
        }

        pthread_mutex_unlock(&pool->lock);

        /* Execute the task */
        if (task != NULL) {
            task->function(task->arg);
            free(task);
        }
    }
    return NULL;
}

/* ─── Create Thread Pool ─── */
ThreadPool* thread_pool_create() {
    ThreadPool *pool = (ThreadPool *)malloc(sizeof(ThreadPool));
    if (!pool) {
        perror("malloc thread pool");
        return NULL;
    }

    pool->task_queue  = NULL;
    pool->shutdown    = 0;
    pool->task_count  = 0;

    if (pthread_mutex_init(&pool->lock, NULL) != 0) {
        perror("mutex init");
        free(pool);
        return NULL;
    }

    if (pthread_cond_init(&pool->notify, NULL) != 0) {
        perror("cond init");
        pthread_mutex_destroy(&pool->lock);
        free(pool);
        return NULL;
    }

    /* Create worker threads */
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        if (pthread_create(&pool->threads[i], NULL,
                           worker_thread, pool) != 0) {
            perror("pthread_create");
            thread_pool_shutdown(pool);
            return NULL;
        }
        printf("[ThreadPool] Worker thread %d created\n", i);
    }

    return pool;
}

/* ─── Add Task to Pool ─── */
void thread_pool_add_task(ThreadPool *pool,
                           void (*func)(void *), void *arg) {
    Task *task = (Task *)malloc(sizeof(Task));
    if (!task) {
        perror("malloc task");
        return;
    }

    task->function = func;
    task->arg      = arg;
    task->next     = NULL;

    pthread_mutex_lock(&pool->lock);

    /* Add task to end of queue */
    if (pool->task_queue == NULL) {
        pool->task_queue = task;
    } else {
        Task *tmp = pool->task_queue;
        while (tmp->next != NULL) tmp = tmp->next;
        tmp->next = task;
    }
    pool->task_count++;

    pthread_cond_signal(&pool->notify);
    pthread_mutex_unlock(&pool->lock);
}

/* ─── Shutdown Thread Pool ─── */
void thread_pool_shutdown(ThreadPool *pool) {
    if (!pool) return;

    pthread_mutex_lock(&pool->lock);
    pool->shutdown = 1;
    pthread_cond_broadcast(&pool->notify);
    pthread_mutex_unlock(&pool->lock);

    /* Wait for all threads to finish */
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_join(pool->threads[i], NULL);
        printf("[ThreadPool] Worker thread %d finished\n", i);
    }

    pthread_mutex_destroy(&pool->lock);
    pthread_cond_destroy(&pool->notify);
    free(pool);
    printf("[ThreadPool] All threads shut down cleanly\n");
}

/* ═══════════════════════════════════════════════
   RACE CONDITION DEMONSTRATION
   Many threads increment a shared counter
   ═══════════════════════════════════════════════ */

#define NUM_THREADS  4
#define NUM_INCREMENTS 250000

static long shared_counter = 0;
static pthread_mutex_t counter_mutex;

/* ─── WITHOUT mutex (race condition) ─── */
static void *increment_no_mutex(void *arg) {
    (void)arg;
    for (int i = 0; i < NUM_INCREMENTS; i++) {
        shared_counter++;   /* NOT protected — race condition! */
    }
    return NULL;
}

/* ─── WITH mutex (fixed) ─── */
static void *increment_with_mutex(void *arg) {
    (void)arg;
    for (int i = 0; i < NUM_INCREMENTS; i++) {
        pthread_mutex_lock(&counter_mutex);
        shared_counter++;   /* Protected — always correct */
        pthread_mutex_unlock(&counter_mutex);
    }
    return NULL;
}

/* ─── Run Race Condition Demo ─── */
void demo_race_condition(int use_mutex) {
    pthread_t threads[NUM_THREADS];
    shared_counter = 0;

    if (use_mutex) {
        pthread_mutex_init(&counter_mutex, NULL);
        printf("\n[Race Demo] Running WITH mutex protection\n");
    } else {
        printf("\n[Race Demo] Running WITHOUT mutex (expect wrong results)\n");
    }

    /* Create threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        if (use_mutex) {
            pthread_create(&threads[i], NULL,
                           increment_with_mutex, NULL);
        } else {
            pthread_create(&threads[i], NULL,
                           increment_no_mutex, NULL);
        }
    }

    /* Wait for all threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    long expected = (long)NUM_THREADS * NUM_INCREMENTS;
    printf("[Race Demo] Expected counter = %ld\n", expected);
    printf("[Race Demo] Actual  counter = %ld\n",  shared_counter);

    if (shared_counter == expected) {
        printf("[Race Demo] CORRECT — mutex worked!\n");
    } else {
        printf("[Race Demo] WRONG  — race condition occurred!\n");
    }

    if (use_mutex) {
        pthread_mutex_destroy(&counter_mutex);
    }
}

/* ═══════════════════════════════════════════════
   DEADLOCK DEMONSTRATION & FIX
   Two mutexes acquired in opposite order
   ═══════════════════════════════════════════════ */

static pthread_mutex_t mutex_A;
static pthread_mutex_t mutex_B;

/* ─── Fixed: always lock A before B ─── */
static void *thread_lock_AB(void *arg) {
    (void)arg;
    pthread_mutex_lock(&mutex_A);
    printf("[Deadlock Fix] Thread 1 locked A, waiting for B\n");
    sleep(1);
    pthread_mutex_lock(&mutex_B);
    printf("[Deadlock Fix] Thread 1 locked B — done\n");
    pthread_mutex_unlock(&mutex_B);
    pthread_mutex_unlock(&mutex_A);
    return NULL;
}

static void *thread_lock_AB2(void *arg) {
    (void)arg;
    pthread_mutex_lock(&mutex_A);   /* consistent order: A then B */
    printf("[Deadlock Fix] Thread 2 locked A, waiting for B\n");
    pthread_mutex_lock(&mutex_B);
    printf("[Deadlock Fix] Thread 2 locked B — done\n");
    pthread_mutex_unlock(&mutex_B);
    pthread_mutex_unlock(&mutex_A);
    return NULL;
}

void demo_deadlock_fix() {
    printf("\n[Deadlock Fix] Demonstrating consistent lock ordering\n");
    pthread_mutex_init(&mutex_A, NULL);
    pthread_mutex_init(&mutex_B, NULL);

    pthread_t t1, t2;
    pthread_create(&t1, NULL, thread_lock_AB,  NULL);
    pthread_create(&t2, NULL, thread_lock_AB2, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    pthread_mutex_destroy(&mutex_A);
    pthread_mutex_destroy(&mutex_B);
    printf("[Deadlock Fix] No deadlock — consistent ordering works!\n");
}

/* ═══════════════════════════════════════════════
   PRODUCER CONSUMER WITH SEMAPHORE
   ═══════════════════════════════════════════════ */

#define BUFFER_SIZE 5

static int    buffer[BUFFER_SIZE];
static int    buf_in  = 0;
static int    buf_out = 0;
static sem_t  sem_empty;
static sem_t  sem_full;
static pthread_mutex_t buf_mutex;

static void *producer(void *arg) {
    (void)arg;
    for (int i = 0; i < 10; i++) {
        sem_wait(&sem_empty);
        pthread_mutex_lock(&buf_mutex);

        buffer[buf_in] = i;
        printf("[Producer] Produced item %d at slot %d\n", i, buf_in);
        buf_in = (buf_in + 1) % BUFFER_SIZE;

        pthread_mutex_unlock(&buf_mutex);
        sem_post(&sem_full);
        usleep(100000);
    }
    return NULL;
}

static void *consumer(void *arg) {
    (void)arg;
    for (int i = 0; i < 10; i++) {
        sem_wait(&sem_full);
        pthread_mutex_lock(&buf_mutex);

        int item = buffer[buf_out];
        printf("[Consumer] Consumed item %d from slot %d\n", item, buf_out);
        buf_out = (buf_out + 1) % BUFFER_SIZE;

        pthread_mutex_unlock(&buf_mutex);
        sem_post(&sem_empty);
        usleep(150000);
    }
    return NULL;
}

void demo_producer_consumer() {
    printf("\n[Semaphore] Producer-Consumer demonstration\n");
    sem_init(&sem_empty, 0, BUFFER_SIZE);
    sem_init(&sem_full,  0, 0);
    pthread_mutex_init(&buf_mutex, NULL);

    pthread_t prod, cons;
    pthread_create(&prod, NULL, producer, NULL);
    pthread_create(&cons, NULL, consumer, NULL);
    pthread_join(prod, NULL);
    pthread_join(cons, NULL);

    sem_destroy(&sem_empty);
    sem_destroy(&sem_full);
    pthread_mutex_destroy(&buf_mutex);
    printf("[Semaphore] Producer-Consumer done!\n");
}

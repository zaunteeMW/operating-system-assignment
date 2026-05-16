#ifndef EDUOS_H
#define EDUOS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <sys/types.h>

/* ─── Process States ─── */
#define NEW        0
#define READY      1
#define RUNNING    2
#define WAITING    3
#define TERMINATED 4

/* ─── Thread Pool Size ─── */
#define THREAD_POOL_SIZE 4

/* ─── Maximum Processes ─── */
#define MAX_PROCESSES 50

/* ─── Process Control Block ─── */
typedef struct {
    pid_t  pid;              /* unique process ID */
    char   name[64];         /* process name / program */
    int    state;            /* NEW|READY|RUNNING|WAITING|TERMINATED */
    int    priority;         /* 0 = highest */
    int    burst_time;       /* total CPU time needed */
    int    arrival_time;     /* clock tick of arrival */
    int    remaining_time;   /* used by preemptive algorithms */
    int    memory_req_kb;    /* memory footprint in KB */
    int    thread_count;     /* threads spawned by process */
    time_t creation_time;    /* wall-clock timestamp */
    int    exit_code;        /* exit code when terminated */
    pid_t  parent_pid;       /* parent process ID */
    int    owner_id;         /* owner for access control */
} PCB;

/* ─── Task for Thread Pool ─── */
typedef struct Task {
    void (*function)(void *arg);
    void *arg;
    struct Task *next;
} Task;

/* ─── Thread Pool ─── */
typedef struct {
    pthread_t threads[THREAD_POOL_SIZE];
    Task     *task_queue;
    pthread_mutex_t lock;
    pthread_cond_t  notify;
    int shutdown;
    int task_count;
} ThreadPool;

/* ─── Shared Memory Structure ─── */
typedef struct {
    int    pid;
    int    state;
    int    burst_time;
    int    remaining_time;
    int    owner_id;
    pthread_mutex_t shm_mutex;
} SharedMetrics;

/* ─── Function Declarations: Process Manager ─── */
pid_t edu_fork(PCB *parent);
void  edu_exec(pid_t pid, char *prog_name);
int   edu_wait(pid_t parent_pid);
void  edu_exit(pid_t pid, int exit_code);
void  edu_ps();
void  save_pcb_snapshot();
void  print_state(int state);

/* ─── Function Declarations: Thread Manager ─── */
ThreadPool* thread_pool_create();
void        thread_pool_add_task(ThreadPool *pool,
                                  void (*func)(void*), void *arg);
void        thread_pool_shutdown(ThreadPool *pool);

/* ─── Function Declarations: IPC Module ─── */
void demo_shared_memory();
void demo_pipe();

#endif /* EDUOS_H */

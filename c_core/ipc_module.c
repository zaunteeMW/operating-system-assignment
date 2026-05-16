#include "include/eduos.h"
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>

/* ═══════════════════════════════════════════════
   SHARED MEMORY IPC DEMONSTRATION
   Two processes share a struct via shm_open/mmap
   ═══════════════════════════════════════════════ */

#define SHM_NAME "/eduos_shm"
#define OWNER_ID  42

void demo_shared_memory() {
    printf("\n[IPC-SHM] Starting Shared Memory demonstration\n");

    /* Create shared memory object */
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return;
    }

    /* Set size of shared memory */
    if (ftruncate(shm_fd, sizeof(SharedMetrics)) == -1) {
        perror("ftruncate");
        close(shm_fd);
        shm_unlink(SHM_NAME);
        return;
    }

    /* Map shared memory into address space */
    SharedMetrics *metrics = (SharedMetrics *)mmap(
        NULL, sizeof(SharedMetrics),
        PROT_READ | PROT_WRITE,
        MAP_SHARED, shm_fd, 0
    );

    if (metrics == MAP_FAILED) {
        perror("mmap");
        close(shm_fd);
        shm_unlink(SHM_NAME);
        return;
    }

    /* Initialize shared memory */
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&metrics->shm_mutex, &attr);
    pthread_mutexattr_destroy(&attr);

    metrics->pid           = 1001;
    metrics->state         = RUNNING;
    metrics->burst_time    = 8;
    metrics->remaining_time = 8;
    metrics->owner_id      = OWNER_ID;

    /* Fork a child process */
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        munmap(metrics, sizeof(SharedMetrics));
        close(shm_fd);
        shm_unlink(SHM_NAME);
        return;
    }

    if (pid == 0) {
        /* ── CHILD PROCESS ── */

        /* Access control check */
        if (metrics->owner_id != OWNER_ID) {
            printf("[IPC-SHM] CHILD: Access DENIED — owner_id mismatch\n");
            munmap(metrics, sizeof(SharedMetrics));
            close(shm_fd);
            exit(1);
        }

        printf("[IPC-SHM] CHILD: Access GRANTED — owner_id matches\n");

        pthread_mutex_lock(&metrics->shm_mutex);

        printf("[IPC-SHM] CHILD: Reading shared metrics\n");
        printf("[IPC-SHM] CHILD: PID=%d State=%d Burst=%d\n",
               metrics->pid,
               metrics->state,
               metrics->burst_time);

        /* Update shared memory */
        metrics->remaining_time = 5;
        metrics->state          = WAITING;
        printf("[IPC-SHM] CHILD: Updated remaining_time=5 state=WAITING\n");

        pthread_mutex_unlock(&metrics->shm_mutex);

        munmap(metrics, sizeof(SharedMetrics));
        close(shm_fd);
        exit(0);

    } else {
        /* ── PARENT PROCESS ── */
        wait(NULL);

        pthread_mutex_lock(&metrics->shm_mutex);
        printf("[IPC-SHM] PARENT: Child finished\n");
        printf("[IPC-SHM] PARENT: remaining_time now = %d\n",
               metrics->remaining_time);
        printf("[IPC-SHM] PARENT: state now = %d\n",
               metrics->state);
        pthread_mutex_unlock(&metrics->shm_mutex);

        /* Cleanup */
        pthread_mutex_destroy(&metrics->shm_mutex);
        munmap(metrics, sizeof(SharedMetrics));
        close(shm_fd);
        shm_unlink(SHM_NAME);
        printf("[IPC-SHM] Shared Memory demonstration complete\n");
    }
}

/* ═══════════════════════════════════════════════
   ANONYMOUS PIPE IPC DEMONSTRATION
   Parent sends PCB data to child via pipe()
   ═══════════════════════════════════════════════ */

void demo_pipe() {
    printf("\n[IPC-PIPE] Starting Anonymous Pipe demonstration\n");

    int pipefd[2];

    if (pipe(pipefd) == -1) {
        perror("pipe");
        return;
    }

    /* Sample PCB data to send */
    PCB send_pcb;
    send_pcb.pid            = 2001;
    send_pcb.state          = READY;
    send_pcb.priority       = 2;
    send_pcb.burst_time     = 6;
    send_pcb.arrival_time   = 0;
    send_pcb.remaining_time = 6;
    send_pcb.memory_req_kb  = 256;
    send_pcb.thread_count   = 2;
    send_pcb.exit_code      = 0;
    send_pcb.parent_pid     = 1;
    send_pcb.owner_id       = OWNER_ID;
    strncpy(send_pcb.name, "pipe_test_process", 63);

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return;
    }

    if (pid == 0) {
        /* ── CHILD: reads from pipe ── */
        close(pipefd[1]);   /* close write end */

        PCB recv_pcb;
        ssize_t bytes = read(pipefd[0], &recv_pcb, sizeof(PCB));
        if (bytes == -1) {
            perror("read pipe");
            close(pipefd[0]);
            exit(1);
        }

        printf("[IPC-PIPE] CHILD: Received PCB from pipe\n");
        printf("[IPC-PIPE] CHILD: PID=%-5d Name=%-20s "
               "State=%d Priority=%d Burst=%d\n",
               recv_pcb.pid,
               recv_pcb.name,
               recv_pcb.state,
               recv_pcb.priority,
               recv_pcb.burst_time);

        close(pipefd[0]);
        exit(0);

    } else {
        /* ── PARENT: writes to pipe ── */
        close(pipefd[0]);   /* close read end */

        printf("[IPC-PIPE] PARENT: Sending PCB PID=%d via pipe\n",
               send_pcb.pid);

        ssize_t bytes = write(pipefd[1], &send_pcb, sizeof(PCB));
        if (bytes == -1) {
            perror("write pipe");
            close(pipefd[1]);
            return;
        }

        close(pipefd[1]);
        wait(NULL);
        printf("[IPC-PIPE] Pipe demonstration complete\n");
    }
}

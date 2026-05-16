#include "include/eduos.h"

/* ─── External function declarations ─── */
void demo_race_condition(int use_mutex);
void demo_deadlock_fix();
void demo_producer_consumer();
void demo_shared_memory();
void demo_pipe();

/* ═══════════════════════════════════════════════
   TASK FUNCTIONS FOR THREAD POOL
   ═══════════════════════════════════════════════ */

static void task_sum(void *arg) {
    int *num = (int *)arg;
    long sum = 0;
    for (int i = 1; i <= *num; i++) sum += i;
    printf("[Task] Sum of 1 to %d = %ld\n", *num, sum);
}

/* ═══════════════════════════════════════════════
   MAIN SIMULATION DRIVER
   ═══════════════════════════════════════════════ */

int main(void) {
    srand((unsigned int)time(NULL));

    printf("\n");
    printf("╔══════════════════════════════════════════════════╗\n");
    printf("║         EduOS Simulator — Hajj Sulaiman          ║\n");
    printf("║         Reg: 25311351025 | 351 CS 2104           ║\n");
    printf("╚══════════════════════════════════════════════════╝\n\n");

    /* ─── PART 1: Process Management ─── */
    printf("═══════════════════════════════════════════════════\n");
    printf(" PART 1: PROCESS MANAGEMENT\n");
    printf("═══════════════════════════════════════════════════\n");

    /* Create parent process */
    PCB parent;
    parent.pid            = 1;
    parent.parent_pid     = 0;
    parent.state          = RUNNING;
    parent.priority       = 0;
    parent.burst_time     = 10;
    parent.arrival_time   = 0;
    parent.remaining_time = 10;
    parent.memory_req_kb  = 512;
    parent.thread_count   = 1;
    parent.exit_code      = 0;
    parent.owner_id       = 42;
    parent.creation_time  = time(NULL);
    strncpy(parent.name, "init", 63);

    /* Add parent to PCB table */
    extern PCB pcb_table[];
    extern int process_count;
    extern int pid_counter;
    pcb_table[0]  = parent;
    process_count = 1;
    pid_counter   = 2;

    /* Fork three child processes */
    pid_t child1 = edu_fork(&parent);
    pid_t child2 = edu_fork(&parent);
    pid_t child3 = edu_fork(&parent);

    /* Execute programs in children */
    edu_exec(child1, "calculator");
    edu_exec(child2, "file_manager");
    edu_exec(child3, "text_editor");

    /* Show process table */
    edu_ps();

    /* Terminate children */
    edu_exit(child1, 0);
    edu_exit(child2, 0);
    edu_exit(child3, 0);

    /* Parent waits */
    edu_wait(parent.pid);

    /* Terminate parent */
    edu_exit(parent.pid, 0);

    /* Show final process table */
    printf("\nFinal Process Table:\n");
    edu_ps();

    /* ─── PART 2: Thread Pool ─── */
    printf("═══════════════════════════════════════════════════\n");
    printf(" PART 2: THREAD POOL DEMONSTRATION\n");
    printf("═══════════════════════════════════════════════════\n");

    ThreadPool *pool = thread_pool_create();
    if (pool) {
        int nums[] = {100, 200, 300, 400, 500, 600, 700, 800};
        for (int i = 0; i < 8; i++) {
            thread_pool_add_task(pool, task_sum, &nums[i]);
        }
        sleep(2);
        thread_pool_shutdown(pool);
    }

    /* ─── PART 3: Race Condition Demo ─── */
    printf("\n═══════════════════════════════════════════════════\n");
    printf(" PART 3: RACE CONDITION DEMONSTRATION\n");
    printf("═══════════════════════════════════════════════════\n");

    /* Without mutex — wrong results */
    demo_race_condition(0);

    /* With mutex — correct results */
    demo_race_condition(1);

    /* ─── PART 4: Deadlock Fix ─── */
    printf("\n═══════════════════════════════════════════════════\n");
    printf(" PART 4: DEADLOCK FIX DEMONSTRATION\n");
    printf("═══════════════════════════════════════════════════\n");

    demo_deadlock_fix();

    /* ─── PART 5: Producer Consumer ─── */
    printf("\n═══════════════════════════════════════════════════\n");
    printf(" PART 5: PRODUCER CONSUMER WITH SEMAPHORE\n");
    printf("═══════════════════════════════════════════════════\n");

    demo_producer_consumer();

    /* ─── PART 6: IPC Demonstrations ─── */
    printf("\n═══════════════════════════════════════════════════\n");
    printf(" PART 6: IPC — SHARED MEMORY\n");
    printf("═══════════════════════════════════════════════════\n");

    demo_shared_memory();

    printf("\n═══════════════════════════════════════════════════\n");
    printf(" PART 7: IPC — ANONYMOUS PIPE\n");
    printf("═══════════════════════════════════════════════════\n");

    demo_pipe();

    /* ─── Final Summary ─── */
    printf("\n");
    printf("╔══════════════════════════════════════════════════╗\n");
    printf("║              EduOS Simulation Complete           ║\n");
    printf("║         PCB snapshot saved to pcb_snapshot.json ║\n");
    printf("╚══════════════════════════════════════════════════╝\n\n");

    return 0;
}

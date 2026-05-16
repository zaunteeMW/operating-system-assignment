#include "include/eduos.h"

/* ─── Global PCB Table ─── */
PCB pcb_table[MAX_PROCESSES];
int process_count = 0;
int pid_counter   = 1;

/* ─── Helper: Get timestamp string ─── */
static void get_timestamp(char *buf, size_t len) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buf, len, "%H:%M:%S", t);
}

/* ─── Helper: Print state as string ─── */
void print_state(int state) {
    switch(state) {
        case NEW:        printf("NEW       "); break;
        case READY:      printf("READY     "); break;
        case RUNNING:    printf("RUNNING   "); break;
        case WAITING:    printf("WAITING   "); break;
        case TERMINATED: printf("TERMINATED"); break;
        default:         printf("UNKNOWN   "); break;
    }
}

/* ─── Helper: State as string for JSON ─── */
static const char* state_str(int state) {
    switch(state) {
        case NEW:        return "NEW";
        case READY:      return "READY";
        case RUNNING:    return "RUNNING";
        case WAITING:    return "WAITING";
        case TERMINATED: return "TERMINATED";
        default:         return "UNKNOWN";
    }
}

/* ─── Save PCB Table to JSON ─── */
void save_pcb_snapshot() {
    FILE *f = fopen("pcb_snapshot.json", "w");
    if (!f) {
        perror("fopen pcb_snapshot.json");
        return;
    }
    fprintf(f, "[\n");
    for (int i = 0; i < process_count; i++) {
        PCB *p = &pcb_table[i];
        fprintf(f, "  {\n");
        fprintf(f, "    \"pid\": %d,\n",            p->pid);
        fprintf(f, "    \"name\": \"%s\",\n",        p->name);
        fprintf(f, "    \"state\": \"%s\",\n",       state_str(p->state));
        fprintf(f, "    \"priority\": %d,\n",        p->priority);
        fprintf(f, "    \"burst_time\": %d,\n",      p->burst_time);
        fprintf(f, "    \"arrival_time\": %d,\n",    p->arrival_time);
        fprintf(f, "    \"remaining_time\": %d,\n",  p->remaining_time);
        fprintf(f, "    \"memory_req_kb\": %d,\n",   p->memory_req_kb);
        fprintf(f, "    \"thread_count\": %d,\n",    p->thread_count);
        fprintf(f, "    \"exit_code\": %d,\n",       p->exit_code);
        fprintf(f, "    \"parent_pid\": %d,\n",      p->parent_pid);
        fprintf(f, "    \"owner_id\": %d,\n",        p->owner_id);
        fprintf(f, "    \"creation_time\": %ld\n",   p->creation_time);
        fprintf(f, "  }%s\n", (i < process_count - 1) ? "," : "");
    }
    fprintf(f, "]\n");
    fclose(f);
}

/* ─── edu_fork ─── */
pid_t edu_fork(PCB *parent) {
    char ts[16];
    get_timestamp(ts, sizeof(ts));

    if (process_count >= MAX_PROCESSES) {
        printf("[%s] ERROR: Process table full\n", ts);
        return -1;
    }

    PCB *child = &pcb_table[process_count++];
    memcpy(child, parent, sizeof(PCB));

    child->pid          = pid_counter++;
    child->parent_pid   = parent->pid;
    child->state        = NEW;
    child->creation_time = time(NULL);
    child->exit_code    = 0;

    printf("[%s] edu_fork: Created child PID=%d from parent PID=%d | State=NEW\n",
           ts, child->pid, parent->pid);

    child->state = READY;
    printf("[%s] edu_fork: PID=%d state changed NEW -> READY\n",
           ts, child->pid);

    save_pcb_snapshot();
    return child->pid;
}

/* ─── edu_exec ─── */
void edu_exec(pid_t pid, char *prog_name) {
    char ts[16];
    get_timestamp(ts, sizeof(ts));

    for (int i = 0; i < process_count; i++) {
        if (pcb_table[i].pid == pid) {
            strncpy(pcb_table[i].name, prog_name, 63);
            pcb_table[i].name[63]        = '\0';
            pcb_table[i].burst_time      = (rand() % 10) + 1;
            pcb_table[i].remaining_time  = pcb_table[i].burst_time;
            pcb_table[i].state           = RUNNING;

            printf("[%s] edu_exec: PID=%d loaded program='%s' "
                   "burst=%d | State=RUNNING\n",
                   ts, pid, prog_name, pcb_table[i].burst_time);

            save_pcb_snapshot();
            return;
        }
    }
    printf("[%s] edu_exec: ERROR PID=%d not found\n", ts, pid);
}

/* ─── edu_wait ─── */
int edu_wait(pid_t parent_pid) {
    char ts[16];
    get_timestamp(ts, sizeof(ts));

    printf("[%s] edu_wait: PID=%d waiting for children to terminate\n",
           ts, parent_pid);

    /* Find the parent and set to WAITING */
    for (int i = 0; i < process_count; i++) {
        if (pcb_table[i].pid == parent_pid) {
            pcb_table[i].state = WAITING;
            break;
        }
    }
    save_pcb_snapshot();

    /* Check all children are terminated */
    int all_done = 0;
    while (!all_done) {
        all_done = 1;
        for (int i = 0; i < process_count; i++) {
            if (pcb_table[i].parent_pid == parent_pid &&
                pcb_table[i].state != TERMINATED) {
                all_done = 0;
                break;
            }
        }
        if (!all_done) sleep(1);
    }

    /* Get last child exit code */
    int exit_code = 0;
    for (int i = 0; i < process_count; i++) {
        if (pcb_table[i].parent_pid == parent_pid) {
            exit_code = pcb_table[i].exit_code;
        }
    }

    printf("[%s] edu_wait: PID=%d all children terminated | exit_code=%d\n",
           ts, parent_pid, exit_code);
    return exit_code;
}

/* ─── edu_exit ─── */
void edu_exit(pid_t pid, int exit_code) {
    char ts[16];
    get_timestamp(ts, sizeof(ts));

    for (int i = 0; i < process_count; i++) {
        if (pcb_table[i].pid == pid) {
            pcb_table[i].state     = TERMINATED;
            pcb_table[i].exit_code = exit_code;

            printf("[%s] edu_exit: PID=%d TERMINATED | exit_code=%d\n",
                   ts, pid, exit_code);

            save_pcb_snapshot();
            return;
        }
    }
    printf("[%s] edu_exit: ERROR PID=%d not found\n", ts, pid);
}

/* ─── edu_ps ─── */
void edu_ps() {
    printf("\n╔══════════════════════════════════════════════════"
           "═══════════════════════╗\n");
    printf("║                        EduOS Process Table (ps aux)"
           "                      ║\n");
    printf("╠═══════╦════════════════════╦════════════╦══════════"
           "╦══════════╦══════════╣\n");
    printf("║  PID  ║ NAME               ║ STATE      ║ PRIORITY "
           "║ BURST    ║ MEMORY   ║\n");
    printf("╠═══════╬════════════════════╬════════════╬══════════"
           "╬══════════╬══════════╣\n");

    for (int i = 0; i < process_count; i++) {
        PCB *p = &pcb_table[i];
        printf("║ %-5d ║ %-18s ║ ", p->pid, p->name);
        print_state(p->state);
        printf(" ║ %-8d ║ %-8d ║ %-8d ║\n",
               p->priority, p->burst_time, p->memory_req_kb);
    }

    printf("╚═══════╩════════════════════╩════════════╩══════════"
           "╩══════════╩══════════╝\n\n");
}

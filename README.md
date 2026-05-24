markdown# EduOS — Operating Systems Simulator

Module Code: 351 CS 2104
Module Name: Operating Systems
Student Name: Laurent Namacha
Registration Number: 25311351030
Semester: III

## Project Overview

EduOS is a multi-component operating system simulator built in C and Python.
It demonstrates core OS concepts including process management, thread pools,
inter-process communication, and CPU scheduling algorithms. The C core handles
low-level simulation while Python handles scheduling visualisation.

## Prerequisites

### For the C components:
- GCC compiler (MinGW on Windows)
- POSIX threads support (-pthread)
- Valgrind (Linux/WSL recommended for memory checks)

### For the Python components:
- Python 3.8 or higher
- pip (Python package manager)

## Build Instructions

### C Core
```bash
cd c_core
make all
```

### Python Scheduler
```bash
cd python_scheduler
pip install -r requirements.txt
python scheduler_sim.py --random 10
```

### Run Full Integration
```bash
cd controller
python main_controller.py
```

## Directory Structure
EduOS-25311351030/
├── README.md               # Project documentation
├── .gitignore              # Excludes build artifacts and cache files
├── docs/
│   ├── report.pdf          # Final written report
│   └── screenshots/        # All Gantt charts and comparison charts
├── c_core/
│   ├── Makefile            # Build targets: all, clean, race, fixed, memcheck
│   ├── include/
│   │   └── eduos.h         # Shared struct definitions (PCB, etc.)
│   ├── process_manager.c   # edu_fork, edu_exec, edu_wait, edu_exit, edu_ps
│   ├── thread_manager.c    # Thread pool, race condition demo, deadlock fix
│   ├── ipc_module.c        # Shared memory and anonymous pipe IPC
│   └── main_sim.c          # Driver that runs all C components
├── python_scheduler/
│   ├── scheduler_sim.py    # All 4 scheduling algorithms
│   ├── gantt.py            # Gantt chart and visualisation code
│   ├── sample_processes.csv # Sample input file
│   └── requirements.txt    # Python dependencies
└── controller/
└── main_controller.py  # Integration bridge between C and Python

## Screenshots
 C Simulator Output
 FCFS Gantt Chart
 SJF Gantt Chart
 Priority Scheduling
 Round Robin Scheduling
 Scheduling Comparison Charts

## Valgrind Output

Valgrind memory analysis was successfully performed on the C simulator executable.

### Command Used
```bash
valgrind ./eduos
```

### Summary
```text
HEAP SUMMARY:
    in use at exit: 0 bytes in 0 blocks
    total heap usage: 59 allocs, 59 frees, 61,834 bytes allocated

All heap blocks were freed -- no leaks are possible

ERROR SUMMARY: 0 errors from 0 contexts
```

### Interpretation
- No memory leaks were detected.
- All dynamically allocated heap memory was properly released.
- The simulator executed successfully under Valgrind analysis.
  
## Challenges and Solutions

### 1. GCC Compilation Warnings
**Challenge:**  
The compiler produced warnings related to the `usleep()` function during compilation.

**Solution:**  
The issue was resolved by including the `<unistd.h>` header file and recompiling the project using GCC with pthread support.

---

### 2. Python Dependency Errors
**Challenge:**  
The Python scheduler simulator failed due to missing modules such as `tabulate` and other dependencies.

**Solution:**  
Required packages were installed using `pip3 install` and the `requirements.txt` configuration file.

---

### 3. GitHub Authentication Problems
**Challenge:**  
Git push commands failed because GitHub no longer supports password authentication for HTTPS operations.

**Solution:**  
Project files and screenshots were uploaded manually through the GitHub web interface and repository synchronization was completed successfully.

---

### 4. Script Execution Issues
**Challenge:**  
The Python scheduling script initially produced permission and interpreter errors when executed directly.

**Solution:**  
The script was executed using `python3 scheduler_sim.py`, and executable permissions were configured correctly where necessary.

---

### 5. Gantt Chart Generation
**Challenge:**  
The simulator initially failed to locate `gantt.py` for chart generation.

**Solution:**  
The program was executed from the correct project directory, allowing chart generation and screenshot export to function properly.

## References

- Silberschatz, A., Galvin, P. B., & Gagne, G. *Operating System Concepts*, 10th Edition
- Linux man pages: `fork(2)`, `execve(2)`, `wait(2)`, `pthread_create(3)`
- Python Documentation — https://docs.python.org/3/
- GCC Documentation — https://gcc.gnu.org/onlinedocs/
- POSIX Threads Programming Guide
- Matplotlib Documentation — https://matplotlib.org/
- W3Schools — https://www.w3schools.com/
- GeeksforGeeks — https://www.geeksforgeeks.org/
- Linux man pages: fork(2), execve(2), wait(2), pthread_create(3)
- Course notes - 351 CS 2104 Operating Systems

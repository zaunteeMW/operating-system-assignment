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
 <img width="1366" height="768" alt="Screenshot (24)" src="https://github.com/user-attachments/assets/f20d4a9e-8751-4dc9-9f59-a035e6c74b88" />
<img width="1366" height="768" alt="Screenshot (23)" src="https://github.com/user-attachments/assets/f6f30ef2-49a2-4703-919c-748f532af70f" />
<img width="1366" height="768" alt="Screenshot (22)" src="https://github.com/user-attachments/assets/72cdafae-6487-40ef-8148-64baab43a910" />
<img width="1366" height="768" alt="Screenshot (21)" src="https://github.com/user-attachments/assets/f7e8d8f7-f1d1-4f07-9b61-4aebf4a93c63" />
<img width="1366" height="768" alt="Screenshot (20)" src="https://github.com/user-attachments/assets/67688bef-c260-4bad-9f6b-9d19828b835c" />
<img width="1366" height="768" alt="Screenshot (19)" src="https://github.com/user-attachments/assets/04e30741-c025-4e1a-b4c5-c0ff5010d73d" />
<img width="1366" height="768" alt="Screenshot (18)" src="https://github.com/user-attachments/assets/aeb987f7-6d64-4973-a392-1ed27532d30e" />
<img width="1366" height="768" alt="Screenshot (17)" src="https://github.com/user-attachments/assets/fa1d2946-f25c-4bd1-bff0-8a9bdbd8c384" />
<img width="1366" height="768" alt="Screenshot (16)" src="https://github.com/user-attachments/assets/43569332-86f4-4418-b92c-0b44cdd41058" />
<img width="1366" height="768" alt="Screenshot (15)" src="https://github.com/user-attachments/assets/8e916df0-0dc9-46f2-a745-7b79d3202f93" />
<img width="1366" height="768" alt="Screenshot (14)" src="https://github.com/user-attachments/assets/bd4b8e00-e8a8-4e68-b549-2c943373b618" />



SJF Gantt Chart
<img width="2085" height="734" alt="gantt_sjf" src="https://github.com/user-attachments/assets/24291372-eeed-4754-9c7f-4b2a8eba9ee2" />
Round Robin Scheduling
<img width="2085" height="734" alt="gantt_rr" src="https://github.com/user-attachments/assets/95602ae9-0d41-4377-a929-66576d5e81ba" />
<img width="2085" height="734" alt="gantt_priority" src="https://github.com/user-attachments/assets/87ba1036-55d5-45da-ba9d-376b85892c05" />
FCFS Gantt Chart
<img width="2085" height="734" alt="gantt_fcfs" src="https://github.com/user-attachments/assets/e6acc512-0f15-4914-a22f-215d7ce61271" />
 Scheduling Comparison Charts
<img width="2384" height="892" alt="comparison_charts" src="https://github.com/user-attachments/assets/a9d1944d-a751-47ee-913e-6446524c574d" />


 

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

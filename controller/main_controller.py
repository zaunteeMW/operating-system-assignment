# ═══════════════════════════════════════════════════════════
# EduOS Main Controller — Integration Bridge
# Student: Laurent Namacha | Reg: 25311351030
# Module: 351 CS 2104 — Operating Systems
# ═══════════════════════════════════════════════════════════

import subprocess
import json
import os
import sys
import time
import datetime

# ─── Path setup ───
BASE_DIR      = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
C_BINARY      = os.path.join(BASE_DIR, "c_core", "eduos")
SCHEDULER_DIR = os.path.join(BASE_DIR, "python_scheduler")
SNAPSHOT_FILE = os.path.join(BASE_DIR, "c_core", "pcb_snapshot.json")
REPORT_FILE   = os.path.join(BASE_DIR, "simulation_report.json")

sys.path.insert(0, SCHEDULER_DIR)


# ═══════════════════════════════════════════════════════════
# STEP 1 — LAUNCH C SIMULATOR
# ═══════════════════════════════════════════════════════════

def run_c_simulator():
    """
    Launch the compiled C binary using subprocess.
    Capture stdout in real time and print each line.
    """
    print("\n" + "═"*55)
    print(" STEP 1: Launching EduOS C Simulator")
    print("═"*55)

    if not os.path.exists(C_BINARY):
        print(f"[Controller] ERROR: C binary not found at {C_BINARY}")
        print("[Controller] Please run 'make all' in c_core/ first")
        print("[Controller] Continuing with existing snapshot if available")
        return False

    try:
        process = subprocess.Popen(
            [C_BINARY],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            cwd=os.path.join(BASE_DIR, "c_core")
        )

        # Read stdout in real time
        print("[Controller] C Simulator output:")
        print("-" * 40)
        for line in iter(process.stdout.readline, ""):
            print(f"  [C] {line}", end="")

        process.wait()
        print("-" * 40)

        if process.returncode == 0:
            print("[Controller] C Simulator finished successfully")
            return True
        else:
            stderr = process.stderr.read()
            print(f"[Controller] C Simulator error: {stderr}")
            return False

    except Exception as e:
        print(f"[Controller] Failed to run C simulator: {e}")
        return False


# ═══════════════════════════════════════════════════════════
# STEP 2 — WAIT FOR PCB SNAPSHOT
# ═══════════════════════════════════════════════════════════

def wait_for_snapshot(timeout=30):
    """
    Monitor pcb_snapshot.json and wait until
    all processes show TERMINATED state.
    """
    print("\n" + "═"*55)
    print(" STEP 2: Monitoring PCB Snapshot")
    print("═"*55)

    # If no snapshot exists create a default one for testing
    if not os.path.exists(SNAPSHOT_FILE):
        print("[Controller] No snapshot found — creating sample data")
        sample = [
            {
                "pid": 1, "name": "init",
                "state": "TERMINATED", "priority": 0,
                "burst_time": 10, "arrival_time": 0,
                "remaining_time": 0, "memory_req_kb": 512,
                "thread_count": 1, "exit_code": 0,
                "parent_pid": 0, "owner_id": 42,
                "creation_time": int(time.time())
            },
            {
                "pid": 2, "name": "calculator",
                "state": "TERMINATED", "priority": 1,
                "burst_time": 5, "arrival_time": 1,
                "remaining_time": 0, "memory_req_kb": 256,
                "thread_count": 1, "exit_code": 0,
                "parent_pid": 1, "owner_id": 42,
                "creation_time": int(time.time())
            },
            {
                "pid": 3, "name": "file_manager",
                "state": "TERMINATED", "priority": 2,
                "burst_time": 8, "arrival_time": 2,
                "remaining_time": 0, "memory_req_kb": 384,
                "thread_count": 2, "exit_code": 0,
                "parent_pid": 1, "owner_id": 42,
                "creation_time": int(time.time())
            },
            {
                "pid": 4, "name": "text_editor",
                "state": "TERMINATED", "priority": 3,
                "burst_time": 6, "arrival_time": 3,
                "remaining_time": 0, "memory_req_kb": 128,
                "thread_count": 1, "exit_code": 0,
                "parent_pid": 1, "owner_id": 42,
                "creation_time": int(time.time())
            },
            {
                "pid": 5, "name": "browser",
                "state": "TERMINATED", "priority": 2,
                "burst_time": 9, "arrival_time": 4,
                "remaining_time": 0, "memory_req_kb": 512,
                "thread_count": 3, "exit_code": 0,
                "parent_pid": 1, "owner_id": 42,
                "creation_time": int(time.time())
            },
        ]
        with open(SNAPSHOT_FILE, "w") as f:
            json.dump(sample, f, indent=2)
        print("[Controller] Sample snapshot created")
        return True

    # Check if all processes terminated
    start = time.time()
    while time.time() - start < timeout:
        try:
            with open(SNAPSHOT_FILE) as f:
                data = json.load(f)
            all_done = all(
                p.get("state") == "TERMINATED"
                for p in data
            )
            if all_done:
                print(f"[Controller] All {len(data)} processes TERMINATED")
                return True
            else:
                running = [
                    p["pid"] for p in data
                    if p.get("state") != "TERMINATED"
                ]
                print(f"[Controller] Still running: PIDs {running}")
                time.sleep(1)
        except Exception as e:
            print(f"[Controller] Error reading snapshot: {e}")
            time.sleep(1)

    print("[Controller] Timeout waiting for processes to terminate")
    return False


# ═══════════════════════════════════════════════════════════
# STEP 3 — RUN PYTHON SCHEDULER
# ═══════════════════════════════════════════════════════════

def run_scheduler(quantum=3):
    """
    Load PCB snapshot and run all 4 scheduling algorithms.
    Returns results dictionary.
    """
    print("\n" + "═"*55)
    print(" STEP 3: Running Python Scheduler")
    print("═"*55)

    try:
        from scheduler_sim import (
            load_from_json,
            run_all,
            print_comparison_table
        )

        # Load processes from C output
        processes = load_from_json(SNAPSHOT_FILE)
        print(f"[Controller] Loaded {len(processes)} processes from snapshot")

        # Run all 4 algorithms
        all_results = run_all(processes, quantum)

        # Print comparison
        print_comparison_table(all_results)

        return all_results, processes

    except Exception as e:
        print(f"[Controller] Scheduler error: {e}")
        import traceback
        traceback.print_exc()
        return None, None


# ═══════════════════════════════════════════════════════════
# STEP 4 — GENERATE CHARTS
# ═══════════════════════════════════════════════════════════

def generate_charts(all_results, processes):
    """Generate all Gantt and comparison charts."""
    print("\n" + "═"*55)
    print(" STEP 4: Generating Charts")
    print("═"*55)

    try:
        from gantt import generate_all_charts
        generate_all_charts(all_results, processes)
        print("[Controller] All charts generated successfully")
    except Exception as e:
        print(f"[Controller] Chart generation error: {e}")


# ═══════════════════════════════════════════════════════════
# STEP 5 — GENERATE SUMMARY REPORT
# ═══════════════════════════════════════════════════════════

def generate_report(all_results):
    """
    Write timestamped simulation_report.json
    containing metrics from all four algorithms.
    """
    print("\n" + "═"*55)
    print(" STEP 5: Generating Summary Report")
    print("═"*55)

    if not all_results:
        print("[Controller] No results to report")
        return

    report = {
        "timestamp":    datetime.datetime.now().isoformat(),
        "student":      "Hajj Sulaiman",
        "reg_number":   "25311351025",
        "module":       "351 CS 2104 — Operating Systems",
        "algorithms":   {}
    }

    for algo, data in all_results.items():
        report["algorithms"][algo] = {
            "aggregate": data["aggregate"],
            "schedule":  data["schedule"],
        }

    # Find best algorithm by lowest avg waiting time
    best = min(
        all_results.keys(),
        key=lambda a: all_results[a]["aggregate"]["avg_waiting_time"]
    )
    report["best_algorithm"] = {
        "name":   best,
        "reason": "Lowest average waiting time",
        "avg_waiting_time": all_results[best]["aggregate"]["avg_waiting_time"]
    }

    with open(REPORT_FILE, "w") as f:
        json.dump(report, f, indent=2)

    print(f"[Controller] Report saved to {REPORT_FILE}")
    print(f"[Controller] Best algorithm: {best}")
    print(f"[Controller] Avg waiting time: "
          f"{all_results[best]['aggregate']['avg_waiting_time']}")


# ═══════════════════════════════════════════════════════════
# MAIN
# ═══════════════════════════════════════════════════════════

def main():
    print("\n")
    print("╔═══════════════════════════════════════════════════╗")
    print("║         EduOS Main Controller Starting            ║")
    print("║         Hajj Sulaiman | 25311351025               ║")
    print("║         351 CS 2104 — Operating Systems           ║")
    print("╚═══════════════════════════════════════════════════╝")

    start_time = time.time()

    # Step 1: Run C simulator
    c_success = run_c_simulator()

    # Step 2: Wait for snapshot
    snapshot_ready = wait_for_snapshot()

    if not snapshot_ready:
        print("[Controller] Could not get PCB snapshot — exiting")
        sys.exit(1)

    # Step 3: Run Python scheduler
    all_results, processes = run_scheduler(quantum=3)

    if all_results is None:
        print("[Controller] Scheduler failed — exiting")
        sys.exit(1)

    # Step 4: Generate charts
    generate_charts(all_results, processes)

    # Step 5: Generate report
    generate_report(all_results)

    # Done
    elapsed = time.time() - start_time
    print("\n")
    print("╔═══════════════════════════════════════════════════╗")
    print("║           EduOS Simulation Complete!              ║")
    print(f"║           Total time: {elapsed:.2f} seconds"
          + " " * (26 - len(f"{elapsed:.2f}")) + "║")
    print("║           Check docs/screenshots/ for charts      ║")
    print("║           Check simulation_report.json for data   ║")
    print("╚═══════════════════════════════════════════════════╝\n")


if __name__ == "__main__":
    main()

# ═══════════════════════════════════════════════════════════
# EduOS Scheduling Simulator
# Student: Laurent Namacha | Reg: 25311351030
# Module: 351 CS 2104 — Operating Systems
# ═══════════════════════════════════════════════════════════

import argparse
import random
import json
import csv
import os
from copy import deepcopy
from tabulate import tabulate
from rich.console import Console
from rich.table import Table

console = Console()

# ═══════════════════════════════════════════════════════════
# HELPER FUNCTIONS
# ═══════════════════════════════════════════════════════════

def make_process(pid, name, arrival, burst, priority, memory=256):
    """Create a process dictionary."""
    return {
        "pid":            pid,
        "name":           name,
        "arrival_time":   arrival,
        "burst_time":     burst,
        "priority":       priority,
        "memory_req_kb":  memory,
        "remaining_time": burst,
        "start_time":     -1,
        "completion_time": 0,
        "waiting_time":   0,
        "turnaround_time": 0,
        "response_time":  -1,
    }

def calculate_metrics(processes, schedule):
    """Calculate per-process and aggregate metrics."""
    results = []
    total_time = schedule[-1][2] if schedule else 0

    for p in processes:
        pid = p["pid"]
        # Find first and last slot for this process
        slots = [(s, e) for (pp, s, e) in schedule if pp == pid]
        if not slots:
            continue

        start        = slots[0][0]
        completion   = slots[-1][1]
        turnaround   = completion - p["arrival_time"]
        waiting      = turnaround - p["burst_time"]
        response     = start - p["arrival_time"]

        results.append({
            "pid":             pid,
            "name":            p["name"],
            "arrival_time":    p["arrival_time"],
            "burst_time":      p["burst_time"],
            "completion_time": completion,
            "turnaround_time": turnaround,
            "waiting_time":    waiting,
            "response_time":   response,
        })

    if not results:
        return results, {}

    avg_wt  = sum(r["waiting_time"]    for r in results) / len(results)
    avg_tat = sum(r["turnaround_time"] for r in results) / len(results)
    avg_rt  = sum(r["response_time"]   for r in results) / len(results)

    # CPU utilisation
    busy_time = sum(e - s for (_, s, e) in schedule)
    cpu_util  = (busy_time / total_time * 100) if total_time > 0 else 0
    throughput = len(results) / total_time if total_time > 0 else 0

    aggregate = {
        "avg_waiting_time":    round(avg_wt,  2),
        "avg_turnaround_time": round(avg_tat, 2),
        "avg_response_time":   round(avg_rt,  2),
        "cpu_utilisation":     round(cpu_util, 2),
        "throughput":          round(throughput, 4),
    }

    return results, aggregate


def print_results(algo_name, results, aggregate):
    """Print a rich formatted results table."""
    console.print(f"\n[bold cyan]{'═'*60}[/bold cyan]")
    console.print(f"[bold yellow] Algorithm: {algo_name}[/bold yellow]")
    console.print(f"[bold cyan]{'═'*60}[/bold cyan]")

    table = Table(show_header=True, header_style="bold magenta")
    table.add_column("PID",         style="cyan",  width=6)
    table.add_column("Name",        style="white", width=16)
    table.add_column("Arrival",     style="green", width=8)
    table.add_column("Burst",       style="green", width=8)
    table.add_column("Completion",  style="yellow",width=12)
    table.add_column("TAT",         style="yellow",width=8)
    table.add_column("WT",          style="red",   width=8)
    table.add_column("RT",          style="blue",  width=8)

    for r in results:
        table.add_row(
            str(r["pid"]),
            r["name"],
            str(r["arrival_time"]),
            str(r["burst_time"]),
            str(r["completion_time"]),
            str(r["turnaround_time"]),
            str(r["waiting_time"]),
            str(r["response_time"]),
        )

    console.print(table)

    console.print(f"\n[bold green]Aggregate Metrics:[/bold green]")
    console.print(f"  Average Waiting Time:    [red]{aggregate['avg_waiting_time']}[/red]")
    console.print(f"  Average Turnaround Time: [yellow]{aggregate['avg_turnaround_time']}[/yellow]")
    console.print(f"  Average Response Time:   [blue]{aggregate['avg_response_time']}[/blue]")
    console.print(f"  CPU Utilisation:         [green]{aggregate['cpu_utilisation']}%[/green]")
    console.print(f"  Throughput:              [cyan]{aggregate['throughput']} proc/unit[/cyan]")


# ═══════════════════════════════════════════════════════════
# ALGORITHM 1: FIRST COME FIRST SERVED (FCFS)
# ═══════════════════════════════════════════════════════════

def fcfs(processes):
    """
    First Come First Served — Non-preemptive.
    Sort by arrival_time, break ties by lower PID first.
    Returns schedule: list of [pid, start, end] tuples.
    """
    procs    = sorted(deepcopy(processes),
                      key=lambda p: (p["arrival_time"], p["pid"]))
    schedule = []
    time     = 0

    for p in procs:
        if time < p["arrival_time"]:
            time = p["arrival_time"]   # CPU idle gap
        start = time
        end   = time + p["burst_time"]
        schedule.append((p["pid"], start, end))
        time  = end

    return schedule


# ═══════════════════════════════════════════════════════════
# ALGORITHM 2: SHORTEST JOB FIRST (SJF)
# ═══════════════════════════════════════════════════════════

def sjf(processes):
    """
    Shortest Job First — Non-preemptive.
    At each decision point pick shortest burst.
    Ties broken by arrival_time then PID.
    """
    procs    = sorted(deepcopy(processes),
                      key=lambda p: (p["arrival_time"], p["pid"]))
    schedule = []
    time     = 0
    done     = []
    remaining = procs[:]

    while remaining:
        # Get all processes that have arrived
        available = [p for p in remaining if p["arrival_time"] <= time]

        if not available:
            time = remaining[0]["arrival_time"]
            continue

        # Pick shortest burst, break ties by arrival then PID
        chosen = min(available,
                     key=lambda p: (p["burst_time"],
                                    p["arrival_time"],
                                    p["pid"]))
        remaining.remove(chosen)

        start = time
        end   = time + chosen["burst_time"]
        schedule.append((chosen["pid"], start, end))
        time  = end

    return schedule


# ═══════════════════════════════════════════════════════════
# ALGORITHM 3: PRIORITY SCHEDULING WITH AGEING
# ═══════════════════════════════════════════════════════════

def priority_scheduling(processes):
    """
    Priority Scheduling — Non-preemptive.
    Lower priority number = higher urgency.
    Ageing: every 3 time units waiting, priority improves by 1.
    This prevents starvation of low-priority processes.
    """
    procs     = deepcopy(processes)
    for p in procs:
        p["wait_since"] = p["arrival_time"]

    schedule  = []
    time      = 0
    completed = []
    remaining = sorted(procs, key=lambda p: p["arrival_time"])

    while remaining:
        available = [p for p in remaining if p["arrival_time"] <= time]

        if not available:
            time = remaining[0]["arrival_time"]
            continue

        # Apply ageing — every 3 units waiting, improve priority by 1
        for p in available:
            waited = time - p["wait_since"]
            age_bonus = waited // 3
            p["effective_priority"] = max(0, p["priority"] - age_bonus)

        # Pick highest priority (lowest number after ageing)
        chosen = min(available,
                     key=lambda p: (p["effective_priority"],
                                    p["arrival_time"],
                                    p["pid"]))
        remaining.remove(chosen)

        start = time
        end   = time + chosen["burst_time"]
        schedule.append((chosen["pid"], start, end))
        time  = end

        # Update wait_since for remaining processes
        for p in remaining:
            if p["arrival_time"] <= time and p["wait_since"] == p["arrival_time"]:
                p["wait_since"] = time

    return schedule


# ═══════════════════════════════════════════════════════════
# ALGORITHM 4: ROUND ROBIN
# ═══════════════════════════════════════════════════════════

def round_robin(processes, quantum=3):
    """
    Round Robin — Preemptive.
    Each process gets a time quantum. If not done,
    it goes back to the end of the ready queue.
    """
    procs     = sorted(deepcopy(processes),
                       key=lambda p: (p["arrival_time"], p["pid"]))
    schedule  = []
    time      = 0
    queue     = []
    remaining = procs[:]
    in_queue  = set()

    # Add first process
    if remaining:
        first = remaining.pop(0)
        queue.append(first)
        in_queue.add(first["pid"])

    while queue:
        p = queue.pop(0)
        in_queue.discard(p["pid"])

        start    = max(time, p["arrival_time"])
        run_time = min(quantum, p["remaining_time"])
        end      = start + run_time

        schedule.append((p["pid"], start, end))
        p["remaining_time"] -= run_time
        time = end

        # Add newly arrived processes to queue
        newly = [proc for proc in remaining
                 if proc["arrival_time"] <= time
                 and proc["pid"] not in in_queue]
        for proc in newly:
            remaining.remove(proc)
            queue.append(proc)
            in_queue.add(proc["pid"])

        # If not finished put back in queue
        if p["remaining_time"] > 0:
            queue.append(p)
            in_queue.add(p["pid"])

    return schedule


# ═══════════════════════════════════════════════════════════
# INPUT: RANDOM GENERATION
# ═══════════════════════════════════════════════════════════

def generate_random_processes(n, seed=None):
    """Generate n random processes reproducibly with seed."""
    if seed is not None:
        random.seed(seed)
    processes = []
    for i in range(1, n + 1):
        processes.append(make_process(
            pid      = i,
            name     = f"proc_{i}",
            arrival  = random.randint(0, n),
            burst    = random.randint(1, 10),
            priority = random.randint(1, 5),
            memory   = random.choice([64, 128, 256, 512]),
        ))
    return processes


# ═══════════════════════════════════════════════════════════
# INPUT: CSV FILE
# ═══════════════════════════════════════════════════════════

def load_from_csv(filepath):
    """Load processes from CSV file."""
    processes = []
    with open(filepath, newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            processes.append(make_process(
                pid      = int(row["pid"]),
                name     = row["name"],
                arrival  = int(row["arrival_time"]),
                burst    = int(row["burst_time"]),
                priority = int(row["priority"]),
                memory   = int(row.get("memory_req_kb", 256)),
            ))
    return processes


# ═══════════════════════════════════════════════════════════
# INPUT: PCB SNAPSHOT JSON FROM C PROGRAM
# ═══════════════════════════════════════════════════════════

def load_from_json(filepath):
    """Load processes from pcb_snapshot.json written by C program."""
    with open(filepath) as f:
        data = json.load(f)
    processes = []
    for p in data:
        processes.append(make_process(
            pid      = p["pid"],
            name     = p["name"],
            arrival  = p.get("arrival_time", 0),
            burst    = p["burst_time"],
            priority = p.get("priority", 1),
            memory   = p.get("memory_req_kb", 256),
        ))
    return processes


# ═══════════════════════════════════════════════════════════
# RUN ALL ALGORITHMS
# ═══════════════════════════════════════════════════════════

def run_all(processes, quantum=3):
    """Run all 4 algorithms and return results."""
    algorithms = {
        "FCFS":     fcfs(processes),
        "SJF":      sjf(processes),
        "Priority": priority_scheduling(processes),
        f"RR(q={quantum})": round_robin(processes, quantum),
    }

    all_results = {}
    for name, schedule in algorithms.items():
        results, aggregate = calculate_metrics(processes, schedule)
        print_results(name, results, aggregate)
        all_results[name] = {
            "schedule":  schedule,
            "results":   results,
            "aggregate": aggregate,
        }

    return all_results


# ═══════════════════════════════════════════════════════════
# COMPARISON TABLE
# ═══════════════════════════════════════════════════════════

def print_comparison_table(all_results):
    """Print side by side comparison of all algorithms."""
    console.print("\n[bold cyan]═══ ALGORITHM COMPARISON TABLE ═══[/bold cyan]")

    headers = ["Metric"] + list(all_results.keys())
    rows = []

    metrics = [
        ("avg_waiting_time",    "Avg Waiting Time"),
        ("avg_turnaround_time", "Avg Turnaround Time"),
        ("avg_response_time",   "Avg Response Time"),
        ("cpu_utilisation",     "CPU Utilisation (%)"),
        ("throughput",          "Throughput (proc/unit)"),
    ]

    for key, label in metrics:
        row = [label]
        for algo in all_results:
            row.append(all_results[algo]["aggregate"].get(key, "N/A"))
        rows.append(row)

    print(tabulate(rows, headers=headers, tablefmt="fancy_grid"))


# ═══════════════════════════════════════════════════════════
# MAIN — COMMAND LINE INTERFACE
# ═══════════════════════════════════════════════════════════

def main():
    parser = argparse.ArgumentParser(
        description="EduOS CPU Scheduling Simulator"
    )
    parser.add_argument("--random",  type=int,
                        help="Generate N random processes")
    parser.add_argument("--seed",    type=int, default=42,
                        help="Random seed for reproducibility")
    parser.add_argument("--file",    type=str,
                        help="Load processes from CSV file")
    parser.add_argument("--json",    type=str,
                        help="Load processes from JSON file")
    parser.add_argument("--quantum", type=int, default=3,
                        help="Time quantum for Round Robin")
    parser.add_argument("--mode",    type=str, default="process",
                        choices=["process", "thread"],
                        help="Scheduling mode")
    args = parser.parse_args()

    # Load processes
    if args.random:
        processes = generate_random_processes(args.random, args.seed)
        console.print(f"[green]Generated {args.random} random processes "
                      f"(seed={args.seed})[/green]")
    elif args.file:
        processes = load_from_csv(args.file)
        console.print(f"[green]Loaded {len(processes)} processes "
                      f"from {args.file}[/green]")
    elif args.json:
        processes = load_from_json(args.json)
        console.print(f"[green]Loaded {len(processes)} processes "
                      f"from {args.json}[/green]")
    else:
        # Default: use sample processes
        processes = generate_random_processes(5, seed=42)
        console.print("[yellow]No input specified — using 5 default "
                      "processes[/yellow]")

    console.print(f"[cyan]Mode: {args.mode} | "
                  f"Quantum: {args.quantum}[/cyan]\n")

    # Run all algorithms
    all_results = run_all(processes, args.quantum)

    # Print comparison table
    print_comparison_table(all_results)

    # Save results to JSON
    output = {}
    for algo, data in all_results.items():
        output[algo] = {
            "aggregate": data["aggregate"],
            "schedule":  data["schedule"],
        }

    with open("scheduling_results.json", "w") as f:
        json.dump(output, f, indent=2)

    console.print("\n[green]Results saved to scheduling_results.json[/green]")

    # Generate charts
    try:
        from gantt import generate_all_charts
        generate_all_charts(all_results, processes)
        console.print("[green]Charts saved to docs/screenshots/[/green]")
    except ImportError:
        console.print("[yellow]gantt.py not found — skipping charts[/yellow]")


if __name__ == "__main__":
    main()

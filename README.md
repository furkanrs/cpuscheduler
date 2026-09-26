# CPU Scheduling Simulator

A C++ implementation of a CPU scheduling simulator that supports multiple scheduling algorithms and produces either execution timelines or scheduling statistics.

## Implemented Algorithms

The simulator currently supports:

1. **FCFS** — First Come First Serve
2. **Round Robin (RR)** — Configurable time quantum
3. **SRT** — Shortest Remaining Time
4. **HRRN** — Highest Response Ratio Next
5. **FB-1** — Feedback scheduling with quantum 1
6. **FB-2i** — Feedback scheduling with exponentially increasing quantum

## Features

* Multiple CPU scheduling algorithms in one simulator
* Preemptive and non-preemptive scheduling
* Configurable Round Robin quantum
* Multi-level feedback queue scheduling
* Execution timeline visualization
* Scheduling statistics
* Multiple input/output test cases

## Project Structure

```text
cpuscheduler/
│
├── README.md
├── .gitignore
├── main.cpp
├── parser.h
│
└── testcases/
    ├── basic.txt
    ├── testcase1.txt
    ├── testcase1.op
    ├── testcase2.txt
    ├── testcase2.op
    ├── testcase3.txt
    ├── testcase3.op
    ├── testcase4.txt
    ├── testcase4.op
    ├── testcase5.txt
    ├── testcase5.op
    ├── testcase6.txt
    └── testcase6.op
```

### `parser.h`

Responsible for parsing the simulation input and storing:

* Algorithm specifications
* Process information
* Simulation mode
* Last simulation instant
* Timeline data

### `main.cpp`

Contains:

* Scheduling algorithms
* Scheduling helper functions
* Statistics calculation
* Timeline generation
* Trace output
* Statistics output
* Algorithm dispatch

## Input Format

The input consists of:

```text
mode
algorithm_list
last_instant
number_of_processes
process_1
process_2
...
```

### 1. Mode

The simulator supports two modes:

```text
trace
```

or

```text
stats
```

### 2. Algorithm List

Algorithms are specified using their numeric IDs:

```text
1,2-2,3,4,5,6
```

The IDs are:

```text
1     -> FCFS
2-q   -> Round Robin with quantum q
3     -> SRT
4     -> HRRN
5     -> FB-1
6     -> FB-2i
```

For example:

```text
1,2-2,3,4,5,6
```

runs all six algorithms, with Round Robin using a quantum of 2.

### 3. Last Instant

The next value specifies the final simulation instant.

For example:

```text
20
```

### 4. Number of Processes

The next value specifies how many processes are present.

For example:

```text
4
```

### 5. Process Format

Each process is represented as:

```text
process_name,arrival_time,service_time
```

Example:

```text
P1,0,5
P2,0,3
P3,0,8
P4,0,2
```

## Example Input

```text
trace
1,2-2,3,4,5,6
20
4
P1,0,5
P2,0,3
P3,0,8
P4,0,2
```

This runs:

* FCFS
* Round Robin with quantum 2
* SRT
* HRRN
* FB-1
* FB-2i

and produces a timeline for each algorithm.

## Output Modes

### Trace Mode

`trace` displays the execution timeline of every process.

Example structure:

```text
FCFS
0 1 2 3 4 5 ...

P1    |*|*|*|*|*| |
P2    |.|.|.|.|.|*|
...
```

Where:

* `*` represents CPU execution
* `.` represents waiting time after arrival
* Blank positions represent time before a process arrives or unused timeline positions

### Stats Mode

`stats` displays scheduling statistics for each algorithm.

The output includes:

* Arrival time
* Service time
* Finish time
* Turnaround time
* Normalized turnaround time
* Mean turnaround time
* Mean normalized turnaround time

Example structure:

```text
FCFS
Process    |  P1  |  P2  |
Arrival    |  0   |  1   |
Service    |  5   |  3   |
Finish     |  5   |  8   |
Turnaround |  5   |  7   |
NormTurn   | 1.00 | 2.33 |
```

## Scheduling Algorithms

### FCFS

Processes are executed in input/arrival order without preemption.

### Round Robin

Each ready process receives a fixed CPU time slice. If it does not finish within the quantum, it returns to the ready queue.

Example:

```text
2-2
```

means Round Robin with a quantum of 2.

### SRT

Shortest Remaining Time is a preemptive scheduling algorithm. At each time unit, the process with the smallest remaining service time is selected.

### HRRN

Highest Response Ratio Next is non-preemptive.

The response ratio is:

```text
(wait time + service time) / service time
```

The process with the highest response ratio is selected.

### FB-1

Feedback scheduling using multiple priority levels.

A process that does not finish during its current quantum moves to the next level. FB-1 uses:

```text
quantum = 1
```

at every level.

### FB-2i

Uses the same feedback scheduling structure as FB-1, but the quantum increases with the level:

```text
quantum = 2^level
```

For example:

```text
Level 0 -> 1
Level 1 -> 2
Level 2 -> 4
Level 3 -> 8
...
```

## Building

Compile the project using `g++`:

```powershell
g++ main.cpp -o cpuscheduler.exe
```

## Running

In PowerShell, input can be piped into the executable.

For example:

```powershell
Get-Content testcases\testcase1.txt | .\cpuscheduler.exe
```

To save the generated output:

```powershell
Get-Content testcases\testcase1.txt | .\cpuscheduler.exe > testcases\testcase1.op
```

The same procedure can be used for the other test cases.

## Test Cases

The `testcases` directory contains six demonstration inputs along with their generated outputs.

The test cases cover different scheduling situations, including:

* Processes arriving at the same time
* Staggered process arrivals
* Short and long CPU bursts
* Initial CPU idle periods
* Larger mixed workloads
* Both `trace` and `stats` output modes

Each demonstration test runs all six implemented scheduling algorithms.

## Technologies

* **C++**
* **STL**
* **Data Structures**
* **CPU Scheduling Algorithms**

#include <bits/stdc++.h>
#include "parser.h"
using namespace std;

static void clearTimeline(SimulationInput &sim) {
    for (auto &row : sim.timeline)
        fill(row.begin(), row.end(), ' ');
}

static void fillWaitGaps(SimulationInput &sim) {
    for (auto &p : sim.processes)
        for (int t = p.arrival; t < p.finish && t < sim.lastInstant; t++)
            if (sim.timeline[t][p.index] != '*')
                sim.timeline[t][p.index] = '.';
}

static void recordStats(Process &p, int finishTime) {
    p.finish     = finishTime;
    p.turnaround = finishTime - p.arrival;
    p.normTurn   = (float)p.turnaround / p.value;
}

static vector<Process *> byInputOrder(SimulationInput &sim) {
    vector<Process *> ptrs(sim.processes.size());
    for (auto &p : sim.processes) ptrs[p.index] = &p;
    return ptrs;
}

// 1. FCFS
static void fcfs(SimulationInput &sim) {
    int clock = sim.processes[0].arrival;
    for (auto *p : byInputOrder(sim)) {
        int start = max(clock, p->arrival);
        for (int t = p->arrival; t < start; t++)
            sim.timeline[t][p->index] = '.';
        for (int t = start; t < start + p->value; t++)
            sim.timeline[t][p->index] = '*';
        recordStats(*p, start + p->value);
        clock = p->finish;
    }
}

// 2. Round Robin
static void roundRobin(SimulationInput &sim, int quantum) {
    int n = (int)sim.processes.size();
    vector<int> remaining(n);
    for (int i = 0; i < n; i++) remaining[i] = sim.processes[i].value;

    deque<int> ready;
    int nextIn = 0;
    int t = 0;

    while (nextIn < n && sim.processes[nextIn].arrival <= t)
        ready.push_back(nextIn++);

    while (t < sim.lastInstant) {
        if (ready.empty()) {
            t++;
            while (nextIn < n && sim.processes[nextIn].arrival <= t)
                ready.push_back(nextIn++);
            continue;
        }

        int current = ready.front();
        ready.pop_front();
        int used = 0;

        while (used < quantum && remaining[current] > 0 && t < sim.lastInstant) {
            sim.timeline[t][sim.processes[current].index] = '*';
            remaining[current]--;
            used++;
            t++;
            while (nextIn < n && sim.processes[nextIn].arrival <= t)
                ready.push_back(nextIn++);
        }

        if (remaining[current] == 0)
            recordStats(sim.processes[current], t);
        else
            ready.push_back(current);
    }
    fillWaitGaps(sim);
}

// Dispatch
static void runAlgorithm(SimulationInput &sim, const AlgorithmSpec &spec) {
    switch (spec.id) {
        case 1: fcfs(sim);                    break;
        case 2: roundRobin(sim, spec.quantum); break;
        default:
            cerr << "Algorithm " << spec.id << " not implemented.\n";
            break;
    }
}

// Output
static const string ALGO_NAMES[3] = {"", "FCFS", "RR-"};

static string algoLabel(const AlgorithmSpec &s) {
    if (s.id == 2) return "RR-" + to_string(s.quantum);
    return ALGO_NAMES[s.id];
}

static void printTimeline(SimulationInput &sim, const AlgorithmSpec &spec) {
    cout << algoLabel(spec) << "\n";
    for (int i = 0; i <= sim.lastInstant; i++) cout << i % 10 << ' ';
    cout << "\n" << string(50, '-') << "\n";
    for (auto *p : byInputOrder(sim)) {
        cout << left << setw(6) << p->name << "|";
        for (int t = 0; t < sim.lastInstant; t++)
            cout << sim.timeline[t][p->index] << '|';
        cout << "\n";
    }
    cout << string(50, '-') << "\n";
}

static void printStats(SimulationInput &sim, const AlgorithmSpec &spec) {
    auto procs = byInputOrder(sim);
    cout << algoLabel(spec) << "\n";

    cout << "Process    ";
    for (auto *p : procs) cout << "|  " << p->name << "  ";
    cout << "|\n";

    cout << "Arrival    ";
    for (auto *p : procs) printf("|%3d  ", p->arrival);
    cout << "|\n";

    cout << "Service    |";
    for (auto *p : procs) printf("%3d  |", p->value);
    cout << " Mean|\n";

    cout << "Finish     ";
    for (auto *p : procs) printf("|%3d  ", p->finish);
    cout << "|-----|\n";

    cout << "Turnaround |";
    float tSum = 0;
    for (auto *p : procs) { printf("%3d  |", p->turnaround); tSum += p->turnaround; }
    float tMean = tSum / procs.size();
    tMean >= 10 ? printf("%2.2f|\n", tMean) : printf(" %2.2f|\n", tMean);

    cout << "NormTurn   |";
    float nSum = 0;
    for (auto *p : procs) {
        p->normTurn >= 10 ? printf("%2.2f|", p->normTurn) : printf(" %2.2f|", p->normTurn);
        nSum += p->normTurn;
    }
    float nMean = nSum / procs.size();
    nMean >= 10 ? printf("%2.2f|\n", nMean) : printf(" %2.2f|\n", nMean);
}

int main() {
    SimulationInput sim = parseInput();

    for (auto &spec : sim.algorithms) {
        clearTimeline(sim);
        for (auto &p : sim.processes) { p.finish = 0; p.turnaround = 0; p.normTurn = 0.0f; }

        runAlgorithm(sim, spec);

        if (sim.mode == "trace")      printTimeline(sim, spec);
        else if (sim.mode == "stats") printStats(sim, spec);
        cout << "\n";
    }
    return 0;
}
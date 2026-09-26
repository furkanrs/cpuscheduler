#include <bits/stdc++.h>
#include "parser.h"
using namespace std;
// ─────────────────────────────────────────────────────────────────────────────
// Shared helpers
// ─────────────────────────────────────────────────────────────────────────────

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

// ─────────────────────────────────────────────────────────────────────────────
// 1. FCFS — runs each process to completion in arrival order, no preemption
// ─────────────────────────────────────────────────────────────────────────────
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

// ─────────────────────────────────────────────────────────────────────────────
// 2. Round Robin — each process gets a fixed time slice (quantum), then yields
// ─────────────────────────────────────────────────────────────────────────────
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
// ─────────────────────────────────────────────────────────────────────────────
// 3. SRT — preemptive; at each tick, runs the process with least remaining time
// ─────────────────────────────────────────────────────────────────────────────
static void srt(SimulationInput &sim) {
    int n = (int)sim.processes.size();
    vector<int> remaining(n);
    for (int i = 0; i < n; i++) remaining[i] = sim.processes[i].value;

    for (int t = 0; t < sim.lastInstant; t++) {
        int best = -1;
        for (int i = 0; i < n; i++) {
            if (sim.processes[i].arrival > t || remaining[i] <= 0) continue;
            if (best == -1 || remaining[i] < remaining[best]) best = i;
        }
        if (best == -1) continue;

        sim.timeline[t][sim.processes[best].index] = '*';
        remaining[best]--;
        if (remaining[best] == 0)
            recordStats(sim.processes[best], t + 1);
    }
    fillWaitGaps(sim);
}
// ─────────────────────────────────────────────────────────────────────────────
// 4. HRRN — non-preemptive; picks process with highest (wait+service)/service
//           ratio, preventing starvation of long processes
// ─────────────────────────────────────────────────────────────────────────────
static void hrrn(SimulationInput &sim) {
    int n = (int)sim.processes.size();
    vector<bool> done(n, false);
    int clock = 0, completed = 0;

    while (completed < n) {
        int best = -1;
        double bestRatio = -1.0;
        for (int i = 0; i < n; i++) {
            if (done[i] || sim.processes[i].arrival > clock) continue;
            int wait = clock - sim.processes[i].arrival;
            double ratio = (wait + sim.processes[i].value) * 1.0 / sim.processes[i].value;
            if (ratio > bestRatio) { bestRatio = ratio; best = i; }
        }
        if (best == -1) { clock++; continue; }

        Process &p = sim.processes[best];
        for (int t = p.arrival; t < clock; t++)
            sim.timeline[t][p.index] = '.';
        for (int t = clock; t < clock + p.value; t++)
            sim.timeline[t][p.index] = '*';
        recordStats(p, clock + p.value);
        clock = p.finish;
        done[best] = true;
        completed++;
    }
}
// ─────────────────────────────────────────────────────────────────────────────
// 5 & 6. Feedback — shared engine, quantumAt(level) controls burst length
// ─────────────────────────────────────────────────────────────────────────────
static void feedback(SimulationInput &sim, const function<int(int)> &quantumAt) {
    int n = (int)sim.processes.size();
    vector<int> remaining(n);
    for (int i = 0; i < n; i++) remaining[i] = sim.processes[i].value;
 
    vector<deque<int>> levels(1);   // multi-level FIFO queues
    int nextIn = 0;
 
    auto admitArrivals = [&](int upTo) {
        while (nextIn < n && sim.processes[nextIn].arrival <= upTo) {
            levels[0].push_back(nextIn);
            nextIn++;
        }
    };
 
    int t = 0;
    while (t < sim.lastInstant) {
        admitArrivals(t);
 
        int lvl = -1;
        for (int L = 0; L < (int)levels.size(); L++)
            if (!levels[L].empty()) { lvl = L; break; }
        if (lvl == -1) { t++; continue; }
 
        int idx   = levels[lvl].front(); levels[lvl].pop_front();
        int slice = quantumAt(lvl);
        int ran   = 0;
 
        while (ran < slice && remaining[idx] > 0 && t < sim.lastInstant) {
            sim.timeline[t][sim.processes[idx].index] = '*';
            remaining[idx]--;
            ran++; t++;
            admitArrivals(t);
        }
 
        if (remaining[idx] == 0) {
            recordStats(sim.processes[idx], t);
        } else {
            int next = lvl + 1;
            if (next >= (int)levels.size()) levels.emplace_back();
            levels[next].push_back(idx);
        }
    }
    fillWaitGaps(sim);
}
 
static void feedbackQ1 (SimulationInput &sim) { feedback(sim, [](int)    { return 1; }); }
static void feedbackQ2i(SimulationInput &sim) { feedback(sim, [](int lvl){ return 1 << lvl; }); }


// Dispatch
static void runAlgorithm(SimulationInput &sim, const AlgorithmSpec &spec) {
    switch (spec.id) {
        case 1: fcfs(sim);                     break;
        case 2: roundRobin(sim, spec.quantum); break;
        case 3: srt(sim);                      break;
        case 4: hrrn(sim);                     break;
        case 5: feedbackQ1(sim);               break;
        case 6: feedbackQ2i(sim);              break;
        default:
            cerr << "Algorithm " << spec.id << " not implemented.\n";
            break;
    }
}

// Output
static const string ALGO_NAMES[7] = {"", "FCFS", "RR-","SRT", "HRRN","FB-1", "FB-2i"};

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
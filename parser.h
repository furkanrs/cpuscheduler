#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include <bits/stdc++.h>
using namespace std;


struct AlgorithmSpec {
    int id;
    int quantum;
};

struct Process {
    string name;
    int arrival;
    int value;          
    int index;           

    
    int finish = 0;
    int turnaround = 0;
    float normTurn = 0.0f;
};

struct SimulationInput {
    string mode;                    
    vector<AlgorithmSpec> algorithms;
    int lastInstant = 0;
    vector<Process> processes;
    vector<vector<char>> timeline;   

   
};
static vector<string> splitOn(const string &s, char delim) {
    vector<string> parts;
    size_t start = 0;
    while (true) {
        size_t pos = s.find(delim, start);
        if (pos == string::npos) {
            parts.push_back(s.substr(start));
            break;
        }
        parts.push_back(s.substr(start, pos - start));
        start = pos + 1;
    }
    return parts;
}

static AlgorithmSpec parseOneAlgorithm(const string &token) {
    size_t dash = token.find('-');
    AlgorithmSpec spec;
    if (dash == string::npos) {
        spec.id = stoi(token);
        spec.quantum = -1;
    } else {
        spec.id = stoi(token.substr(0, dash));
        string q = token.substr(dash + 1);
        spec.quantum = q.empty() ? -1 : stoi(q);
    }
    return spec;
}

static vector<AlgorithmSpec> parseAlgorithmLine(const string &line) {
    vector<AlgorithmSpec> result;
    for (auto &tok : splitOn(line, ','))
        result.push_back(parseOneAlgorithm(tok));
    return result;
}

static Process parseProcessLine(const string &line, int idx) {
    vector<string> fields = splitOn(line, ',');
    Process p;
    p.name = fields[0];
    p.arrival = stoi(fields[1]);
    p.value = stoi(fields[2]);
    p.index = idx;
    return p;
}

SimulationInput parseInput() {
    SimulationInput input;
    string algoLine;

    cin >> input.mode >> algoLine >> input.lastInstant;

    int processCount;
    cin >> processCount;

    input.algorithms = parseAlgorithmLine(algoLine);

    input.processes.reserve(processCount);
    for (int i = 0; i < processCount; i++) {
        string line;
        cin >> line;
        input.processes.push_back(parseProcessLine(line, i));
    }

    input.timeline.assign(input.lastInstant, vector<char>(processCount, ' '));
    return input;
}

#endif // PARSER_H_INCLUDED
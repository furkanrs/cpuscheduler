#include <iostream>
#include "parser.h"

using namespace std;

int main()
{
    SimulationInput input = parseInput();

    cout << "Input parsed successfully\n";
    cout << "Mode: " << input.mode << '\n';
    cout << "Processes: " << input.processes.size() << '\n';
    cout << "Algorithms: " << input.algorithms.size() << '\n';

    return 0;
}
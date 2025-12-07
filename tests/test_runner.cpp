#include <iostream>
#include <iomanip>
#include "scheduler.h"

int main() {
    std::cout << "--- Scheduler Test Runner ---\n";
    
    Scheduler scheduler;
    
    // Choose Algorithm to test
    // scheduler.setAlgorithm("FCFS");
    // scheduler.setAlgorithm("SJF");
    scheduler.setAlgorithm("RR");
    scheduler.setTimeQuantum(2);
    
    std::cout << "Algorithm: RR (Q=2)\n";

    // Add Processes
    // ID, Name, Arrival, Burst, Priority
    scheduler.addProcess(1, "P1", 0, 5, 2);
    scheduler.addProcess(2, "P2", 1, 3, 1);
    scheduler.addProcess(3, "P3", 2, 1, 3);
    scheduler.addProcess(4, "P4", 4, 2, 4);

    std::cout << "Starting Simulation...\n";
    
    while (!scheduler.isFinished()) {
        std::string log = scheduler.tick();
        std::cout << log << std::endl;
        
        // Safety Break for infinite loops
        // if (scheduler.currentTime > 100) break; 
        
        // Print JSON State snippet
        // nlohmann::json state = scheduler.getStateJSON();
        // std::cout << state.dump() << "\n";
    }

    std::cout << "\n--- Final Statistics ---\n";
    nlohmann::json finalState = scheduler.getStateJSON();
    auto finished = finalState["finished"];
    
    std::cout << std::left << std::setw(10) << "ID" 
              << std::setw(15) << "Waiting Time" 
              << std::setw(15) << "Turnaround" << "\n";
              
    double totalWait = 0;
    double totalTurnaround = 0;
    
    for (auto& p : finished) {
        std::cout << std::left << std::setw(10) << p["id"] << "\t"
                  << std::setw(15) << p["waiting_time"] << "\t"
                  << std::setw(15) << p["turnaround_time"] << "\n";
        
        totalWait += (double)p["waiting_time"];
        totalTurnaround += (double)p["turnaround_time"];
    }
    
    if (finished.size() > 0) {
        std::cout << "\nAverage Waiting Time: " << totalWait / finished.size() << "\n";
        std::cout << "Average Turnaround Time: " << totalTurnaround / finished.size() << "\n";
    }

    return 0;
}

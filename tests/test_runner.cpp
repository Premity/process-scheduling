#include <iostream>
#include <iomanip>
#include <vector>
#include "scheduler.h"

// Helper to run a single test case
void runTest(std::string algo, std::string title, int quantum, std::vector<Process> processes, bool enableAging) {
    std::cout << "\n========================================\n";
    std::cout << "Testing Algorithm: " << title << "\n";
    std::cout << "Aging: " << (enableAging ? "ON" : "OFF") << "\n";
    std::cout << "========================================\n";
    
    Scheduler scheduler;
    scheduler.setAlgorithm(algo);
    scheduler.setTimeQuantum(quantum);
    scheduler.setAging(enableAging);
    
    for (const auto& p : processes) {
        scheduler.addProcess(p.id, p.name, p.arrivalTime, p.burstTime, p.priority);
    }
    
    std::cout << "Starting Simulation...\n";
    
    int maxTicks = 100; // Safety break
    while (!scheduler.isFinished() && maxTicks-- > 0) {
        std::cout << scheduler.tick() << std::endl;
    }
    
    if (maxTicks <= 0) std::cout << "WARNING: Simulation terminated due to safety limit.\n";

    std::cout << "\n--- Final Statistics ---\n";
    nlohmann::json finalState = scheduler.getStateJSON();
    auto finished = finalState["finished"];
    
    std::cout << std::left << std::setw(10) << "ID" 
              << std::setw(15) << "Waiting Time" 
              << std::setw(15) << "Turnaround" << "\n";
              
    double totalWait = 0;
    double totalTurnaround = 0;
    
    // Sort finished by ID for consistent output reading
    // (Wait, the json array order depends on finish order, let's keep it that way to see finish order)
    
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
}

int main() {
    std::cout << "--- Combined Scheduler Test Runner ---\n";
    
    // Standard Process Set 1
    // P1: Arr 0, Burst 5, Prio 2
    // P2: Arr 1, Burst 3, Prio 1
    // P3: Arr 2, Burst 1, Prio 3
    // P4: Arr 4, Burst 2, Prio 4
    std::vector<Process> set1;
    struct PData { int id; std::string name; int arr; int burst; int prio; };
    std::vector<PData> dataset1 = {
        {1, "P1", 0, 5, 2},
        {2, "P2", 1, 3, 1},
        {3, "P3", 2, 1, 3},
        {4, "P4", 4, 2, 4}
    };
    
    std::vector<Process> pSet1;
    for (auto& d : dataset1) {
        Process p; p.id = d.id; p.name = d.name; p.arrivalTime = d.arr; p.burstTime = d.burst; p.priority = d.prio; 
        p.remainingTime = d.burst; 
        pSet1.push_back(p);
    }

    // 1. FCFS
    runTest("FCFS", "First Come First Served", 2, pSet1, false);
    
    // 2. SJF (Non-Preemptive)
    runTest("SJF", "Shortest Job First (Non-Preemptive)", 2, pSet1, true);
    
    // 3. SRTF (Preemptive)
    runTest("SRTF", "Shortest Remaining Time First (Preemptive)", 2, pSet1, true);
    
    // 4. RR (Quantum 2)
    runTest("RR", "Round Robin (Q=2)", 2, pSet1, false);
    
    // 5. Priority (Preemptive) - Use a specific dataset for preemption?
    // Let's use the one from previous test for clarity.
    // P1: 0, 20, 5
    // P2: 2, 5, 1
    // P3: 4, 5, 2
    // P4: 6, 5, 3
    std::vector<PData> datasetPrio = {
        {1, "P1", 0, 20, 5},
        {2, "P2", 2, 5, 1},
        {3, "P3", 4, 5, 2},
        {4, "P4", 6, 5, 3}
    };
    std::vector<Process> pSetPrio;
    for (auto& d : datasetPrio) {
        Process p; p.id = d.id; p.name = d.name; p.arrivalTime = d.arr; p.burstTime = d.burst; p.priority = d.prio;
        p.remainingTime = d.burst;
        pSetPrio.push_back(p);
    }
    
    // 5. Priority (Aging ON)
    runTest("Priority", "Priority (Preemptive + Aging)", 2, pSetPrio, true);
    
    // 6. PriorityNP (Aging ON)
    runTest("PriorityNP", "Priority (Non-Preemptive + Aging)", 2, pSetPrio, true);

    // 7. Priority (Aging OFF)
    runTest("Priority", "Priority (Preemptive + NO Aging)", 2, pSetPrio, false);

    return 0;
}


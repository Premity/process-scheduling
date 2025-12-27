#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include "scheduler.h"

/**
 * Test result structure for validation
 */
struct TestResult {
    std::string testName;
    bool passed;
    std::string details;
    double avgWaiting;
    double avgTurnaround;
};

/**
 * Helper to create process data more easily
 */
struct PData {
    int id;
    std::string name;
    int arr;
    int burst;
    int prio;
};

/**
 * Convert PData to Process vector
 */
std::vector<Process> createProcessSet(const std::vector<PData>& data) {
    std::vector<Process> processes;
    for (const auto& d : data) {
        Process p;
        p.id = d.id;
        p.name = d.name;
        p.arrivalTime = d.arr;
        p.burstTime = d.burst;
        p.priority = d.prio;
        p.remainingTime = d.burst;
        p.originalPriority = d.prio;
        processes.push_back(p);
    }
    return processes;
}

/**
 * Run a single test case with detailed logging
 */
TestResult runTest(std::string algo, std::string title, int quantum, 
                    std::vector<Process> processes, bool enableAging, 
                    bool verbose = true) {
    TestResult result;
    result.testName = title;
    result.passed = true;
    
    if (verbose) {
        std::cout << "\n========================================\n";
        std::cout << "Test: " << title << "\n";
        std::cout << "Algorithm: " << algo << " | Quantum: " << quantum 
                  << " | Aging: " << (enableAging ? "ON" : "OFF") << "\n";
        std::cout << "========================================\n";
    }
    
    Scheduler scheduler;
    scheduler.setAlgorithm(algo);
    scheduler.setTimeQuantum(quantum);
    scheduler.setAging(enableAging);
    scheduler.setAgingThreshold(5);
    
    for (const auto& p : processes) {
        scheduler.addProcess(p.id, p.name, p.arrivalTime, p.burstTime, p.priority);
    }
    
    if (verbose) std::cout << "\n--- Execution Log ---\n";
    
    int maxTicks = 500;  // Safety limit for complex test cases
    int tickCount = 0;
    
    while (!scheduler.isFinished() && maxTicks-- > 0) {
        std::string tickLog = scheduler.tick();
        if (verbose) std::cout << tickLog << std::endl;
        tickCount++;
    }
    
    if (maxTicks <= 0) {
        if (verbose) std::cout << "\n WARNING: Simulation terminated due to safety limit.\n";
        result.passed = false;
        result.details = "Exceeded maximum ticks";
    }
    
    // Calculate statistics
    nlohmann::json finalState = scheduler.getStateJSON();
    auto finished = finalState["finished"];
    
    if (verbose) {
        std::cout << "\n--- Statistics ---\n";
        std::cout << std::left << std::setw(6) << "ID" 
                  << std::setw(20) << "Name"
                  << std::setw(12) << "Response" 
                  << std::setw(12) << "Waiting" 
                  << std::setw(15) << "Turnaround" << "\n";
        std::cout << std::string(65, '-') << "\n";
    }
    
    double totalWait = 0;
    double totalTurnaround = 0;
    double totalResponse = 0;
    
    for (auto& p : finished) {
        if (verbose) {
            std::cout << std::left << std::setw(6) << (int)p["id"] 
                      << std::setw(20) << (std::string)p["name"]
                      << std::setw(12) << (int)p["response_time"]
                      << std::setw(12) << (int)p["waiting_time"]
                      << std::setw(15) << (int)p["turnaround_time"] << "\n";
        }
        
        totalWait += (double)p["waiting_time"];
        totalTurnaround += (double)p["turnaround_time"];
        totalResponse += (double)p["response_time"];
    }
    
    int numProcesses = finished.size();
    result.avgWaiting = (numProcesses > 0) ? totalWait / numProcesses : 0;
    result.avgTurnaround = (numProcesses > 0) ? totalTurnaround / numProcesses : 0;
    double avgResponse = (numProcesses > 0) ? totalResponse / numProcesses : 0;
    
    if (verbose) {
        std::cout << std::string(65, '-') << "\n";
        std::cout << "Average Response Time: " << avgResponse << "\n";
        std::cout << "Average Waiting Time: " << result.avgWaiting << "\n";
        std::cout << "Average Turnaround Time: " << result.avgTurnaround << "\n";
        std::cout << "Total Execution Time: " << tickCount << " time units\n";
    }
    
    // Validation: Check if all processes finished
    if ((int)finished.size() != (int)processes.size()) {
        result.passed = false;
        result.details = "Not all processes completed";
    }
    
    return result;
}

/**
 * COMPREHENSIVE TEST SUITE
 * Tests all edge cases and algorithm variations
 */
int main() {
    std::cout << "╔════════════════════════════════════════╗\n";
    std::cout << "║CPU Scheduler - Comprehensive Test Suite║\n";
    std::cout << "╚════════════════════════════════════════╝\n";
    
    std::vector<TestResult> allResults;
    
    // ========================================
    // TEST SET 1: Basic Functionality
    // ========================================
    std::cout << "\n\n═══ TEST SET 1: Basic Functionality ═══\n";
    
    std::vector<PData> basicSet = {
        {1, "P1", 0, 5, 2},
        {2, "P2", 1, 3, 1},
        {3, "P3", 2, 1, 3},
        {4, "P4", 4, 2, 4}
    };
    
    allResults.push_back(runTest("FCFS", "FCFS - Basic", 2, createProcessSet(basicSet), false));
    allResults.push_back(runTest("SJF", "SJF - Basic", 2, createProcessSet(basicSet), false));
    allResults.push_back(runTest("SRTF", "SRTF - Basic", 2, createProcessSet(basicSet), false));
    allResults.push_back(runTest("RR", "RR (Q=2) - Basic", 2, createProcessSet(basicSet), false));
    allResults.push_back(runTest("Priority", "Priority Preemptive - Basic", 2, createProcessSet(basicSet), false));
    allResults.push_back(runTest("PriorityNP", "Priority Non-Preemptive - Basic", 2, createProcessSet(basicSet), false));
    
    // ========================================
    // TEST SET 2: Simultaneous Arrivals
    // ========================================
    std::cout << "\n\n═══ TEST SET 2: Simultaneous Arrivals ═══\n";
    
    std::vector<PData> simArrival = {
        {1, "P1", 0, 8, 3},
        {2, "P2", 0, 4, 2},
        {3, "P3", 0, 2, 1},
        {4, "P4", 0, 1, 4}
    };
    
    allResults.push_back(runTest("SJF", "SJF - Same Arrival", 2, createProcessSet(simArrival), false));
    allResults.push_back(runTest("Priority", "Priority - Same Arrival", 2, createProcessSet(simArrival), false));
    
    // ========================================
    // TEST SET 3: Preemption Stress Test
    // ========================================
    std::cout << "\n\n═══ TEST SET 3: Preemption Scenarios ═══\n";
    
    std::vector<PData> preemptSet = {
        {1, "Long", 0, 20, 5},
        {2, "High", 2, 5, 1},
        {3, "Med", 4, 5, 2},
        {4, "Low", 6, 5, 3}
    };
    
    allResults.push_back(runTest("SRTF", "SRTF - Multiple Preemptions", 2, createProcessSet(preemptSet), false));
    allResults.push_back(runTest("Priority", "Priority - Cascading Preemption", 2, createProcessSet(preemptSet), false));
    
    // ========================================
    // TEST SET 4: Round Robin Variations
    // ========================================
    std::cout << "\n\n═══ TEST SET 4: Round Robin Quantum Tests ═══\n";
    
    std::vector<PData> rrSet = {
        {1, "P1", 0, 10, 0},
        {2, "P2", 1, 5, 0},
        {3, "P3", 2, 8, 0}
    };
    
    allResults.push_back(runTest("RR", "RR - Quantum 1", 1, createProcessSet(rrSet), false));
    allResults.push_back(runTest("RR", "RR - Quantum 3", 3, createProcessSet(rrSet), false));
    allResults.push_back(runTest("RR", "RR - Quantum 10", 10, createProcessSet(rrSet), false));
    
    // ========================================
    // TEST SET 5: Aging Mechanism
    // ========================================
    std::cout << "\n\n═══ TEST SET 5: Aging Prevention ═══\n";
    
    std::vector<PData> agingSet = {
        {1, "Starve", 0, 15, 10},  // Low priority, long burst
        {2, "HighP1", 1, 3, 1},
        {3, "HighP2", 4, 3, 1},
        {4, "HighP3", 7, 3, 1},
        {5, "HighP4", 10, 3, 1}
    };
    
    allResults.push_back(runTest("Priority", "Priority - NO Aging (Starvation Risk)", 2, createProcessSet(agingSet), false));
    allResults.push_back(runTest("Priority", "Priority - WITH Aging", 2, createProcessSet(agingSet), true));
    
    // ========================================
    // TEST SET 6: Edge Cases
    // ========================================
    std::cout << "\n\n═══ TEST SET 6: Edge Cases ═══\n";
    
    // Single process
    std::vector<PData> singleProc = {{1, "Only", 0, 5, 1}};
    allResults.push_back(runTest("FCFS", "Single Process", 2, createProcessSet(singleProc), false));
    
    // Very short bursts
    std::vector<PData> shortBurst = {
        {1, "P1", 0, 1, 1},
        {2, "P2", 0, 1, 1},
        {3, "P3", 0, 1, 1}
    };
    allResults.push_back(runTest("RR", "Very Short Bursts (Q=2)", 2, createProcessSet(shortBurst), false));
    
    // Delayed arrivals
    std::vector<PData> delayedArr = {
        {1, "Early", 0, 3, 1},
        {2, "Late", 10, 5, 1}
    };
    allResults.push_back(runTest("FCFS", "Delayed Arrival with Idle", 2, createProcessSet(delayedArr), false));
    
    // ========================================
    // TEST SET 7: Comprehensive Stress Test
    // ========================================
    std::cout << "\n\n═══ TEST SET 7: Comprehensive Stress Test ═══\n";
    
    std::vector<PData> stressTest = {
        {1, "P1", 0, 10, 5},
        {2, "P2", 1, 1, 1},
        {3, "P3", 2, 15, 8},
        {4, "P4", 3, 3, 2},
        {5, "P5", 4, 8, 4},
        {6, "P6", 5, 2, 3},
        {7, "P7", 6, 12, 7},
        {8, "P8", 7, 5, 1},
        {9, "P9", 8, 6, 6},
        {10, "P10", 9, 4, 2}
    };
    
    allResults.push_back(runTest("FCFS", "10 Process - FCFS", 2, createProcessSet(stressTest), false));
    allResults.push_back(runTest("SJF", "10 Process - SJF", 2, createProcessSet(stressTest), false));
    allResults.push_back(runTest("SRTF", "10 Process - SRTF", 2, createProcessSet(stressTest), false));
    allResults.push_back(runTest("RR", "10 Process - RR (Q=3)", 3, createProcessSet(stressTest), false));
    allResults.push_back(runTest("Priority", "10 Process - Priority + Aging", 2, createProcessSet(stressTest), true));
    
    // ========================================
    // FINAL SUMMARY
    // ========================================
    std::cout << "\n\n╔════════════════════════════════════════════════╗\n";
    std::cout << "║              TEST SUMMARY                      ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n\n";
    
    int passed = 0;
    int failed = 0;
    
    std::cout << std::left << std::setw(50) << "Test Name" 
              << std::setw(10) << "Status" 
              << std::setw(12) << "Avg Wait"
              << std::setw(12) << "Avg TAT" << "\n";
    std::cout << std::string(84, '=') << "\n";
    
    for (const auto& result : allResults) {
        std::string status = result.passed ? "✓ PASS" : "✗ FAIL";
        std::cout << std::left << std::setw(50) << result.testName 
                  << std::setw(10) << status
                  << std::setw(12) << std::fixed << std::setprecision(2) << result.avgWaiting
                  << std::setw(12) << result.avgTurnaround << "\n";
        
        if (result.passed) passed++;
        else failed++;
    }
    
    std::cout << std::string(84, '=') << "\n";
    std::cout << "\nTotal Tests: " << allResults.size() 
              << " | Passed: " << passed 
              << " | Failed: " << failed << "\n";
    
    if (failed == 0) {
        std::cout << "\n All tests passed successfully!\n";
    } else {
        std::cout << "\n Some tests failed. Review the logs above.\n";
    }
    
    return (failed == 0) ? 0 : 1;
}

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <string>
#include <vector>

#include "json.hpp"

/**
 * Process Control Block (PCB) structure
 * Stores all process-related information for scheduling
 */
struct Process {
    int id;
    std::string name;
    int arrivalTime;
    int burstTime;
    int priority;  // Lower value = Higher priority
    
    // Runtime tracking
    int remainingTime;
    int startTime = -1;         // -1 indicates process hasn't started
    int completionTime = -1;
    int waitingTime = 0;
    int turnaroundTime = 0;
    int responseTime = -1;      // Time from arrival to first execution
    
    // Aging support
    int ageCounter = 0;
    int originalPriority;       // Track original priority for aging
};

/**
 * CPU Scheduler Implementation
 * Supports: FCFS, SJF, SRTF, RR, Priority (Preemptive & Non-Preemptive)
 * Optional: Aging mechanism to prevent starvation
 */
class Scheduler {
public:
    Scheduler();

    // Configuration methods
    void addProcess(int id, std::string name, int arrivalTime, int burstTime, int priority);
    void setAlgorithm(std::string algo); 
    void setTimeQuantum(int q);
    void setAging(bool enabled);
    void setAgingThreshold(int threshold);  // Configure aging interval
    
    // Simulation control
    std::string tick();  // Execute one time unit
    bool isFinished() const;
    
    // State inspection
    nlohmann::json getStateJSON() const;

private:
    // Configuration
    std::string algorithm = "FCFS";
    bool agingEnabled = false;
    int timeQuantum = 2;
    int agingThreshold = 5;  // Increase priority after this many ticks
    int currentTime = 0;
    
    // Process queues
    std::vector<Process> jobPool;           // Processes not yet arrived
    std::vector<Process> readyQueue;        // Processes ready to execute
    std::vector<Process> finishedProcesses; // Completed processes
    
    // CPU state (vector of size 0 or 1 for safe access)
    std::vector<Process> cpu; 
    int currentQuantumUsed = 0;
    
    // Track what executed this tick (for accurate Gantt display)
    std::string lastExecutedName = "";
    int lastExecutedId = -1; 
    
    // Helper methods
    void checkArrivals();              // Move arrived processes to ready queue
    void preemptCPU();                 // Move CPU process back to ready queue
    void scheduleNextProcess();        // Select next process based on algorithm
    void executeProcess();             // Execute current CPU process for one tick
    void applyAging();                 // Apply aging to ready queue processes
    void updateWaitingTimes();         // Update waiting times for ready processes
    
    // Algorithm-specific helpers
    void sortBySJF();                  // Sort by burst time
    void sortBySRTF();                 // Sort by remaining time
    void sortByPriority();             // Sort by priority value
    bool shouldPreemptSRTF();          // Check SRTF preemption condition
    bool shouldPreemptPriority();      // Check Priority preemption condition
};

#endif

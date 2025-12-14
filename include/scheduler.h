#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <string>
#include <vector>
#include <iostream>
#include <algorithm> 

#include "json.hpp"

struct Process {
    int id;
    std::string name;
    int arrivalTime;
    int burstTime;
    int priority; 
    
    int remainingTime;
    int startTime = -1;
    int completionTime = -1;
    int waitingTime = 0;
    int turnaroundTime = 0;
            
    // Aging
    int ageCounter = 0;
};

class Scheduler {
public:
    Scheduler();

    void addProcess(int id, std::string name, int arrivalTime, int burstTime, int priority);
    void setAlgorithm(std::string algo); 
    void setTimeQuantum(int q);
    void setAging(bool enabled);
    
    std::string tick();

    bool isFinished() const;
    nlohmann::json getStateJSON() const;

private:
    std::string algorithm = "FCFS";
    bool agingEnabled = false;
    int timeQuantum = 2;
    int currentTime = 0;
    
    std::vector<Process> jobPool; 
    std::vector<Process> readyQueue; 
    std::vector<Process> finishedProcesses;
    
    // Active Process Storage
    // Use a vector of size 0 or 1 to hold the active process safely
    std::vector<Process> cpu; 
    int currentQuantumUsed = 0; 
    
    void checkArrivals();
    // Helper to move from cpu back to ready queue
    void preemptCPU();
};

#endif

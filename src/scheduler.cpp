#include "scheduler.h"
#include <sstream>
#include <iostream>
#include <algorithm>

Scheduler::Scheduler() {
    currentTime = 0;
    currentQuantumUsed = 0;
}

void Scheduler::addProcess(int id, std::string name, int arrivalTime, int burstTime, int priority) {
    Process p;
    p.id = id;
    p.name = name;
    p.arrivalTime = arrivalTime;
    p.burstTime = burstTime;
    p.priority = priority;
    p.remainingTime = burstTime;
    p.startTime = -1;
    // Pushing to jobPool, will be moved to readyQueue when arrivalTime <= currentTime
    jobPool.push_back(p);
}

void Scheduler::setAlgorithm(std::string algo) {
    algorithm = algo;
}

void Scheduler::setTimeQuantum(int q) {
    timeQuantum = q;
}

void Scheduler::setAging(bool enabled) {
    agingEnabled = enabled;
}

bool Scheduler::isFinished() const {
    return jobPool.empty() && readyQueue.empty() && cpu.empty();
}

void Scheduler::checkArrivals() {
    // Move all processes with arrivalTime <= currentTime from jobPool to readyQueue.
    // Order of arrival at same time: Usually defined by input order (FIFO).
    
    // Using a stable partition or just iterating safely.
    // We iterate and erase.
    auto it = jobPool.begin();
    while (it != jobPool.end()) {
        if (it->arrivalTime <= currentTime) {
            readyQueue.push_back(*it);
            it = jobPool.erase(it);
        } else {
            ++it;
        }
    }
}

void Scheduler::preemptCPU() {
    if (!cpu.empty()) {
        Process p = cpu.front();
        cpu.clear();
        readyQueue.push_back(p);
    }
}

std::string Scheduler::tick() {
    std::stringstream log;
    log << "Time " << currentTime << ": ";

    // 1. Handling Round Robin Quantum Expiry
    // Constraint: "process goes to the back... before a process that just arrived"
    // So if RR and Quantum Expired, we must preempt NOW, before checking new arrivals.
    
    bool justPreemptedRR = false;
    
    if (algorithm == "RR" && !cpu.empty()) {
        currentQuantumUsed++;
        
        // Check for Quantum Expiry
        if (currentQuantumUsed >= timeQuantum) {
             if (cpu[0].remainingTime > 0) {
                 log << "Process " << cpu[0].id << " quantum expired. ";
                 preemptCPU();
                 justPreemptedRR = true;
                 currentQuantumUsed = 0;
             }
        }
    }
    
    // 2. Check Arrivals (happens after RR expiry check to respect order constraints)
    checkArrivals();
    
    // Constraint check: Preempted RR process must be BEFORE new arrivals in scheduling order
    // "Process goes to the back before a "new" process that has just arrived."
    // Implementation:
    // i. Quantum Expired -> preemptCPU() -> Pushes to back of ReadyQueue.
    // ii. checkArrivals() -> Pushes new arrivals to back of ReadyQueue.
    // Result in Queue: [..., Preempted, Arrived]
    // Since we pick from Front, Preempted is ahead of Arrived. Correct.
    
    // Handling Preemption for SRTF
    if (algorithm == "SRTF" && !cpu.empty() && !readyQueue.empty()) {
        // Check if any process in readyQueue has shorter remaining time
        auto shortestInQueue = std::min_element(readyQueue.begin(), readyQueue.end(), 
            [](const Process& a, const Process& b){
                if (a.remainingTime != b.remainingTime) return a.remainingTime < b.remainingTime;
                return a.id < b.id;
            });
            
        if (shortestInQueue->remainingTime < cpu[0].remainingTime) {
            log << "Process " << cpu[0].id << " preempted by " << shortestInQueue->id << ". ";
            preemptCPU(); 
        }
    }
    
    // Handling Priority Preemption
    if (algorithm == "Priority" && !cpu.empty() && !readyQueue.empty()) {
         auto highestPriorityInQueue = std::min_element(readyQueue.begin(), readyQueue.end(), 
            [](const Process& a, const Process& b){
                if (a.priority != b.priority) return a.priority < b.priority; // Lower val = Higher Priority
                return a.id < b.id;
            });
            
        // Check if queue process has strictly higher priority (lower value) than current CPU process
        if (highestPriorityInQueue->priority < cpu[0].priority) {
            log << "Process " << cpu[0].id << " preempted by " << highestPriorityInQueue->id << " (Priority " << highestPriorityInQueue->priority << " < " << cpu[0].priority << "). ";
            preemptCPU(); 
        }
    }
    
    // 3. Scheduling Decision
    if (cpu.empty() && !readyQueue.empty()) {
        if (algorithm == "SJF") {
            // Sort by Burst Time
            std::sort(readyQueue.begin(), readyQueue.end(), [](const Process& a, const Process& b){
                 if (a.burstTime != b.burstTime) return a.burstTime < b.burstTime;
                 return a.id < b.id;
            });
        } else if (algorithm == "SRTF") {
            // Sort by Remaining Time
            std::sort(readyQueue.begin(), readyQueue.end(), [](const Process& a, const Process& b){
                if (a.remainingTime != b.remainingTime) return a.remainingTime < b.remainingTime;
                return a.id < b.id;
            });
        } else if (algorithm == "Priority" || algorithm == "PriorityNP") {
            // Sort by Priority (Lower value = Higher Priority)
            std::sort(readyQueue.begin(), readyQueue.end(), [](const Process& a, const Process& b){
                if (a.priority != b.priority) return a.priority < b.priority;
                return a.id < b.id;
            });
        }
        // FCFS and RR use default arrival order (no sort needed)
        
        // Move process from Ready Queue to CPU
        cpu.push_back(readyQueue.front());
        readyQueue.erase(readyQueue.begin());
        currentQuantumUsed = 0;
        
        if (cpu[0].startTime == -1) {
            cpu[0].startTime = currentTime;
        }
        log << "Process " << cpu[0].id << " starts/resumes. ";
    }
    
    // 4. Execution
    if (!cpu.empty()) {
        cpu[0].remainingTime--;
        
        // Stats update? Waiting time for others?
        // Standard: Increment waiting time for everyone in ready queue?
        // Yes, typically waiting time = (Time - Arrival - Burst) or accumulated.
        // Let's accumulate waitingTime for everyone in readyQueue.
        for (auto &p : readyQueue) {
            p.waitingTime++;
        }
        
        log << "Running Process " << cpu[0].id << " (" << cpu[0].remainingTime << " left).";
        
        // Check for finish
        if (cpu[0].remainingTime <= 0) {
            cpu[0].completionTime = currentTime + 1; // It finishes at the END of this tick unit
            cpu[0].turnaroundTime = cpu[0].completionTime - cpu[0].arrivalTime;
            // waitingTime is already accumulating in ready queue.
            // If it was preempted, waitingTime logic holds.
            
            finishedProcesses.push_back(cpu[0]);
            log << " Process " << cpu[0].id << " finished.";
            cpu.clear();
            currentQuantumUsed = 0;
        }
    } else {
        log << "Idle.";
    }
    
    currentTime++;
    
    // 5. Aging Logic (End of Tick)
    // Apply to ALL algorithms if enabled (User request)
    // "implement ageing to all the algorithms" implies we shouldn't restrict by name, 
    // though valid only if priority matters.
    if (agingEnabled && !readyQueue.empty()) {
        for (auto &p : readyQueue) {
            p.ageCounter++;
            if (p.ageCounter >= 5) {
                if (p.priority > 0) {
                    p.priority--;
                    log << " [Aging: Process " << p.id << " priority -> " << p.priority << "]";
                }
                p.ageCounter = 0;
            }
        }
    }

    return log.str();
}

nlohmann::json Scheduler::getStateJSON() const {
    nlohmann::json j;
    j["time"] = currentTime;
    
    if (!cpu.empty()) {
        j["cpu_process"] = {
            {"id", cpu[0].id},
            {"remaining", cpu[0].remainingTime}
        };
    } else {
        j["cpu_process"] = nullptr;
    }
    
    j["ready_queue"] = nlohmann::json::array();
    for (const auto& p : readyQueue) {
        j["ready_queue"].push_back({
            {"id", p.id},
            {"remaining", p.remainingTime},
            {"priority", p.priority},
            {"age", p.ageCounter}
        });
    }
    
    j["finished"] = nlohmann::json::array();
    for (const auto& p : finishedProcesses) {
        j["finished"].push_back({
            {"id", p.id},
            {"waiting_time", p.waitingTime},
            {"turnaround_time", p.turnaroundTime}
        });
    }
    
    return j;
}

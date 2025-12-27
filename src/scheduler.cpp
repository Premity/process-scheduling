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
    p.originalPriority = priority;  // Store original for reference
    p.remainingTime = burstTime;
    p.startTime = -1;
    p.responseTime = -1;
    
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

void Scheduler::setAgingThreshold(int threshold) {
    agingThreshold = threshold;
}

bool Scheduler::isFinished() const {
    return jobPool.empty() && readyQueue.empty() && cpu.empty();
}

/**
 * Check for process arrivals and move them to ready queue
 * Processes are added in arrival order (FIFO within same arrival time)
 */
void Scheduler::checkArrivals() {
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

/**
 * Preempt the currently running process
 * Moves CPU process back to ready queue
 */
void Scheduler::preemptCPU() {
    if (!cpu.empty()) {
        Process p = cpu.front();
        cpu.clear();
        readyQueue.push_back(p);
        currentQuantumUsed = 0;
    }
}

// Sorting helpers
void Scheduler::sortBySJF() {
    std::sort(readyQueue.begin(), readyQueue.end(), [](const Process& a, const Process& b){
        if (a.burstTime != b.burstTime) return a.burstTime < b.burstTime;
        if (a.arrivalTime != b.arrivalTime) return a.arrivalTime < b.arrivalTime;
        return a.id < b.id;
    });
}

void Scheduler::sortBySRTF() {
    std::sort(readyQueue.begin(), readyQueue.end(), [](const Process& a, const Process& b){
        if (a.remainingTime != b.remainingTime) return a.remainingTime < b.remainingTime;
        if (a.arrivalTime != b.arrivalTime) return a.arrivalTime < b.arrivalTime;
        return a.id < b.id;
    });
}

void Scheduler::sortByPriority() {
    std::sort(readyQueue.begin(), readyQueue.end(), [](const Process& a, const Process& b){
        if (a.priority != b.priority) return a.priority < b.priority;
        if (a.arrivalTime != b.arrivalTime) return a.arrivalTime < b.arrivalTime;
        return a.id < b.id;
    });
}

/**
 * Check if SRTF preemption should occur
 * Returns true if a ready process has shorter remaining time than current CPU process
 */
bool Scheduler::shouldPreemptSRTF() {
    if (cpu.empty() || readyQueue.empty()) return false;
    
    auto shortestInQueue = std::min_element(readyQueue.begin(), readyQueue.end(), 
        [](const Process& a, const Process& b){
            if (a.remainingTime != b.remainingTime) return a.remainingTime < b.remainingTime;
            return a.id < b.id;
        });
        
    return shortestInQueue->remainingTime < cpu[0].remainingTime;
}

/**
 * Check if Priority preemption should occur
 * Returns true if a ready process has higher priority (lower value) than current CPU process
 */
bool Scheduler::shouldPreemptPriority() {
    if (cpu.empty() || readyQueue.empty()) return false;
    
    auto highestPriorityInQueue = std::min_element(readyQueue.begin(), readyQueue.end(), 
        [](const Process& a, const Process& b){
            if (a.priority != b.priority) return a.priority < b.priority;
            return a.id < b.id;
        });
        
    return highestPriorityInQueue->priority < cpu[0].priority;
}

/**
 * Select and dispatch the next process based on the scheduling algorithm
 */
void Scheduler::scheduleNextProcess() {
    if (cpu.empty() && !readyQueue.empty()) {
        // Apply algorithm-specific sorting
        if (algorithm == "SJF") {
            sortBySJF();
        } else if (algorithm == "SRTF") {
            sortBySRTF();
        } else if (algorithm == "Priority" || algorithm == "PriorityNP") {
            sortByPriority();
        }
        // FCFS and RR use arrival order (no sorting needed)
        
        // Dispatch process to CPU
        cpu.push_back(readyQueue.front());
        readyQueue.erase(readyQueue.begin());
        currentQuantumUsed = 0;
        
        // Record first execution time (for response time calculation)
        if (cpu[0].startTime == -1) {
            cpu[0].startTime = currentTime;
            cpu[0].responseTime = currentTime - cpu[0].arrivalTime;
        }
    }
}

/**
 * Execute the current CPU process for one time unit
 * Updates statistics and handles process completion
 */
void Scheduler::executeProcess() {
    if (!cpu.empty()) {
        cpu[0].remainingTime--;
        currentQuantumUsed++;
        
        // Check for completion
        if (cpu[0].remainingTime <= 0) {
            cpu[0].completionTime = currentTime + 1;
            cpu[0].turnaroundTime = cpu[0].completionTime - cpu[0].arrivalTime;
            cpu[0].waitingTime = cpu[0].turnaroundTime - cpu[0].burstTime;
            // overwrite waiting time with calculated value for redundancy
            
            finishedProcesses.push_back(cpu[0]);
            cpu.clear();
            currentQuantumUsed = 0;
        }
    }
}

/**
 * Update waiting times for all processes in ready queue
 * Called once per tick for accurate statistics
 */
void Scheduler::updateWaitingTimes() {
    for (auto& p : readyQueue) {
        p.waitingTime++;
    }
}

/**
 * Apply aging mechanism to prevent starvation
 * Increases priority (decreases value) for processes waiting too long
 */
void Scheduler::applyAging() {
    if (!agingEnabled || readyQueue.empty()) return;
    
    for (auto &p : readyQueue) {
        p.ageCounter++;
        
        // Apply priority boost at aging threshold
        if (p.ageCounter >= agingThreshold) {
            if (p.priority > 0) {
                p.priority--;
            }
            p.ageCounter = 0;  // Reset counter after boost
        }
    }
}

/**
 * Main simulation tick - executes one time unit
 * Order of operations is critical for correct algorithm behavior
 */
std::string Scheduler::tick() {
    std::stringstream log;
    log << "Time " << currentTime << ": ";

    // === PHASE 1: Check for new arrivals (BEFORE preemption checks) ===
    // New arrivals join the ready queue
    checkArrivals();

    // === PHASE 2: Handle preemption based on algorithm ===
    
    // Round Robin: Check quantum expiration
    if (algorithm == "RR" && !cpu.empty() && cpu[0].remainingTime > 0) {
        if (currentQuantumUsed >= timeQuantum) {
            log << "Process " << cpu[0].id << " quantum expired. ";
            preemptCPU();
        }
    }
    
    // SRTF: Check for shorter process
    if (algorithm == "SRTF" && shouldPreemptSRTF()) {
        auto shortestInQueue = std::min_element(readyQueue.begin(), readyQueue.end(), 
            [](const Process& a, const Process& b){
                return a.remainingTime < b.remainingTime;
            });
        log << "Process " << cpu[0].id << " preempted by Process " 
            << shortestInQueue->id << " (SRTF). ";
        preemptCPU();
    }
    
    // Priority (Preemptive): Check for higher priority process
    if (algorithm == "Priority" && shouldPreemptPriority()) {
        auto highestInQueue = std::min_element(readyQueue.begin(), readyQueue.end(), 
            [](const Process& a, const Process& b){
                return a.priority < b.priority;
            });
        log << "Process " << cpu[0].id << " preempted by Process " 
            << highestInQueue->id << " (Priority " << highestInQueue->priority 
            << " < " << cpu[0].priority << "). ";
        preemptCPU();
    }
    
    // === PHASE 3: Schedule next process if CPU is idle ===
    scheduleNextProcess();
    
    // === PHASE 4: Execute current process ===
    if (!cpu.empty()) {
        // Track what's running BEFORE execution (for accurate Gantt display)
        lastExecutedName = cpu[0].name;
        lastExecutedId = cpu[0].id;
        
        int remainingBefore = cpu[0].remainingTime;
        log << "Running Process " << cpu[0].id << " (" << remainingBefore << " remaining). ";
        
        executeProcess();
        updateWaitingTimes(); // Update stats for waiting processes
        
        // Check if process just finished
        if (cpu.empty()) {
            log << "Process " << finishedProcesses.back().id << " finished.";
        }
    } else {
        lastExecutedName = "";
        lastExecutedId = -1;
        log << "CPU Idle.";
    }
    
    // === PHASE 5: Apply aging (end of tick) ===
    applyAging();
    if (agingEnabled && !readyQueue.empty()) {
        for (const auto &p : readyQueue) {
            if (p.ageCounter == 0 && p.priority < p.originalPriority) {
                log << " [Aged: P" << p.id << " priority=" << p.priority << "]";
            }
        }
    }
    
    currentTime++;
    return log.str();
}

nlohmann::json Scheduler::getStateJSON() const {
    nlohmann::json j;
    j["time"] = currentTime;
    j["algorithm"] = algorithm;
    
    if (!cpu.empty()) {
        j["cpu_process"] = {
            {"id", cpu[0].id},
            {"name", cpu[0].name},
            {"remaining", cpu[0].remainingTime},
            {"quantum_used", currentQuantumUsed}
        };
    } else {
        j["cpu_process"] = nullptr;
    }
    
    // Include what executed THIS tick (for Gantt chart accuracy)
    if (lastExecutedId != -1) {
        j["last_executed"] = {
            {"id", lastExecutedId},
            {"name", lastExecutedName}
        };
    } else {
        j["last_executed"] = nullptr;
    }
    
    j["ready_queue"] = nlohmann::json::array();
    for (const auto& p : readyQueue) {
        j["ready_queue"].push_back({
            {"id", p.id},
            {"name", p.name},
            {"remaining", p.remainingTime},
            {"priority", p.priority},
            {"age_counter", p.ageCounter}
        });
    }
    
    j["job_pool"] = nlohmann::json::array();
    for (const auto& p : jobPool) {
        j["job_pool"].push_back({
            {"id", p.id},
            {"arrival", p.arrivalTime}
        });
    }
    
    j["finished"] = nlohmann::json::array();
    for (const auto& p : finishedProcesses) {
        j["finished"].push_back({
            {"id", p.id},
            {"name", p.name},
            {"waiting_time", p.waitingTime},
            {"turnaround_time", p.turnaroundTime},
            {"response_time", p.responseTime}
        });
    }
    
    return j;
}

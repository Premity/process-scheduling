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
    // Sort logic for jobPool isn't strict, but we check every tick. 
    // Optimization: Sort by arrival time? Or just iterate. Iteration is fine for small count.
}

void Scheduler::setAlgorithm(std::string algo) {
    algorithm = algo;
}

void Scheduler::setTimeQuantum(int q) {
    timeQuantum = q;
}

bool Scheduler::isFinished() const {
    return jobPool.empty() && readyQueue.empty() && cpu.empty();
}

void Scheduler::checkArrivals() {
    // Standard approach: Move all processes with arrivalTime <= currentTime from jobPool to readyQueue.
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

void Scheduler::preemptCPU(bool toBackOfQueue) {
    if (!cpu.empty()) {
        Process p = cpu.front();
        cpu.clear();
        if (toBackOfQueue) {
            readyQueue.push_back(p);
        } else {
            // Put at front (for some theoretical preemption cases, or generic stack logic)
            // But usually preemption means back of queue for RR.
            // For SRTF, it goes back into pool, and we resort.
            readyQueue.push_back(p);
        }
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
        if (currentQuantumUsed >= timeQuantum) {
            // Quantum expired.
            // Check if process is finished?
            // Actually, we check finish status typically after decrementing logic. 
            // BUT wait, a tick represents "doing work". 
            // So logic flow:
            // Start of Tick -> Manage State (New Arrivals? Preemptions?) -> Do 1 unit of work -> End of Tick?
            // OR: Do 1 unit of work (if CPU busy) -> Increment Time -> Check Events?
            
            // Standard Simulation Loop usually:
            // 1. Check Arrivals (at CurrentTime)
            // 2. Schedule/Select Process
            // 3. Execute (update stats)
            // 4. Increment Time
            
            // But prompt says: "advances time by 1 unit".
            // So calling tick() means we ARE at `currentTime`, and we want to move to `currentTime + 1`.
            
            // Let's stick to the prompt's specific RR constraint: 
            // "Quantum expires ... process goes to back ... before a process that just arrived"
            // This implies the expiry happens at the interrupt level BEFORE the arrival interrupt handling?
            // Or rather, the ordering in the queue is [Preempted, Arrived].
            
            // So, sequence:
            // 1. Handle RR Expiry (if needed).
            // 2. Handle Arrivals.
            // 3. Select CPU.
            
            // Wait, if I expire logic creates a "Preempted" process, I push it to ReadyQueue.
            // Then I check arrivals, and push "Arrived" to ReadyQueue.
            // Since `push_back` appends, `Preempted` will be before `Arrived`. This matches the requirement.
            
            // BUT, strictly speaking, quantum check usually happens AFTER execution?
            // Let's assume we are checking the state at the BEGINNING of the tick before work is done?
            // No, `tick()` advances time. 
            // Let's assume `tick()` covers the time interval [t, t+1).
            
            // Re-evaluating "Quantum Expired".
            // If we just finished a quantum (runs 0..Q-1), we need to preempt.
            // BUT, did it finish the quantum in the PREVIOUS tick?
            // Current State: CPU has process, `currentQuantumUsed`.
            // If `currentQuantumUsed == timeQuantum`, it means it ran for Q units already.
            // So we should have preempted it at the end of previous tick? Or start of this one.
            // Let's do it at start of this one.
            
            // Logically:
            // Loop:
            //   tick():
            //     Manage Queues (Preempt, Arrive)
            //     Run Process (Work--)
            //     Time++
            
            if (currentQuantumUsed >= timeQuantum) {
                 // Check if it finished? If remaining <= 0, it would have been removed already?
                 // Let's ensure finish logic handles itself.
                 if (cpu[0].remainingTime > 0) {
                     log << "Process " << cpu[0].id << " quantum expired. ";
                     preemptCPU(true); // To Back
                     justPreemptedRR = true;
                     currentQuantumUsed = 0;
                 }
            }
        }
    }
    
    // 2. Check Arrivals
    // Arrivals happen at `currentTime`.
    checkArrivals(); // Pushes to back of readyQueue.
    
    // If we just preempted RR, and a new process arrived, the order in ReadyQueue is:
    // [Old Ready Queue Processes..., Preempted Process, New Arrivals...]
    // Validating constraint: "process goes to the back... before a process that just arrived".
    // My code:
    //   preemptCPU() -> push_back(Preempted)
    //   checkArrivals() -> push_back(Arrivals)
    // Result: ... -> Preempted -> Arrivals.
    // Yes, Preempted is "before" (ahead of? No, in standard Queue, "Back" is end).
    // "Goes to the back ... before a process that just arrived."
    // User logic: "Insert X at back. Then insert Y at back." 
    // Queue: [..., X, Y]. 
    // X is ahead of Y? No, X is effectively earlier in buffer, but both are at back. 
    // Scheduling order (FIFO) picks front. So X is picked BEFORE Y. 
    // Yes. Correct.
    
    // 3. Scheduling Decision (Select Process if CPU idle)
    // If CPU is idle, pick from ReadyQueue
    
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
            preemptCPU(true); // Put back to ready queue
            // CPU is now empty, will pick up shortest below
        }
    }
    
    // Handling Priority Preemption? (Usually standard Priority is non-preemptive unless specified "Preemptive Priority")
    // Prompt says "Priority". Usually implies non-preemptive unless specified.
    // I will stick to Non-Preemptive for Priority unless CPU is empty.
    
    if (cpu.empty() && !readyQueue.empty()) {
        // Sort/Pick based on Algorithm
        if (algorithm == "FCFS" || algorithm == "RR") {
            // Pick value at front (already sorted by arrival / queue logic)
            // No sorting needed.
        } else if (algorithm == "SJF") {
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
        } else if (algorithm == "Priority") {
            // Sort by Priority
            std::sort(readyQueue.begin(), readyQueue.end(), [](const Process& a, const Process& b){
                if (a.priority != b.priority) return a.priority < b.priority;
                return a.id < b.id;
            });
        }
        
        // Move front to CPU
        cpu.push_back(readyQueue.front());
        readyQueue.erase(readyQueue.begin());
        currentQuantumUsed = 0;
        
        // Start Time check
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
            {"priority", p.priority}
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

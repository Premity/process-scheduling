/**
 * CPU Scheduler Simulator - Frontend Logic
 * Vanilla JavaScript implementation
 */

// === State Management ===
let scheduler = null;
let processes = [];
let processIdCounter = 1;
let isPlaying = false;
let playInterval = null;
let ganttData = [];

// === DOM Elements ===
const elements = {
    processName: document.getElementById('processName'),
    arrivalTime: document.getElementById('arrivalTime'),
    burstTime: document.getElementById('burstTime'),
    priority: document.getElementById('priority'),
    addProcessBtn: document.getElementById('addProcessBtn'),
    loadCsvBtn: document.getElementById('loadCsvBtn'),
    csvFileInput: document.getElementById('csvFileInput'),
    algorithmSelect: document.getElementById('algorithmSelect'),
    timeQuantum: document.getElementById('timeQuantum'),
    quantumContainer: document.getElementById('quantumContainer'),
    enableAging: document.getElementById('enableAging'),
    processList: document.getElementById('processList'),
    processCount: document.getElementById('processCount'),
    currentTime: document.getElementById('currentTime'),
    cpuBox: document.getElementById('cpuBox'),
    cpuStatus: document.getElementById('cpuStatus'),
    readyQueue: document.getElementById('readyQueue'),
    ganttChart: document.getElementById('ganttChart'),
    stepBtn: document.getElementById('stepBtn'),
    playBtn: document.getElementById('playBtn'),
    pauseBtn: document.getElementById('pauseBtn'),
    resetBtn: document.getElementById('resetBtn'),
    speedSlider: document.getElementById('speedSlider'),
    resultsContainer: document.getElementById('resultsContainer'),
    avgWaitingTime: document.getElementById('avgWaitingTime'),
    avgTurnaroundTime: document.getElementById('avgTurnaroundTime'),
    avgResponseTime: document.getElementById('avgResponseTime'),
    resultsTable: document.getElementById('resultsTable'),
    executionLog: document.getElementById('executionLog')
};

// Expose initialization to be called by Module
window.initializeApp = function() {
    console.log("Initializing App...");
    setupEventListeners();
    updateQuantumVisibility();
    updateProcessList();
}

// === Event Listeners ===
function setupEventListeners() {
    elements.addProcessBtn.addEventListener('click', addProcess);
    elements.loadCsvBtn.addEventListener('click', () => elements.csvFileInput.click());
    elements.csvFileInput.addEventListener('change', handleCsvUpload);
    elements.algorithmSelect.addEventListener('change', updateQuantumVisibility);
    elements.stepBtn.addEventListener('click', stepSimulation);
    elements.playBtn.addEventListener('click', startPlayback);
    elements.pauseBtn.addEventListener('click', pausePlayback);
    elements.resetBtn.addEventListener('click', resetSimulation);
    
    // Keyboard shortcuts
    document.addEventListener('keydown', (e) => {
        if (e.key === 'Enter' && document.activeElement.tagName === 'INPUT') {
            addProcess();
        }
    });
}

// === Process Management ===
function addProcess() {
    const name = elements.processName.value.trim() || `P${processIdCounter}`;
    const arrival = parseInt(elements.arrivalTime.value) || 0;
    const burst = parseInt(elements.burstTime.value) || 1;
    const priority = parseInt(elements.priority.value) || 0;
    
    if (burst < 1) {
        alert('Burst time must be at least 1');
        return;
    }
    
    processes.push({
        id: processIdCounter++,
        name: name,
        arrival: arrival,
        burst: burst,
        priority: priority
    });
    
    // Reset input fields
    elements.processName.value = '';
    elements.arrivalTime.value = '0';
    elements.burstTime.value = '5';
    elements.priority.value = '0';
    
    updateProcessList();
}

function removeProcess(id) {
    processes = processes.filter(p => p.id !== id);
    updateProcessList();
}

function updateProcessList() {
    elements.processList.innerHTML = '';
    
    if (processes.length === 0) {
        elements.processList.innerHTML = '<span style="color: var(--text-secondary);">No processes added yet</span>';
    } else {
        processes.forEach(p => {
            const chip = document.createElement('div');
            chip.className = 'process-chip';
            chip.innerHTML = `
                <span>${p.name} (A:${p.arrival}, B:${p.burst}, P:${p.priority})</span>
                <button class="remove-btn" onclick="removeProcess(${p.id})">×</button>
            `;
            elements.processList.appendChild(chip);
        });
    }
    
    elements.processCount.textContent = `(${processes.length} processes)`;
}

// === CSV Handling ===
function handleCsvUpload(event) {
    const file = event.target.files[0];
    if (!file) return;
    
    const reader = new FileReader();
    reader.onload = function(e) {
        const lines = e.target.result.split('\n');
        
        lines.forEach((line, index) => {
            if (index === 0 && line.toLowerCase().includes('id')) return; // Skip header
            
            const parts = line.split(',').map(p => p.trim());
            if (parts.length >= 5) {
                processes.push({
                    id: parseInt(parts[0]) || processIdCounter++,
                    name: parts[1] || `P${processIdCounter}`,
                    arrival: parseInt(parts[2]) || 0,
                    burst: parseInt(parts[3]) || 1,
                    priority: parseInt(parts[4]) || 0
                });
            }
        });
        
        updateProcessList();
    };
    reader.readAsText(file);
    event.target.value = ''; // Reset file input
}

// === UI Updates ===
function updateQuantumVisibility() {
    const algo = elements.algorithmSelect.value;
    if (algo === 'RR') {
        elements.quantumContainer.classList.add('visible');
    } else {
        elements.quantumContainer.classList.remove('visible');
    }
}

// === Simulation Control ===
function initializeScheduler() {
    if (typeof Module === 'undefined' || !Module.Scheduler) {
        console.error('WASM Module not available');
        alert('Error: WASM module not loaded. Please refresh the page.');
        return false;
    }
    
    scheduler = new Module.Scheduler();
    
    // Configure scheduler
    const algorithm = elements.algorithmSelect.value;
    const quantum = parseInt(elements.timeQuantum.value) || 2;
    const agingEnabled = elements.enableAging.checked;
    
    scheduler.setAlgorithm(algorithm);
    scheduler.setTimeQuantum(quantum);
    scheduler.setAging(agingEnabled);
    
    // Add all processes
    processes.forEach(p => {
        scheduler.addProcess(p.id, p.name, p.arrival, p.burst, p.priority);
    });
    
    // Reset UI state
    ganttData = [];
    elements.ganttChart.innerHTML = '';
    elements.executionLog.innerHTML = '';
    elements.resultsContainer.classList.add('hidden');
    
    return true;
}

function stepSimulation() {
    if (!scheduler && !initializeScheduler()) {
        return;
    }
    
    if (scheduler.isFinished()) {
        showResults();
        pausePlayback();
        return;
    }
    
    // Execute one tick
    const tickLog = scheduler.tick();
    const state = JSON.parse(scheduler.getStateJSON());
    
    // Update UI
    updateDashboard(state);
    updateReadyQueue(state);
    updateGanttChart(state);
    addLogEntry(tickLog);
    
    // Check if finished
    if (scheduler.isFinished()) {
        showResults();
        pausePlayback();
    }
}

function startPlayback() {
    if (!scheduler && !initializeScheduler()) {
        return;
    }
    
    isPlaying = true;
    elements.playBtn.disabled = true;
    elements.pauseBtn.disabled = false;
    elements.stepBtn.disabled = true;
    
    const speed = 1050 - parseInt(elements.speedSlider.value);
    playInterval = setInterval(stepSimulation, speed);
}

function pausePlayback() {
    isPlaying = false;
    elements.playBtn.disabled = false;
    elements.pauseBtn.disabled = true;
    elements.stepBtn.disabled = false;
    
    if (playInterval) {
        clearInterval(playInterval);
        playInterval = null;
    }
}

function resetSimulation() {
    pausePlayback();
    
    if (scheduler) {
        scheduler.delete(); // Free WASM memory
        scheduler = null;
    }
    
    ganttData = [];
    
    // Reset UI
    elements.currentTime.textContent = '0';
    elements.cpuBox.classList.remove('running');
    elements.cpuStatus.innerHTML = '<span class="idle">IDLE</span>';
    elements.readyQueue.innerHTML = '';
    elements.ganttChart.innerHTML = '';
    elements.executionLog.innerHTML = '';
    elements.resultsContainer.classList.add('hidden');
}

// === UI Update Functions ===
function updateDashboard(state) {
    elements.currentTime.textContent = state.time;
    
    if (state.cpu_process) {
        elements.cpuBox.classList.add('running');
        elements.cpuStatus.innerHTML = `
            <span class="process-info">${state.cpu_process.name}</span>
            <span class="remaining">${state.cpu_process.remaining} remaining</span>
        `;
    } else {
        elements.cpuBox.classList.remove('running');
        elements.cpuStatus.innerHTML = '<span class="idle">IDLE</span>';
    }
}

function updateReadyQueue(state) {
    elements.readyQueue.innerHTML = '';
    
    if (!state.ready_queue || state.ready_queue.length === 0) {
        elements.readyQueue.innerHTML = '<span style="color: var(--text-secondary); padding: 0.5rem;">Empty</span>';
        return;
    }
    
    state.ready_queue.forEach(p => {
        const item = document.createElement('div');
        item.className = 'ready-queue-item';
        
        // Check if priority has been boosted (aged)
        const originalProcess = processes.find(proc => proc.id === p.id);
        if (originalProcess && p.priority < originalProcess.priority) {
            item.classList.add('aged');
        }
        
        item.innerHTML = `
            <div class="name">${p.name}</div>
            <div class="priority">P: ${p.priority}</div>
        `;
        elements.readyQueue.appendChild(item);
    });
}

function updateGanttChart(state) {
    const block = document.createElement('div');
    block.className = 'gantt-block';
    
    // Use last_executed for accurate Gantt (shows what ran even if process just finished)
    if (state.last_executed) {
        block.classList.add('running');
        block.innerHTML = `
            <span>${state.last_executed.name}</span>
            <span class="time-label">t=${state.time - 1}</span>
        `;
    } else {
        block.classList.add('idle');
        block.innerHTML = `
            <span>-</span>
            <span class="time-label">t=${state.time - 1}</span>
        `;
    }
    
    elements.ganttChart.appendChild(block);
    
    // Auto-scroll to latest
    elements.ganttChart.scrollLeft = elements.ganttChart.scrollWidth;
}

function addLogEntry(log) {
    const entry = document.createElement('div');
    entry.className = 'log-entry';
    
    if (log.includes('finished')) {
        entry.classList.add('highlight');
    }
    
    entry.textContent = log;
    elements.executionLog.appendChild(entry);
    elements.executionLog.scrollTop = elements.executionLog.scrollHeight;
}

function showResults() {
    const state = JSON.parse(scheduler.getStateJSON());
    const finished = state.finished;
    
    if (!finished || finished.length === 0) return;
    
    // Calculate averages
    let totalWait = 0, totalTurnaround = 0, totalResponse = 0;
    
    finished.forEach(p => {
        totalWait += p.waiting_time;
        totalTurnaround += p.turnaround_time;
        totalResponse += p.response_time;
    });
    
    const n = finished.length;
    elements.avgWaitingTime.textContent = (totalWait / n).toFixed(2);
    elements.avgTurnaroundTime.textContent = (totalTurnaround / n).toFixed(2);
    elements.avgResponseTime.textContent = (totalResponse / n).toFixed(2);
    
    // Populate table
    const tbody = elements.resultsTable.querySelector('tbody');
    tbody.innerHTML = '';
    
    finished.forEach(p => {
        const row = document.createElement('tr');
        row.innerHTML = `
            <td>${p.id}</td>
            <td>${p.name}</td>
            <td>${p.waiting_time}</td>
            <td>${p.turnaround_time}</td>
            <td>${p.response_time}</td>
        `;
        tbody.appendChild(row);
    });
    
    elements.resultsContainer.classList.remove('hidden');
}

// Make removeProcess globally accessible for onclick handlers
window.removeProcess = removeProcess;

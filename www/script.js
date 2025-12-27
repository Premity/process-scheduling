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
let ganttData = [];         // Track Gantt blocks for merging
let priorityCache = {};     // Cache priority values when column is hidden

const elements = {
    // Algorithm Settings
    algorithmSelect: document.getElementById('algorithmSelect'),
    algorithmSelectSim: document.getElementById('algorithmSelectSim'),
    timeQuantum: document.getElementById('timeQuantum'),
    quantumContainer: document.getElementById('quantumContainer'),
    enableAging: document.getElementById('enableAging'),
    enableAgingSim: document.getElementById('enableAgingSim'),
    agingContainer: document.getElementById('agingContainer'),
    agingContainerSim: document.getElementById('agingContainerSim'),
    agingParams: document.getElementById('agingParams'),
    agingBoostAmount: document.getElementById('agingBoostAmount'),
    agingThreshold: document.getElementById('agingThreshold'),
    
    // Theme
    themeToggle: document.getElementById('themeToggle'),
    
    // Process Table
    processTable: document.getElementById('processTable'),
    processTableBody: document.getElementById('processTableBody'),
    processCount: document.getElementById('processCount'),
    addRowBtn: document.getElementById('addRowBtn'),
    loadCsvBtn: document.getElementById('loadCsvBtn'),
    csvFileInput: document.getElementById('csvFileInput'),
    
    // Simulation
    currentTime: document.getElementById('currentTime'),
    cpuBox: document.getElementById('cpuBox'),
    cpuStatus: document.getElementById('cpuStatus'),
    readyQueue: document.getElementById('readyQueue'),
    ganttChart: document.getElementById('ganttChart'),
    ganttAxis: document.getElementById('ganttAxis'),
    stepBtn: document.getElementById('stepBtn'),
    playBtn: document.getElementById('playBtn'),
    pauseBtn: document.getElementById('pauseBtn'),
    resetBtn: document.getElementById('resetBtn'),
    speedSlider: document.getElementById('speedSlider'),
    
    // Results
    avgWaitingTime: document.getElementById('avgWaitingTime'),
    avgTurnaroundTime: document.getElementById('avgTurnaroundTime'),
    avgResponseTime: document.getElementById('avgResponseTime'),
    resultsTableBody: document.getElementById('resultsTableBody'),
    
    // Log
    executionLog: document.getElementById('executionLog')
};

// === Module Initialization ===
window.initializeApp = function() {
    console.log("Initializing App...");
    initializeTheme(); // Set initial theme
    setupEventListeners();
    updateAlgorithmUI();
    addProcessRow(); // Start with one empty row
    updateResultsTable();
};

// === Event Listeners ===
function setupEventListeners() {
    // Algorithm changes
    elements.algorithmSelect.addEventListener('change', onAlgorithmChange);
    elements.algorithmSelectSim.addEventListener('change', onAlgorithmChangeSim);
    
    // Aging checkbox
    elements.enableAging.addEventListener('change', onAgingChange);
    elements.enableAgingSim.addEventListener('change', onAgingChangeSim);
    
    // Process management
    elements.addRowBtn.addEventListener('click', addProcessRow);
    elements.loadCsvBtn.addEventListener('click', () => elements.csvFileInput.click());
    elements.csvFileInput.addEventListener('change', handleCsvUpload);
    
    // Simulation controls
    elements.stepBtn.addEventListener('click', stepSimulation);
    elements.playBtn.addEventListener('click', startPlayback);
    elements.pauseBtn.addEventListener('click', pausePlayback);
    elements.resetBtn.addEventListener('click', resetSimulation);

    // Theme toggle
    elements.themeToggle.addEventListener('click', toggleTheme);
}

// === Theme Management ===
function initializeTheme() {
    const savedTheme = localStorage.getItem('theme');
    // Default to light (no attribute), but respect saved 'dark'
    if (savedTheme === 'dark') {
        document.documentElement.setAttribute('data-theme', 'dark');
    }
}

function toggleTheme() {
    const currentTheme = document.documentElement.getAttribute('data-theme');
    const newTheme = currentTheme === 'dark' ? 'light' : 'dark';
    
    if (newTheme === 'dark') {
        document.documentElement.setAttribute('data-theme', 'dark');
        localStorage.setItem('theme', 'dark');
    } else {
        document.documentElement.removeAttribute('data-theme');
        localStorage.setItem('theme', 'light');
    }
}

// === Algorithm UI ===
function onAlgorithmChange() {
    const algo = elements.algorithmSelect.value;
    elements.algorithmSelectSim.value = algo;
    updateAlgorithmUI();
}

function onAlgorithmChangeSim() {
    const algo = elements.algorithmSelectSim.value;
    elements.algorithmSelect.value = algo;
    updateAlgorithmUI();
}

function updateAlgorithmUI() {
    const algo = elements.algorithmSelect.value;
    const isPriority = algo === 'Priority' || algo === 'PriorityNP';
    const isRR = algo === 'RR';
    
    // Toggle Quantum UI
    if (isRR) {
        elements.quantumContainer.classList.add('visible');
    } else {
        elements.quantumContainer.classList.remove('visible');
    }
    
    // Toggle Aging UI
    if (isPriority) {
        elements.agingContainer.classList.add('visible');
        elements.agingContainerSim.classList.add('visible');
        updateAgingParamsVisibility();
    } else {
        elements.agingContainer.classList.remove('visible');
        elements.agingContainerSim.classList.remove('visible');
        elements.agingParams.classList.remove('visible');
    }
    
    // Toggle Priority Column
    if (isPriority) {
        elements.processTable.classList.add('show-priority');
        restorePriorityValues();
    } else {
        cachePriorityValues();
        elements.processTable.classList.remove('show-priority');
    }
}

function onAgingChange() {
    const enabled = elements.enableAging.checked;
    elements.enableAgingSim.checked = enabled;
    updateAgingParamsVisibility();
}

function onAgingChangeSim() {
    const enabled = elements.enableAgingSim.checked;
    elements.enableAging.checked = enabled;
    updateAgingParamsVisibility();
}

function updateAgingParamsVisibility() {
    if (elements.enableAging.checked) {
        elements.agingParams.classList.add('visible');
    } else {
        elements.agingParams.classList.remove('visible');
    }
}

// === Priority Caching ===
function cachePriorityValues() {
    const rows = elements.processTableBody.querySelectorAll('tr');
    rows.forEach((row, index) => {
        const priorityInput = row.querySelector('.priority-input');
        if (priorityInput) {
            priorityCache[index] = priorityInput.value;
        }
    });
}

function restorePriorityValues() {
    const rows = elements.processTableBody.querySelectorAll('tr');
    rows.forEach((row, index) => {
        const priorityInput = row.querySelector('.priority-input');
        if (priorityInput && priorityCache[index] !== undefined) {
            priorityInput.value = priorityCache[index];
        }
    });
}

// === Process Table Management ===
function addProcessRow() {
    const rowCount = elements.processTableBody.querySelectorAll('tr').length;
    const nextName = `P${rowCount + 1}`;
    
    const row = document.createElement('tr');
    row.innerHTML = `
        <td><input type="text" class="name-input" value="${nextName}" placeholder="Name"></td>
        <td><input type="number" class="arrival-input" value="0" min="0"></td>
        <td><input type="number" class="burst-input" value="5" min="1"></td>
        <td class="priority-col"><input type="number" class="priority-input" value="0" min="0"></td>
        <td class="action-col"><button class="delete-btn" onclick="removeProcessRow(this)">×</button></td>
    `;
    
    elements.processTableBody.appendChild(row);
    updateProcessCount();
    syncProcessesFromTable();
    updateResultsTable();
}

function removeProcessRow(btn) {
    const row = btn.closest('tr');
    row.remove();
    renameProcesses(); // Auto-rename after deletion
    updateProcessCount();
    syncProcessesFromTable();
    updateResultsTable();
}

function renameProcesses() {
    const rows = elements.processTableBody.querySelectorAll('tr');
    rows.forEach((row, index) => {
        const nameInput = row.querySelector('.name-input');
        // Only rename if it follows the P# pattern
        if (nameInput && /^P\d+$/.test(nameInput.value)) {
            nameInput.value = `P${index + 1}`;
        }
    });
}

// Make globally accessible
window.removeProcessRow = removeProcessRow;

function updateProcessCount() {
    const count = elements.processTableBody.querySelectorAll('tr').length;
    elements.processCount.textContent = `(${count} processes)`;
}

function syncProcessesFromTable() {
    processes = [];
    const rows = elements.processTableBody.querySelectorAll('tr');
    
    rows.forEach((row, index) => {
        const name = row.querySelector('.name-input').value.trim() || `P${index + 1}`;
        const arrival = parseInt(row.querySelector('.arrival-input').value) || 0;
        const burst = parseInt(row.querySelector('.burst-input').value) || 1;
        const priorityInput = row.querySelector('.priority-input');
        const priority = priorityInput ? (parseInt(priorityInput.value) || 0) : 0;
        
        processes.push({
            id: index + 1,
            name: name,
            arrival: arrival,
            burst: burst,
            priority: priority
        });
    });
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
            if (parts.length >= 4) {
                addProcessRowWithData(
                    parts[1] || `P${processIdCounter}`,
                    parseInt(parts[2]) || 0,
                    parseInt(parts[3]) || 1,
                    parseInt(parts[4]) || 0
                );
            }
        });
        
        updateResultsTable();
    };
    reader.readAsText(file);
    event.target.value = '';
}

function addProcessRowWithData(name, arrival, burst, priority) {
    const row = document.createElement('tr');
    
    row.innerHTML = `
        <td><input type="text" class="name-input" value="${name}" placeholder="Name"></td>
        <td><input type="number" class="arrival-input" value="${arrival}" min="0"></td>
        <td><input type="number" class="burst-input" value="${burst}" min="1"></td>
        <td class="priority-col"><input type="number" class="priority-input" value="${priority}" min="0"></td>
        <td><button class="delete-btn" onclick="removeProcessRow(this)">×</button></td>
    `;
    
    elements.processTableBody.appendChild(row);
    processIdCounter++;
    updateProcessCount();
    syncProcessesFromTable();
}

// === Results Table (Live Updates) ===
function updateResultsTable() {
    syncProcessesFromTable();
    elements.resultsTableBody.innerHTML = '';
    
    processes.forEach(p => {
        const row = document.createElement('tr');
        row.id = `result-row-${p.id}`;
        row.className = 'state-not-arrived';
        row.innerHTML = `
            <td>${p.name}</td>
            <td>${p.arrival}</td>
            <td>${p.burst}</td>
            <td class="waiting-cell">-</td>
            <td class="finish-cell">-</td>
            <td class="tat-cell">-</td>
            <td class="response-cell">-</td>
        `;
        elements.resultsTableBody.appendChild(row);
    });
}

function updateResultsTableFromState(state) {
    const currentTime = state.time;
    
    // First, mark all as not-arrived
    processes.forEach(p => {
        const row = document.getElementById(`result-row-${p.id}`);
        if (row) {
            row.className = p.arrival > currentTime ? 'state-not-arrived' : '';
        }
    });
    
    // Mark processes in ready queue as waiting
    if (state.ready_queue) {
        state.ready_queue.forEach(p => {
            const row = document.getElementById(`result-row-${p.id}`);
            if (row) {
                row.className = 'state-waiting';
                const waitingCell = row.querySelector('.waiting-cell');
                const localProcess = processes.find(proc => proc.id === p.id);
                if (localProcess) {
                    const waitTime = currentTime - localProcess.arrival - (localProcess.burst - p.remaining);
                    waitingCell.textContent = waitTime >= 0 ? waitTime : '-';
                }
            }
        });
    }
    
    if (state.cpu_process) {
        const row = document.getElementById(`result-row-${state.cpu_process.id}`);
        if (row) {
            row.className = 'state-running';
            const localProcess = processes.find(proc => proc.id === state.cpu_process.id);
            if (localProcess) {
                const waitTime = currentTime - localProcess.arrival - (localProcess.burst - state.cpu_process.remaining);
                row.querySelector('.waiting-cell').textContent = waitTime >= 0 ? waitTime : '-';
            }
        }
    }
    
    if (state.finished) {
        state.finished.forEach(p => {
            const row = document.getElementById(`result-row-${p.id}`);
            if (row) {
                row.className = 'state-finished';
                row.querySelector('.waiting-cell').textContent = p.waiting_time;
                
                const proc = processes.find(proc => proc.id === p.id);
                if (proc) {
                    row.querySelector('.finish-cell').textContent = proc.arrival + p.turnaround_time;
                }
                
                row.querySelector('.tat-cell').textContent = p.turnaround_time;
                row.querySelector('.response-cell').textContent = p.response_time;
            }
        });
        
        // Update averages
        if (state.finished.length > 0) {
            let totalWait = 0, totalTAT = 0, totalResponse = 0;
            state.finished.forEach(p => {
                totalWait += p.waiting_time;
                totalTAT += p.turnaround_time;
                totalResponse += p.response_time;
            });
            const n = state.finished.length;
            elements.avgWaitingTime.textContent = (totalWait / n).toFixed(2);
            elements.avgTurnaroundTime.textContent = (totalTAT / n).toFixed(2);
            elements.avgResponseTime.textContent = (totalResponse / n).toFixed(2);
        }
    }
}

// === Simulation Control ===
function initializeScheduler() {
    if (typeof Module === 'undefined' || !Module.Scheduler) {
        console.error('WASM Module not available');
        alert('Error: WASM module not loaded. Please refresh the page.');
        return false;
    }
    
    syncProcessesFromTable();
    
    if (processes.length === 0) {
        alert('Please add at least one process.');
        return false;
    }
    
    scheduler = new Module.Scheduler();
    
    const algorithm = elements.algorithmSelect.value;
    const quantum = parseInt(elements.timeQuantum.value) || 2;
    const agingEnabled = elements.enableAging.checked;
    const agingThreshold = parseInt(elements.agingThreshold.value) || 5;
    const agingBoostAmount = parseInt(elements.agingBoostAmount.value) || 1;
    
    scheduler.setAlgorithm(algorithm);
    scheduler.setTimeQuantum(quantum);
    scheduler.setAging(agingEnabled);
    scheduler.setAgingThreshold(agingThreshold);
    scheduler.setAgingBoostAmount(agingBoostAmount);
    
    processes.forEach(p => {
        scheduler.addProcess(p.id, p.name, p.arrival, p.burst, p.priority);
    });
    
    // Reset UI state
    ganttData = [];
    elements.ganttChart.innerHTML = '';
    elements.ganttAxis.innerHTML = '';
    elements.executionLog.innerHTML = '';
    updateResultsTable();
    
    return true;
}

function stepSimulation() {
    if (!scheduler && !initializeScheduler()) {
        return;
    }
    
    if (scheduler.isFinished()) {
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
    updateResultsTableFromState(state);
    addLogEntry(tickLog);
    
    // Check if finished
    if (scheduler.isFinished()) {
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
        scheduler.delete();
        scheduler = null;
    }
    
    ganttData = [];
    
    elements.currentTime.textContent = '0';
    elements.cpuBox.classList.remove('running');
    elements.cpuStatus.innerHTML = '<span class="idle">IDLE</span>';
    elements.readyQueue.innerHTML = '';
    elements.ganttChart.innerHTML = '';
    elements.ganttAxis.innerHTML = '';
    elements.executionLog.innerHTML = '';
    elements.avgWaitingTime.textContent = '-';
    elements.avgTurnaroundTime.textContent = '-';
    elements.avgResponseTime.textContent = '-';
    
    updateResultsTable();
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
        elements.readyQueue.innerHTML = '<span class="ready-queue-empty">Empty</span>';
        return;
    }
    
    state.ready_queue.forEach(p => {
        const item = document.createElement('div');
        item.className = 'ready-queue-item';
        
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
    const currentTick = state.time - 1;
    const processName = state.last_executed ? state.last_executed.name : null;
    
    // Check if we should merge with previous block
    if (ganttData.length > 0) {
        const lastBlock = ganttData[ganttData.length - 1];
        if (lastBlock.name === processName) {
            // Extend the last block
            lastBlock.endTime = currentTick + 1;
            lastBlock.duration++;
            renderGanttChart();
            return;
        }
    }
    
    // Add new block
    ganttData.push({
        name: processName,
        startTime: currentTick,
        endTime: currentTick + 1,
        duration: 1
    });
    
    renderGanttChart();
}

function renderGanttChart() {
    elements.ganttChart.innerHTML = '';
    elements.ganttAxis.innerHTML = '';
    
    const blockWidth = 40; // Base width per time unit
    let totalWidth = 0;
    
    // First pass: create blocks and calculate total width
    ganttData.forEach(block => {
        const div = document.createElement('div');
        div.className = 'gantt-block';
        const width = block.duration * blockWidth;
        div.style.width = `${width}px`;
        
        if (block.name) {
            div.classList.add('running');
            div.textContent = block.name;
        } else {
            div.classList.add('idle');
            div.textContent = '';
        }
        
        elements.ganttChart.appendChild(div);
        totalWidth += width;
    });
    
    // Set axis width
    elements.ganttAxis.style.width = `${totalWidth}px`;
    elements.ganttAxis.style.position = 'relative';
    elements.ganttAxis.style.height = '20px';
    
    // Second pass: create time markers
    // First block: start + end markers. All other blocks: only end marker.
    let cumWidth = 0;
    
    ganttData.forEach((block, index) => {
        const blockPixelWidth = block.duration * blockWidth;
        
        // Start time marker only for first block
        if (index === 0) {
            const startMarker = document.createElement('span');
            startMarker.className = 'gantt-axis-marker';
            startMarker.style.left = `${cumWidth}px`;
            startMarker.textContent = block.startTime;
            elements.ganttAxis.appendChild(startMarker);
        }
        
        cumWidth += blockPixelWidth;
        
        // End time marker for all blocks
        const endMarker = document.createElement('span');
        endMarker.className = 'gantt-axis-marker';
        endMarker.style.left = `${cumWidth}px`;
        endMarker.textContent = block.endTime;
        elements.ganttAxis.appendChild(endMarker);
    });
    
    // Auto-scroll
    elements.ganttChart.parentElement.scrollLeft = elements.ganttChart.parentElement.scrollWidth;
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

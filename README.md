# Process Scheduling Project

A C++ Project implementing various CPU scheduling algorithms, designed to target:
- **Native Linux** (for testing/development)
- **WebAssembly** (Emscripten) for web demos
- **Windows** (MinGW Cross-Compile) for desktop usage

## Overview

This project provides a core `scheduler_lib` that implements a CPU scheduling simulation. It supports the following algorithms:
- **FCFS** (First-Come, First-Served)
- **SJF** (Shortest Job First) - Non-preemptive
- **SRTF** (Shortest Remaining Time First) - Preemptive
- **RR** (Round Robin) - Time Quantum based. *Logic note: Preempted processes return to queue before simultaneous arrivals.*
- **Priority** - Non-preemptive (Lower value = Higher priority)

## Project Structure
- `src/`: Source code.
  - `scheduler.cpp`: Core scheduling logic.
  - `wasm_main.cpp`: WebAssembly entry point.
  - `server_main.cpp`: Native executable entry point.
- `include/`: Header files.
  - `scheduler.h`: API definition.
  - `json.hpp`: JSON library (nlohmann).
  - `httplib.h`: OS-specific HTTP server utils.
- `cmake/`: Toolchain files (e.g., MinGW).
- `tests/`: Local test runners.
- `www/`: Web assets.

## Dependencies

- **C++17** Compliant Compiler (GCC/Clang)
- **CMake** (3.10+)
- **Emscripten** (for WebAssembly builds)
- **MinGW-w64** (for Windows builds from Linux)

## Build Instructions

### 1. Native Linux (Test / Dev)
Use this build## 📂 Project Structure

- **`include/scheduler.h`**: Defines the `Process` struct and `Scheduler` class interface.
- **`src/scheduler.cpp`**: Core implementation of scheduling algorithms (`tick()` logic) and state management.
- **`tests/test_runner.cpp`**: Comprehensive test suite that runs all algorithms back-to-back to verify correctness.
- **`src/server_main.cpp`**: Entry point for the planned native server/CLI.
- **`src/wasm_main.cpp`**: Entry point for the WebAssembly build (for web interface).
- **`cmake/`**: Toolchain files for cross-compilation (MinGW).

## 🧠 algorithms Implemented

The scheduler supports the following algorithms, controlled via `setAlgorithm(name)`:

1.  **FCFS (First Come First Served)**:
    - Processes are executed strictly in the order of arrival.
    - Non-preemptive.

2.  **SJF (Shortest Job First)**:
    - Selects the process with the shortest **total burst time** from the ready queue.
    - **Non-Preemptive**: Once started, a process runs to completion.
    - **Aging**: Supported (if enabled) to prevent starvation.

3.  **SRTF (Shortest Remaining Time First)**:
    - Preemptive version of SJF.
    - Continually checks if a new process has a shorter **remaining time** than the current process.

4.  **RR (Round Robin)**:
    - Processes are given a fixed time slice (`timeQuantum`).
    - If a process exceeds its quantum, it is moved to the back of the ready queue.
    - **Logic**: New arrivals are queued *after* the currently preempted process (if simultaneous).

5.  **Priority (Preemptive)**:
    - Processes have a `priority` value (Lower value = Higher Priority).
    - If a process arrives with higher priority than the current one, the CPU is **preempted**.
    - **Aging**: Increases priority of waiting processes every 5 ticks to prevent starvation.

6.  **PriorityNP (Non-Preemptive)**:
    - Similar to Priority, but the currently running process is **never** preempted by a new arrival.
    - **Aging**: Supported.

### ⚙️ Feature: Aging
- **Mechanism**: Every 5 ticks a process waits in the queue, its priority value is decremented (priority increased) by 1.
- **Toggle**: Can be enabled/disabled via `scheduler.setAging(bool)`.

## 🧪 Testing

We use a custom test runner to verify all algorithms.

### How to Run Tests
1.  Create a build directory (if not exists):
    ```bash
    mkdir -p build-test && cd build-test
    ```
2.  Configure and Build:
    ```bash
    cmake ..
    make
    ```
3.  Run the Test Runner:
    ```bash
    ./scheduler_test
    ```

**What the specific command does:**
- The `scheduler_test` executable runs a series of scenarios (FCFS, SJF, SRTF, RR, Priority, PriorityNP).
- It prints a simulation log for every tick (e.g., "Time 0: Running Process 1").
- It calculates and displays **Waiting Time** and **Turnaround Time** stats for each test case.
- You can observe the effects of **Aging** (explicit logs showing priority changes) and **Preemption** in the output.://emscripten.org/).
```bash
mkdir -p build-wasm
cd build-wasm
emcmake cmake ..
make
```
This generates `scheduler_wasm.js` (and potentially `.html` / `.wasm` depending on configuration).

### 3. Windows (Cross-Compile)
Requires `mingw-w64` toolchain installed on your Linux host.
```bash
mkdir -p build-win
cd build-win
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/mingw-w64-x86_64.cmake ..
make
```
This generates `scheduler_server.exe` with static linking.

## Testing

The project includes a `scheduler_test` executable.
It runs a hardcoded simulation of processes using the **Round Robin** algorithm (Quantum=2) by default and prints:
1. Step-by-step Execution Log.
2. Final Statistics (Waiting Time, Turnaround Time).

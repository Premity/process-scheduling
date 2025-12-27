# CPU Scheduler Simulator

An interactive CPU scheduling simulator with a WebAssembly-powered web interface.

[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![WebAssembly](https://img.shields.io/badge/WebAssembly-Enabled-purple.svg)](https://webassembly.org/)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

---

## Features

- **Six Scheduling Algorithms**: FCFS, SJF, SRTF, Round Robin, Priority (Preemptive & Non-Preemptive)
- **Aging Mechanism**: Configurable priority boost to prevent starvation
- **Interactive Web UI**
  - Real-time Gantt Chart
  - Live Ready Queue visualization
  - Process Statistics with color-coded states
  - Light/Dark Mode Toggle
  - Table-based process input
- **Metrics**: Waiting Time, Turnaround Time, Response Time

---

## Quick Start

### 🐧 Linux / macOS

1.  **Dependencies**: Install `git`, `python3`, `cmake`, and `g++`.

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install git python3 cmake g++
```
2.  **Install Emscripten**:
```bash
    git clone https://github.com/emscripten-core/emsdk.git && cd emsdk
    ./emsdk install latest && ./emsdk activate latest
    source ./emsdk_env.sh
    cd ..
```
3.  **Build**:
```bash
    git clone https://github.com/Premity/process-scheduling.git
    cd process-scheduling
    mkdir build && cd build
    emcmake cmake .. && emmake make
    cp scheduler_wasm.js scheduler_wasm.wasm ../www/
```
4.  **Run**:
```bash
    cd ..
    g++ -std=c++17 src/server_main.cpp -o scheduler_server -I include -lpthread
    ./scheduler_server
```

### 🪟 Windows (PowerShell)

1.  **Prerequisites**:
    *   Install [Git](https://git-scm.com/download/win)
    *   Install [Python 3](https://www.python.org/downloads/)
    *   Install [MinGW-w64](https://www.mingw-w64.org/) (ensure `g++` and `mingw32-make` are in PATH)
    *   Install [CMake](https://cmake.org/download/)

2.  **Install Emscripten**:
```powershell
    git clone https://github.com/emscripten-core/emsdk.git
    cd emsdk
    ./emsdk install latest
    ./emsdk activate latest
    ./emsdk_env.bat
    cd ..
```

3.  **Build**:
```powershell
    git clone https://github.com/Premity/process-scheduling.git
    cd process-scheduling
    mkdir build; cd build
    emcmake cmake -G "MinGW Makefiles" ..
    emmake mingw32-make
    copy scheduler_wasm.js ..\www\
    copy scheduler_wasm.wasm ..\www\
```

4.  **Run**:
```powershell
    cd ..
    # Compile server (linking ws2_32 for Windows sockets)
    g++ -std=c++17 src/server_main.cpp -o scheduler_server.exe -I include -lws2_32 -static
    .\scheduler_server.exe
```

---

## Project Structure

```
.
├── include/
│   ├── scheduler.h       # Core scheduler API
│   └── json.hpp          # nlohmann/json (header-only)
├── src/
│   ├── scheduler.cpp     # Scheduler implementation
│   ├── wasm_main.cpp     # WebAssembly bindings
│   └── server_main.cpp   # Native C++ static file server
├── www/                  # Web UI (HTML, CSS, JS)
├── CMakeLists.txt
└── README.md
```

---

## Scheduling Algorithms

| Algorithm | Type | Selection Criteria |
|-----------|------|-------------------|
| FCFS | Non-Preemptive | Earliest arrival |
| SJF | Non-Preemptive | Shortest burst time |
| SRTF | Preemptive | Shortest remaining time |
| Round Robin | Preemptive | Time quantum rotation |
| Priority | Preemptive | Lowest priority value |
| PriorityNP | Non-Preemptive | Lowest priority value |

### Aging Mechanism

Prevents starvation by boosting priority of waiting processes:

```cpp
scheduler.setAging(true);
scheduler.setAgingThreshold(5);    // Boost every 5 ticks
scheduler.setAgingBoostAmount(1);  // Decrease priority by 1
```

---

## License

MIT License — see [LICENSE](LICENSE)

---

## Author

**Mohammad Hamd Ashfaque**  
GitHub: [@Premity](https://github.com/Premity)

---

**Version**: 1.0.0 | **Last Updated**: December 2025
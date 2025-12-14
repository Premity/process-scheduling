# CPU Scheduler Simulator

A comprehensive C++ implementation of CPU scheduling algorithms with support for multiple platforms and deployment targets.

[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![CMake](https://img.shields.io/badge/CMake-3.10+-green.svg)](https://cmake.org/)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Project Structure](#project-structure)
- [Scheduling Algorithms](#scheduling-algorithms)
- [Getting Started](#getting-started)
- [Building the Project](#building-the-project)
- [Testing](#testing)
- [API Usage](#api-usage)
- [Dependencies](#dependencies)

---

## Overview

This project provides a **discrete-time CPU scheduling simulator** that models various process scheduling algorithms used in operating systems.

It is designed to be:

- **Educational** – clear implementations of classic OS scheduling algorithms
- **Portable** – targets native Linux, WebAssembly, and Windows
- **Extensible** – easy to add new algorithms or modify existing ones
- **Well-tested** – comprehensive test suite covering edge cases and stress scenarios

The core `scheduler_lib` can be used as a standalone simulator or embedded as a library.

---

## Features

### Currently Implemented

- **Six Scheduling Algorithms**
  - FCFS
  - SJF
  - SRTF
  - Round Robin
  - Priority (Preemptive & Non-Preemptive)
- **Aging Mechanism** to prevent starvation
- **Metrics**
  - Waiting Time
  - Turnaround Time
  - Response Time
  - CPU Utilization
- **Test Suite** (30+ tests)
- **JSON State Export**
- **Detailed Tick-by-Tick Logging**

---

## Project Structure

```
.
├── include/
│   ├── scheduler.h        # Core scheduler API and Process struct
│   ├── json.hpp           # nlohmann/json (header-only)
│   └── httplib.h          # HTTP utilities
├── src/
│   ├── scheduler.cpp     # Scheduler implementation
│   ├── server_main.cpp   # Native server entry point
│   └── wasm_main.cpp     # WebAssembly entry point
├── tests/
│   └── test_runner.cpp   # Comprehensive test suite
├── cmake/
│   └── mingw-w64-x86_64.cmake  # Windows toolchain
├── www/                  # Web UI assets
├── CMakeLists.txt
└── README.md

````

### Key Components

- **`scheduler.h`** – Scheduler API and PCB definition  
- **`scheduler.cpp`** – Core simulation and algorithms  
- **`test_runner.cpp`** – Automated testing framework  
- **`server_main.cpp`** – REST API server  
- **`wasm_main.cpp`** – WASM bindings  

---

## Scheduling Algorithms

### 1. FCFS (First-Come, First-Served)

- **Type**: Non-Preemptive  
- **Selection**: Earliest arrival  

```cpp
scheduler.setAlgorithm("FCFS");
````

---

### 2. SJF (Shortest Job First)

* **Type**: Non-Preemptive
* **Selection**: Shortest burst time

```cpp
scheduler.setAlgorithm("SJF");
scheduler.setAging(true); // optional prevent starvation
```

---

### 3. SRTF (Shortest Remaining Time First)

* **Type**: Preemptive
* **Selection**: Shortest remaining time

```cpp
scheduler.setAlgorithm("SRTF");
```

**Preemption Logic**

* Checked every tick
* Current process preempted if a shorter job arrives

---

### 4. Round Robin (RR)

* **Type**: Preemptive
* **Selection**: FCFS with time quantum

```cpp
scheduler.setAlgorithm("RR");
scheduler.setTimeQuantum(3);
```

**Important Detail**

> When a quantum expires, the process is placed **behind all processes that arrived during the same tick**.

---

### 5. Priority

* **Type**: Preemptive
* **Selection**: Lower priority value = higher priority

```cpp
scheduler.setAlgorithm("Priority");
scheduler.setAging(true);
scheduler.setAgingThreshold(5);
```

---

### 6. PriorityNP

* **Type**: Non-Preemptive

```cpp
scheduler.setAlgorithm("PriorityNP");
scheduler.setAging(true);
```

---

### Aging Mechanism

**How it works**

1. Each waiting process increments an age counter
2. When threshold is reached, priority improves
3. Counter resets after boost

```cpp
scheduler.setAging(true);
scheduler.setAgingThreshold(10);
```

Applies to: **SJF, SRTF, Priority, PriorityNP**

---

## Getting Started

### Prerequisites

* C++17 compiler
* CMake 3.10+
* Git

### Quick Start

```bash
git clone https://github.com/Premity/process-scheduling.git
cd process-scheduling

mkdir build && cd build
cmake ..
make

./scheduler_test
```

---

## Building the Project

### Native Linux Build

```bash
mkdir -p build
cd build
cmake ..
make
```

**Outputs**

* `scheduler_test`
* `libscheduler.a`

---

### WebAssembly Build (Planned)

```bash
emcmake cmake ..
emmake make
```

Outputs:

* `scheduler_wasm.js`
* `scheduler_wasm.wasm`

---

### Windows Cross-Compile

```bash
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/mingw-w64-x86_64.cmake ..
make
```

---

## Testing

### Test Categories

1. Basic functionality
2. Simultaneous arrivals
3. Preemption scenarios
4. Round Robin variations
5. Aging mechanism
6. Edge cases
7. Stress tests

### Running Tests

```bash
./scheduler_test
```

### Adding a Custom Test

```cpp
std::vector<PData> myTestSet = {
    {1, "ProcessA", 0, 10, 3},
    {2, "ProcessB", 2, 5, 1},
    {3, "ProcessC", 4, 8, 2}
};
```

---

## API Usage

### Example

```cpp
Scheduler scheduler;

scheduler.setAlgorithm("RR");
scheduler.setTimeQuantum(3);

scheduler.addProcess(1, "P1", 0, 10, 5);
scheduler.addProcess(2, "P2", 2, 5, 3);

while (!scheduler.isFinished()) {
    std::cout << scheduler.tick() << std::endl;
}

auto state = scheduler.getStateJSON();
std::cout << state.dump(4) << std::endl;
```

---

## 🔧 Dependencies

* C++17
* CMake
* nlohmann/json (MIT)
* cpp-httplib
* Emscripten
* MinGW-w64

---

## 📄 License

MIT License — see `LICENSE`

---

## 👤 Author

**Mohammad Hamd Ashfaque**
GitHub: [@Premity](https://github.com/Premity)

---

**Last Updated**: 14th December 2025
**Version**: 0.2.0
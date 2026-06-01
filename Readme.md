# Trace-Driven Multi-Level Cache Simulator

## Overview

This project is a configurable trace-driven(trace is a recorded sequence of memory accesses) multi-level cache simulator implemented in C++. It models the behavior of an L1-L2 cache hierarchy and evaluates cache performance under different memory access workloads.

The simulator supports configurable cache parameters such as cache size, block size, associativity, write policies, allocation policies, and memory latency. Memory accesses are replayed from trace files, allowing realistic analysis of cache behavior and performance.

---

## Features

### Cache Hierarchy

- Configurable L1 Cache
- Configurable L2 Cache
- Main Memory Latency Modeling
- Multi-Level Cache Access Flow

### Cache Organization

- Set-Associative Caches
- Configurable Block Size
- Configurable Associativity
- Tag-Based Lookup
- LRU Replacement Policy

### Write Policies

- Write-Back
- Write-Through

### Allocation Policies

- Write Allocate
- No Write Allocate

### Performance Metrics

- Read Hits and Misses
- Write Hits and Misses
- Miss Rates
- Writebacks
- Total Cycles
- Average Access Latency
- Average Memory Access Time (AMAT)

### Workload Support

- Sequential Access Traces
- Random Access Traces
- Mixed Read/Write Traces
- User-Defined Trace Files

### Output

- Console Statistics
- CSV Export for Analysis

---

## Project Structure

```text
CacheSimulator/
│
├── data/
│   ├── config.txt
│   ├── trace_sequential.txt
│   ├── trace_random.txt
│   ├── trace_rw_mixed.txt
│   └── ...
│
├── src/
│   ├── main.cpp
│   ├── CacheLevel.cpp
│   ├── CacheLevel.h
│   ├── MultiLevelCache.cpp
│   ├── MultiLevelCache.h
│   ├── MemoryTrace.cpp
│   └── MemoryTrace.h
│
├── CMakeLists.txt
├── README.md
└── cache_results.csv
```

---

## Cache Architecture

```text
Memory Trace
     |
     v
MultiLevelCache
     |
     +---- L1 Cache
     |
     +---- L2 Cache
     |
     +---- Main Memory
     |
     v
Statistics & CSV Export
```

---

## Address Mapping

For every memory access, the simulator computes:

### Block Number

```text
Block Number = Address / Block Size
```

### Set Index

```text
Set Index = Block Number % Number Of Sets
```

### Tag

```text
Tag = Block Number / Number Of Sets
```

### Example

Given:

```text
Address        = 0x1000
Block Size     = 64
Associativity  = 2
Cache Size     = 1024
```

Calculation:

```text
Block Number = 4096 / 64 = 64

Number of Blocks = 1024 / 64 = 16

Number of Sets = 16 / 2 = 8

Set Index = 64 % 8 = 0

Tag = 64 / 8 = 8
```

Result:

```text
Set 0
Tag 8
```

---

## Replacement Policy

### Least Recently Used (LRU)

The simulator uses LRU replacement.

When a set becomes full, the block that has not been accessed for the longest time is evicted.

Implementation:

- Doubly Linked List
- Hash Map
- O(1) Lookup
- O(1) Update
- O(1) Eviction

---

## Write Policies

### Write-Back

On a write hit:

- Cache block is updated
- Dirty bit is set
- Lower levels are not updated immediately

The modified block is written back only when evicted.

Advantages:

- Reduced memory traffic
- Improved write performance

---

### Write-Through

On a write hit:

- Cache is updated
- Lower cache levels are updated immediately

Advantages:

- Simpler consistency management
- Memory always contains the latest value

---

## Allocation Policies

### Write Allocate

On a write miss:

1. Fetch block into cache
2. Perform write operation

---

### No Write Allocate

On a write miss:

1. Do not cache the block
2. Forward write directly to lower levels

---

## Dirty Bit

Each cache line maintains a dirty bit.

```text
dirty = true
```

indicates:

```text
Cache Data ≠ Memory Data
```

Dirty blocks must be written back before eviction.

---

## Average Memory Access Time (AMAT)

The simulator reports:

```text
AMAT = Hit Time + Miss Rate × Miss Penalty
```

This metric estimates the average cost of memory accesses.

---

## Input Format

Example trace:

```text
R 0x1000
W 0x1040
R 0x1080
R 0x1000
```

Where:

- `R` = Read
- `W` = Write

---

## Configuration

Example `config.txt`

```text
L1CacheSize=1024
L1BlockSize=64
L1Associativity=2
L1Latency=1

WritePolicyL1=1
AllocatePolicyL1=1

L2CacheSize=8192
L2BlockSize=64
L2Associativity=4
L2Latency=10

WritePolicyL2=1
AllocatePolicyL2=1

MemoryLatency=100

TraceFile=data/trace_rw_mixed.txt
```

---

## Build Instructions

### Clone Repository

```bash
git clone <repository-url>
cd CacheSimulator
```

### Create Build Directory

```bash
mkdir build
cd build
```

### Configure Project

```bash
cmake ..
```

### Build

```bash
cmake --build .
```

### Run

Windows:

```bash
.\Debug\cache_simulator.exe
```

Linux/macOS:

```bash
./cache_simulator
```

---

## Example Output

```text
Multi-Level Cache Simulation Results:

Total Accesses: 15
Average Access Latency (cycles): 47.67

AMAT: 52.33 cycles

L1 miss rate: 0.4667
L2 miss rate: 0.6000

L1 write-backs to lower: 4
L2 write-backs to lower: 2

Misses to memory: 6
```

---

## Validation

The simulator was validated using targeted test traces covering:

- Repeated Accesses
- Spatial Locality
- Write-Back Behavior
- LRU Eviction
- Dirty Block Eviction
- Cache-Friendly Workloads
- Random Workloads

---

## Key Concepts Demonstrated

- Cache Hierarchies
- Set-Associative Caches
- Tag-Based Address Mapping
- LRU Replacement
- Dirty Bit Management
- Write-Back and Write-Through Policies
- Write Allocate and No Write Allocate Policies
- Multi-Level Memory Systems
- Performance Analysis
- Average Memory Access Time (AMAT)

---

## Future Improvements

- Next-Line Prefetching
- Inclusive and Exclusive Cache Policies
- Miss Classification (Compulsory, Capacity, Conflict)
- More Accurate Multi-Level AMAT Calculation
- Cache Visualization Tools
- Additional Replacement Policies (FIFO, Random)

---

## Technologies Used

- C++
- STL
- CMake

---

## License

MIT License
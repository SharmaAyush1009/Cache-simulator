#ifndef MULTILEVELCACHE_H
#define MULTILEVELCACHE_H

#include "CacheLevel.h"
#include <iostream>

class MultiLevelCache {
private:
    CacheLevel L1;
    CacheLevel L2;
    int memoryLatency;

    long long totalAccesses = 0;
    long long L1Hits = 0, L2Hits = 0, Misses = 0;
    long long totalCycles = 0;

    // write-through lower writes and write-back evictions counts
    long long writesToLower = 0;
    long long writesToMemory = 0;
    bool enablePrefetch = false;
    void prefetchNextLine(int address);
    
public:
    MultiLevelCache(const CacheLevel &l1, const CacheLevel &l2, int memLatency) : L1(l1), L2(l2), memoryLatency(memLatency) {}

    // Access with read/write flag
    int access(int address, bool isWrite);
    const CacheLevel& getL1() const { return L1; }
    const CacheLevel& getL2() const { return L2; }
    int getMemoryLatency() const { return memoryLatency; }

    long long getTotalAccesses() const { return totalAccesses; }
    long long getTotalCycles() const { return totalCycles; }
    long long getMissesToMemory() const { return Misses; }
    void printStats();
    void exportToCSV(const std::string& filename, const std::string& configName = "Test");
    void setPrefetch(bool val) { enablePrefetch = val; }
    bool getPrefetch() const { return enablePrefetch; }

};

#endif

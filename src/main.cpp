#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include "CacheLevel.h"
#include "MultiLevelCache.h"
#include "MemoryTrace.h"

static int choosePolicy(const std::string& prompt, const std::vector<std::string>& opts) {
    int choice = -1;
    while (choice < 1 || choice > (int)opts.size()) {
        std::cout << prompt << "\n";
        for (size_t i = 0; i < opts.size(); ++i) {
            std::cout << "  " << (i + 1) << ". " << opts[i] << "\n";
        }
        std::cout << "Enter choice [1-" << opts.size() << "]: ";
        std::cin >> choice;
    }
    return choice;
}

int main() {
    std::srand((unsigned)std::time(nullptr));
    
    // Read config file
    std::ifstream configFile("data/config.txt");
    if (!configFile.is_open()) {
        std::cerr << "Config file not found: data/config.txt" << std::endl;
        return 1;
    }
    
    // Initialize variables
    int L1CacheSize, L1BlockSize, L1Associativity, L1Latency;
    int WritePolicyL1, AllocatePolicyL1;
    int L2CacheSize, L2BlockSize, L2Associativity, L2Latency;
    int WritePolicyL2, AllocatePolicyL2;
    int MemoryLatency, TraceType;
    std::string TraceFile;
    
    // Parse config file
    std::string line;
    while (std::getline(configFile, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;
        
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        
        if (key == "L1CacheSize") L1CacheSize = std::stoi(value);
        else if (key == "L1BlockSize") L1BlockSize = std::stoi(value);
        else if (key == "L1Associativity") L1Associativity = std::stoi(value);
        else if (key == "L1Latency") L1Latency = std::stoi(value);
        else if (key == "WritePolicyL1") WritePolicyL1 = std::stoi(value);
        else if (key == "AllocatePolicyL1") AllocatePolicyL1 = std::stoi(value);
        else if (key == "L2CacheSize") L2CacheSize = std::stoi(value);
        else if (key == "L2BlockSize") L2BlockSize = std::stoi(value);
        else if (key == "L2Associativity") L2Associativity = std::stoi(value);
        else if (key == "L2Latency") L2Latency = std::stoi(value);
        else if (key == "WritePolicyL2") WritePolicyL2 = std::stoi(value);
        else if (key == "AllocatePolicyL2") AllocatePolicyL2 = std::stoi(value);
        else if (key == "MemoryLatency") MemoryLatency = std::stoi(value);
        else if (key == "TraceType") TraceType = std::stoi(value);
        else if (key == "TraceFile") TraceFile = value;
    }
    configFile.close();
    
    std::cout << "Cache Simulator - Config Loaded\n";
    std::cout << "L1: " << L1CacheSize << " bytes, " << L1Associativity << "-way\n";
    std::cout << "L2: " << L2CacheSize << " bytes, " << L2Associativity << "-way\n";
    std::cout << "Trace: " << TraceFile << "\n\n";
    
    // Load trace file directly
    std::vector<std::pair<int,bool>> rwOps = MemoryTrace::readTraceFileRW(TraceFile);
    if (rwOps.empty()) {
        std::cout << "Trace file empty: " << TraceFile << "\n";
        return 1;
    }
    
    // Build cache levels
    CacheLevel L1(L1CacheSize, L1BlockSize, L1Associativity, L1Latency,
                  WritePolicyL1 == 1 ? WritePolicy::WriteBack : WritePolicy::WriteThrough,
                  AllocatePolicyL1 == 1 ? AllocatePolicy::WriteAllocate : AllocatePolicy::NoWriteAllocate);
    
    CacheLevel L2(L2CacheSize, L2BlockSize, L2Associativity, L2Latency,
                  WritePolicyL2 == 1 ? WritePolicy::WriteBack : WritePolicy::WriteThrough,
                  AllocatePolicyL2 == 1 ? AllocatePolicy::WriteAllocate : AllocatePolicy::NoWriteAllocate);
    
    MultiLevelCache cacheHierarchy(L1, L2, MemoryLatency);
    
    // Simulate with loaded trace
    long long reads = 0, writes = 0;
    for (auto &op : rwOps) {
        if (op.second) writes++; else reads++;
        cacheHierarchy.access(op.first, op.second);
    }
    
    // Results
    cacheHierarchy.printStats();
    
    std::string configName = "L1_" + std::to_string(L1CacheSize) + "_L2_" + std::to_string(L2CacheSize);
    cacheHierarchy.exportToCSV("cache_results.csv", configName);
    
    std::cout << "Total Reads: " << reads << ", Total Writes: " << writes << "\n";
    return 0;
}

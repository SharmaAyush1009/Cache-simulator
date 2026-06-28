// MultiLevelCache.cpp
#include "MultiLevelCache.h"
#include <fstream>
#include <iomanip>

int MultiLevelCache::access(int address, bool isWrite) {
    totalAccesses++;

    // L1 access
    auto [hitL1, evictDirtyL1] = L1.access(address, isWrite);
    int cycles = L1.getLatency();

    if (hitL1) {
        // If write-through, forward write to L2/memory (handled here)
        if (isWrite && L1.getWritePolicy() == WritePolicy::WriteThrough) {
            // For WT in L1, we must write to lower level.
            // Try to write into L2 (treat as write, may be WT/WB depending on L2 policy)
            auto [hitL2onWT, evictDirtyL2onWT] = L2.access(address, true);
            cycles += L2.getLatency();
            writesToLower++;
            if (!hitL2onWT) {
                // Miss in L2 on WT; for correctness, memory is updated too
                L2.insertOnFill(address, false);
                cycles += memoryLatency;
                writesToMemory++;
            }
            if (evictDirtyL2onWT) {
                // Dirty eviction from L2 => write to memory
                cycles += memoryLatency;
                writesToMemory++;
            }
        }
        L1Hits++;
        totalCycles += cycles;
        return cycles;
    }

    // L1 miss => Access L2 (read or write)
    auto [hitL2, evictDirtyL2] = L2.access(address, isWrite);
    cycles += L2.getLatency();

    if (hitL2) {
        L2Hits++;
        // On L2 hit after L1 miss, fill L1
        bool markDirty = isWrite && (L1.getWritePolicy() == WritePolicy::WriteBack);
        EvictionInfo evicted;
        if (!(isWrite && L1.getAllocatePolicy() == AllocatePolicy::NoWriteAllocate)) {
            evicted = L1.insertOnFill(address, markDirty);
        }
        if (evicted.valid && evicted.dirty){
            int evictedAddress = evicted.blockNumber * L1.getBlockSize();

            auto [h2,e2] = L2.access(evictedAddress,true);

            cycles += L2.getLatency();
            writesToLower++;
            if (!h2) {
                L2.insertOnFill(evictedAddress, true); 
            }

            if (e2) {
                cycles += memoryLatency;
                writesToMemory++;
            }
        }
        totalCycles += cycles;
        return cycles;
    }

    // Miss in both L1 and L2 => go to memory
    Misses++;
    cycles += memoryLatency;
    prefetchNextLine(address);

    // Fill L2
    bool markDirtyL2 = isWrite && (L2.getWritePolicy() == WritePolicy::WriteBack); // if L2 is WB and write-allocate, it will record dirty
    EvictionInfo evictedL2;
    if (!(isWrite && L2.getAllocatePolicy() == AllocatePolicy::NoWriteAllocate)) {
        evictedL2 = L2.insertOnFill(address, markDirtyL2);
    }
    if (evictedL2.valid && evictedL2.dirty) {
        // Dirty eviction at L2 requires writing to memory
        cycles += memoryLatency;
        writesToMemory++;
    }

    // Fill L1
    bool markDirtyL1 = isWrite && (L1.getWritePolicy() == WritePolicy::WriteBack);
    EvictionInfo evictedL1;
    if (!(isWrite && L1.getAllocatePolicy() == AllocatePolicy::NoWriteAllocate)) {
        evictedL1 = L1.insertOnFill(address, markDirtyL1);
    }
    if (evictedL1.valid && evictedL1.dirty) {
        // Dirty eviction at L1 must be written to L2
        int evictedAddress = evictedL1.blockNumber * L1.getBlockSize();

        auto [h2,e2] = L2.access(evictedAddress,true);

        cycles += L2.getLatency();
        writesToLower++;
        if (!h2) {
                L2.insertOnFill(evictedAddress, true); 
        }
        if(e2){
            cycles += memoryLatency;
            writesToMemory++;
        }
    }

    totalCycles += cycles;
    return cycles;
}
void MultiLevelCache::prefetchNextLine(int address){
    if(!enablePrefetch) return;

    int nextAddress = address + L1.getBlockSize();

    L2.insertOnFill(nextAddress,false);
    L1.insertOnFill(nextAddress,false);
}

void MultiLevelCache::printStats() {
    std::cout << "Multi-Level Cache Simulation Results:\n";
    std::cout << "Total Accesses: " << totalAccesses << "\n";
    std::cout << "Average Access Latency (cycles): " << (totalAccesses ? (double)totalCycles / totalAccesses : 0.0) << "\n\n";
    long long l1Accesses = L1.getReadHits() + L1.getReadMisses() + L1.getWriteHits() + L1.getWriteMisses();

    double l1MissRate = l1Accesses ? (double)( L1.getReadMisses() + L1.getWriteMisses() ) / l1Accesses : 0.0;

    long long l2Accesses = L2.getReadHits() + L2.getReadMisses() + L2.getWriteHits() + L2.getWriteMisses();
    double l2MissRate = l2Accesses ? (double)(L2.getReadMisses() + L2.getWriteMisses()) / l2Accesses : 0.0;

    double amat = L1.getLatency() + l1MissRate * (L2.getLatency() + l2MissRate * memoryLatency);

    std::cout << "AMAT: " << amat << " cycles\n\n";
    auto printLevel = [&](const char* name, const CacheLevel& L) {
        long long rh = L.getReadHits();
        long long rm = L.getReadMisses();
        long long wh = L.getWriteHits();
        long long wm = L.getWriteMisses();
        long long wb = L.getWriteBacksToLower();

        long long reads = rh + rm;
        long long writes = wh + wm;
        long long accesses = reads + writes;
        long long misses = rm + wm;
        double missRate = accesses ? (double)misses / accesses : 0.0;

        std::cout << name << " reads: " << reads << "\n";
        std::cout << name << " read misses: " << rm << "\n";
        std::cout << name << " writes: " << writes << "\n";
        std::cout << name << " write misses: " << wm << "\n";
        std::cout << name << " miss rate: " << missRate << "\n";
        std::cout << name << " write-backs to lower: " << wb << "\n\n";
    };

    printLevel("L1", L1);
    printLevel("L2", L2);

    std::cout << "L1 hits (total demand): " << L1Hits << "\n";
    std::cout << "L2 hits (after L1 miss): " << L2Hits << "\n";
    std::cout << "Misses to memory: " << Misses << "\n";
    std::cout << "Writes forwarded to lower levels: " << writesToLower << "\n";
    std::cout << "Writes to memory: " << writesToMemory << "\n";
}

void MultiLevelCache::exportToCSV(const std::string& filename, const std::string& configName) {
    std::ofstream csv;
    bool fileExists = std::ifstream(filename).good();
    
    csv.open(filename, std::ios::app);
    if (!csv) {
        std::cerr << "Error opening CSV file: " << filename << "\n";
        return;
    }
    
    // Write header if file is new
    if (!fileExists) {
        csv << "Config,L1_Size,L1_Block,L1_Assoc,L1_Latency,L1_WritePolicy,L1_AllocPolicy,"
            << "L2_Size,L2_Block,L2_Assoc,L2_Latency,L2_WritePolicy,L2_AllocPolicy,"
            << "MemLatency,TotalAccesses,TotalCycles,AvgLatency,"
            << "L1_Reads,L1_ReadMisses,L1_Writes,L1_WriteMisses,L1_MissRate,L1_Writebacks,"
            << "L2_Reads,L2_ReadMisses,L2_Writes,L2_WriteMisses,L2_MissRate,L2_Writebacks,"
            << "MemoryMisses,WritesToLower,WritesToMemory\n";
    }
    
    auto wpToStr = [](WritePolicy wp) { return wp == WritePolicy::WriteBack ? "WB" : "WT"; };
    auto apToStr = [](AllocatePolicy ap) { return ap == AllocatePolicy::WriteAllocate ? "WA" : "NWA"; };
    
    // L1 stats
    long long l1Reads = L1.getReadHits() + L1.getReadMisses();
    long long l1Writes = L1.getWriteHits() + L1.getWriteMisses();
    double l1MissRate = (l1Reads + l1Writes) ? (double)(L1.getReadMisses() + L1.getWriteMisses()) / (l1Reads + l1Writes) : 0.0;
    
    // L2 stats
    long long l2Reads = L2.getReadHits() + L2.getReadMisses();
    long long l2Writes = L2.getWriteHits() + L2.getWriteMisses();
    double l2MissRate = (l2Reads + l2Writes) ? (double)(L2.getReadMisses() + L2.getWriteMisses()) / (l2Reads + l2Writes) : 0.0;
    
    csv << std::fixed << std::setprecision(4);
    csv << configName << ","
        << L1.getCacheSize() << "," << L1.getBlockSize() << "," << L1.getAssociativity() << "," << L1.getLatency() << ","
        << wpToStr(L1.getWritePolicy()) << "," << apToStr(L1.getAllocatePolicy()) << ","
        << L2.getCacheSize() << "," << L2.getBlockSize() << "," << L2.getAssociativity() << "," << L2.getLatency() << ","
        << wpToStr(L2.getWritePolicy()) << "," << apToStr(L2.getAllocatePolicy()) << ","
        << memoryLatency << "," << totalAccesses << "," << totalCycles << ","
        << (totalAccesses ? (double)totalCycles / totalAccesses : 0.0) << ","
        << l1Reads << "," << L1.getReadMisses() << "," << l1Writes << "," << L1.getWriteMisses() << ","
        << l1MissRate << "," << L1.getWriteBacksToLower() << ","
        << l2Reads << "," << L2.getReadMisses() << "," << l2Writes << "," << L2.getWriteMisses() << ","
        << l2MissRate << "," << L2.getWriteBacksToLower() << ","
        << Misses << "," << writesToLower << "," << writesToMemory << "\n";
    
    csv.close();
    std::cout << "Results exported to " << filename << "\n";
}
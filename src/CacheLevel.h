#ifndef CACHELEVEL_H
#define CACHELEVEL_H

#include <vector>
#include <list>
#include <unordered_map>

enum class WritePolicy { WriteBack, WriteThrough };
enum class AllocatePolicy { WriteAllocate, NoWriteAllocate };

struct LineMeta {
    std::list<int>::iterator it; 
    bool dirty = false;          // valid for WriteBack
};

struct EvictionInfo {
    bool valid = false;
    int blockNumber = -1;
    bool dirty = false;
};

class CacheLevel {
private:
    int cacheSize;
    int blockSize;
    int associativity;
    int numSets;
    int latency;

    WritePolicy writePolicy;
    AllocatePolicy allocatePolicy;

    std::vector<std::list<int>> sets; 
    std::vector<std::unordered_map<int, LineMeta>> tagMeta; 
    // stats
    long long readHits = 0, readMisses = 0;
    long long writeHits = 0, writeMisses = 0;
    long long writeBacksToLower = 0; // number of dirty evictions

public:
    CacheLevel(int cSize, int bSize, int assoc, int latencyCycles, WritePolicy wpol, AllocatePolicy apol);

    std::pair<bool,bool> access(int address, bool isWrite);

    EvictionInfo insertOnFill(int address, bool markDirty);

    // Helpers
    int getCacheSize() const { return cacheSize; }
    int getBlockSize() const { return blockSize; }
    int getAssociativity() const { return associativity; }
    int getLatency() const { return latency; }
    int getBlockNumber(int address) const { return address / blockSize; }
    int getNumSets() const { return numSets; }

    WritePolicy getWritePolicy() const { return writePolicy; }
    AllocatePolicy getAllocatePolicy() const { return allocatePolicy; }


    // Stats accessors
    long long getReadHits() const { return readHits; }
    long long getReadMisses() const { return readMisses; }
    long long getWriteHits() const { return writeHits; }
    long long getWriteMisses() const { return writeMisses; }
    long long getWriteBacksToLower() const { return writeBacksToLower; }
};

#endif

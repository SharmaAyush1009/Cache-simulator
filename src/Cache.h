#ifndef CACHE_H
#define CACHE_H

#include <vector>
#include <list>
#include <unordered_map>

class Cache {
private:
    int cacheSize;            // in bytes
    int blockSize;            // in bytes
    int associativity;        // ways per set (1=Direct Mapped)
    int numSets;              // calculated from cache size & associativity

    std::vector<std::list<int>> sets;  // Stores tags for each set for LRU
    std::vector<std::unordered_map<int, std::list<int>::iterator>> maps; // Tag lookup for each set

public:
    Cache(int cSize, int bSize, int assoc);
    bool access(int address); // Returns true if hit, false if miss
    void printStats();        // To print simulation statistics (later)
};

#endif

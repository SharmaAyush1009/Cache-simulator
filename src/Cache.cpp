#include "Cache.h"
#include <cmath>     
#include <iostream>

//constructor: Initialize cache parameters and data structures
Cache::Cache(int cSize, int bSize, int assoc)
    : cacheSize(cSize), blockSize(bSize), associativity(assoc) {

    int numBlocks = cacheSize / blockSize;

    numSets = numBlocks / associativity;

    sets.resize(numSets);
    maps.resize(numSets);
}

// simulates accessing a memory address; returns true on hit, false on miss
bool Cache::access(int address) {
    int blockNumber = address / blockSize;

    int setIndex = blockNumber % numSets;

    int tag = blockNumber / numSets;

    std::list<int> &setList = sets[setIndex];
    std::unordered_map<int, std::list<int>::iterator> &map = maps[setIndex];

    // Check for tag in cache (cache hit)
    if (map.find(tag) != map.end()) {
        // Move tag to front of LRU list
        setList.erase(map[tag]);
        setList.push_front(tag);
        map[tag] = setList.begin();
        return true;
    }

    // Cache miss: If set full, evict least recently used (back of list)
    if ((int)setList.size() == associativity) {
        int lruTag = setList.back();
        setList.pop_back();
        map.erase(lruTag);
    }

    // Insert new tag at front as most recently used
    setList.push_front(tag);
    map[tag] = setList.begin();

    return false;
}

// // Future: print stats function to summarize simulation (not implemented yet)
// void Cache::printStats() {
//     // Implement statistic summaries here later
// }

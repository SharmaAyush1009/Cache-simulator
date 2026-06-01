#include "CacheLevel.h"

CacheLevel::CacheLevel(int cSize, int bSize, int assoc, int latencyCycles, WritePolicy wpol, AllocatePolicy apol): cacheSize(cSize), blockSize(bSize), associativity(assoc), latency(latencyCycles),writePolicy(wpol), allocatePolicy(apol){
    int numBlocks = cacheSize / blockSize;
    numSets = numBlocks / associativity;
    sets.resize(numSets);
    tagMeta.resize(numSets);
}

std::pair<bool,bool> CacheLevel::access(int address, bool isWrite) {
    int blockNumber = address / blockSize;
    int setIndex = blockNumber % numSets;
    int tag = blockNumber / numSets;

    auto &listRef = sets[setIndex];
    auto &metaRef = tagMeta[setIndex];

    auto it = metaRef.find(tag);
    if (it != metaRef.end()) {
        // Hit: move to MRU
        listRef.erase(it->second.it);
        listRef.push_front(tag);
        it->second.it = listRef.begin();

        if (isWrite) {
            if (writePolicy == WritePolicy::WriteBack) {
                it->second.dirty = true; // mark dirty, defer lower write
            } else { // WriteThrough
                // immediate lower-level write will be accounted at controller level
            }
            writeHits++;
        } else {
            readHits++;
        }
        return {true, false};
    }

    // Miss path
    if (isWrite) writeMisses++; else readMisses++;

    // For writes, check allocation policy
    if (isWrite && allocatePolicy == AllocatePolicy::NoWriteAllocate) {
        // Do not bring into cache; controller will forward to lower level
        return {false, false};
    }

    // Bring block into cache (read miss, or write miss with write-allocate)
    // Evict if full
    bool evictedDirty = false;
    if ((int)listRef.size() == associativity) {
        int lruTag = listRef.back();
        listRef.pop_back();
        auto mIt = metaRef.find(lruTag);
        if (mIt != metaRef.end()) {
            if (mIt->second.dirty && writePolicy == WritePolicy::WriteBack) {
                evictedDirty = true;
                writeBacksToLower++; // count write-back to lower on eviction
            }
            metaRef.erase(mIt);
        }
    }

    // Insert new block as MRU
    listRef.push_front(tag);
    LineMeta lm;
    lm.it = listRef.begin();
    lm.dirty = (isWrite && writePolicy == WritePolicy::WriteBack); // write-allocate + WB
    metaRef[tag] = lm;

    return {false, evictedDirty};
}

EvictionInfo CacheLevel::insertOnFill(int address, bool markDirty)
{
    int blockNumber = address / blockSize;
    int setIndex = blockNumber % numSets;
    int tag = blockNumber / numSets;

    auto &listRef = sets[setIndex];
    auto &metaRef = tagMeta[setIndex];

    EvictionInfo ev;

    if ((int)listRef.size() == associativity)
    {
        int lruTag = listRef.back();
        listRef.pop_back();

        auto mIt = metaRef.find(lruTag);

        if (mIt != metaRef.end())
        {
            ev.valid = true;

            ev.blockNumber =
                lruTag * numSets + setIndex;

            ev.dirty = mIt->second.dirty;

            if (ev.dirty &&
                writePolicy == WritePolicy::WriteBack)
            {
                writeBacksToLower++;
            }

            metaRef.erase(mIt);
        }
    }

    listRef.push_front(tag);

    LineMeta lm;
    lm.it = listRef.begin();
    lm.dirty =
        markDirty &&
        (writePolicy == WritePolicy::WriteBack);

    metaRef[tag] = lm;

    return ev;
}

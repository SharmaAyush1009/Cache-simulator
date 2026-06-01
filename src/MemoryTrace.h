#ifndef MEMORYTRACE_H
#define MEMORYTRACE_H

#include <string>
#include <vector>
#include <utility>

namespace MemoryTrace {

// Synthetic patterns
std::vector<int> generateSequential(int maxAddress, int count);
std::vector<int> generateRandom(int maxAddress, int count);
std::vector<int> generateStrided(int maxAddress, int count, int stride);

// Hex-only traces: one hex address per line (with or without 0x)
std::vector<int> readTraceFile(const std::string& filename);

// R/W traces: lines like "R 0x1a2b3c" or "W DEADBEEF"
std::vector<std::pair<int,bool>> readTraceFileRW(const std::string& filename);

} // namespace MemoryTrace
#endif

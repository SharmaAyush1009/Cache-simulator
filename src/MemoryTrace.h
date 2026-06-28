#ifndef MEMORYTRACE_H
#define MEMORYTRACE_H

#include <string>
#include <vector>
#include <utility>

namespace MemoryTrace {
    // R/W traces: lines like "R 0x1a2b3c" or "W DEADBEEF"
    std::vector<std::pair<int,bool>> readTraceFileRW(const std::string& filename);

} // namespace MemoryTrace
#endif

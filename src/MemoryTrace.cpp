#include "MemoryTrace.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cctype>

namespace MemoryTrace {

std::vector<int> generateSequential(int maxAddress, int count) {
    std::vector<int> addresses;
    addresses.reserve(count);
    for (int i = 0; i < count; i++) {
        addresses.push_back(i % maxAddress);
    }
    return addresses;
}

std::vector<int> generateRandom(int maxAddress, int count) {
    std::vector<int> addresses;
    addresses.reserve(count);
    std::srand((unsigned)std::time(nullptr));
    for (int i = 0; i < count; i++) {
        addresses.push_back(std::rand() % maxAddress);
    }
    return addresses;
}

std::vector<int> generateStrided(int maxAddress, int count, int stride) {
    std::vector<int> addresses;
    addresses.reserve(count);
    int addr = 0;
    for (int i = 0; i < count; i++) {
        addresses.push_back(addr % maxAddress);
        addr += stride;
    }
    return addresses;
}

std::vector<int> readTraceFile(const std::string& filename) {
    std::vector<int> addresses;
    std::ifstream infile(filename);
    if (!infile) {
        std::cerr << "Error opening file: " << filename << "\n";
        return addresses;
    }
    std::string line;
    while (std::getline(infile, line)) {
        std::stringstream ss(line);
        unsigned int val;
        ss >> std::hex >> val;
        if (!ss.fail()) {
            addresses.push_back(static_cast<int>(val));
        }
    }
    return addresses;
}

static bool parseRWLine(const std::string& line, int& outAddr, bool& outIsWrite) {
    // Trim leading whitespace
    std::string s = line;
    size_t p = s.find_first_not_of(" \t\r\n");
    if (p == std::string::npos) return false;            // empty or whitespace-only line
    s = s.substr(p);
    if (s.empty()) return false;                         // nothing left after trim

    size_t hashPos = s.find('#');
    if (hashPos != std::string::npos) {
        s = s.substr(0, hashPos);
        while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
        if (s.empty()) return false;                     // line was only comment
    }

    // Skip comment lines that start with '#'
    if (s[0] == '#') return false;                       
    
    // check first non-space char is the op: R/r/W/w or l/s (load/store)
    char op = s[0];                                      
    if (op!='R' && op!='r' && op!='W' && op!='w' && op!='l' && op!='s') return false; 
    outIsWrite = (op=='W' || op=='w' || op=='s');        // W/w/s treated as writes; R/r/l as reads

    // Parse address token following the op
    std::istringstream iss(s.substr(1));
    std::string addrToken;
    if (!(iss >> addrToken)) return false;               // missing address token

    // Strip trailing commas/spaces from token
    while (!addrToken.empty() &&
           (addrToken.back()==',' || std::isspace(static_cast<unsigned char>(addrToken.back())))) {
        addrToken.pop_back();
    }
    if (addrToken.empty()) return false;                 // empty after stripping

    // Convert hex with or without 0x prefix
    unsigned int addr = 0;
    std::stringstream conv;
    conv << std::hex << addrToken;
    conv >> addr;
    if (conv.fail()) return false;                       // invalid hex token

    outAddr = static_cast<int>(addr);
    return true;
}



std::vector<std::pair<int,bool>> readTraceFileRW(const std::string& filename) {
    std::vector<std::pair<int,bool>> ops;
    std::ifstream infile(filename);
    if (!infile) {
        std::cerr << "Error opening file: " << filename << "\n";
        return ops;
    }
    std::string line;
    int addr = 0;
    bool isWrite = false;
    while (std::getline(infile, line)) {
        if (parseRWLine(line, addr, isWrite)) {
            ops.emplace_back(addr, isWrite);
        } else {
            // Fallback: hex-only line treated as read
            std::stringstream ss(line);
            unsigned int val;
            ss >> std::hex >> val;
            if (!ss.fail()) {
                ops.emplace_back(static_cast<int>(val), false);
            }
        }
    }
    return ops;
}

} 

cmake_minimum_required(VERSION 3.10)
project(CacheSimulator)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add source files
set(SOURCES
    src/main.cpp
    # src/Cache.cpp
    src/MemoryTrace.cpp
    src/MultilevelCache.cpp
    src/CacheLevel.cpp
)

# Define executable
add_executable(cache_simulator ${SOURCES})

# Include directories for header files
target_include_directories(cache_simulator PRIVATE src)

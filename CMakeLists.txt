cmake_minimum_required(VERSION 3.15)
project(DistributedFileSystem)

set(CMAKE_CXX_STANDARD 17)

# Tell CMake where to find headers
include_directories(include)

# List your source files
set(SOURCES
    CommandParsing/CommandParser.cpp
    # add other .cpp files here as needed
)

add_executable(dfs ${SOURCES})


cmake_minimum_required(VERSION 3.15)
project(DistributedFileSystem)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add your include directory
include_directories(include)

# List all your source files here, adjust as needed
set(SOURCES
    MetadataNode.cpp
    Networking/SocketHandler.cpp
    Logger.cpp
    CommandParsing/CommandParser.cpp
    # Add other .cpp files here...
)

add_executable(dfs ${SOURCES})

# Link pthread automatically if on Unix-like systems
if(UNIX)
    find_package(Threads REQUIRED)
    target_link_libraries(dfs PRIVATE Threads::Threads)
endif()

# On Windows, link ws2_32 for Winsock (asio)
if(WIN32)
    target_link_libraries(dfs PRIVATE ws2_32)
endif()

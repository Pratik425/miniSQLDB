cmake_minimum_required(VERSION 3.10)
project(MiniDB)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add compile options for better debugging
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)

# Collect all source files
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "src/models/*.cpp"
)

# Create executable
add_executable(minidb ${SOURCES})

# Set output directory
set_target_properties(minidb PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Add test executable (optional)
enable_testing()
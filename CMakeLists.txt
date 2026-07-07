cmake_minimum_required(VERSION 3.10)
project(MiniDB)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add compile options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)

# Collect all source files
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
)

# Main executable
add_executable(minidb ${SOURCES})

set_target_properties(minidb PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Tests (simple version - no Google Test)
add_executable(minidb_tests 
    tests/test_runner.cpp
    src/utils/FileManager.cpp
    src/models/Column.cpp
    src/models/Record.cpp
    src/models/Table.cpp
    src/storage/StorageManager.cpp
)

target_include_directories(minidb_tests PRIVATE src)

# Make both executables
add_custom_target(build-all DEPENDS minidb minidb_tests)
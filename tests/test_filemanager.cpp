#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include "utils/FileManager.h"

#define TEST(name) void test_##name()
#define RUN_TEST(name) \
    do { \
        std::cout << "Running test: " #name " ... "; \
        test_##name(); \
        std::cout << "PASSED" << std::endl; \
    } while(0)

// Test: File creation and existence
TEST(file_creation) {
    FileManager fm("test_db");
    std::string filename = "test.dat";
    
    // Should not exist initially
    assert(!fm.fileExists(filename));
    
    // Create file
    assert(fm.createFile(filename));
    assert(fm.fileExists(filename));
    
    // Create again should fail
    assert(!fm.createFile(filename));
    
    // Cleanup
    fm.deleteFile(filename);
}

// Test: Write and read
TEST(write_read) {
    FileManager fm("test_db");
    std::string filename = "data.dat";
    std::vector<char> writeData = {'H', 'e', 'l', 'l', 'o'};
    
    fm.writeFile(filename, writeData);
    auto readData = fm.readFile(filename);
    
    assert(writeData.size() == readData.size());
    assert(std::equal(writeData.begin(), writeData.end(), readData.begin()));
    
    fm.deleteFile(filename);
}

// Test: Append
TEST(append) {
    FileManager fm("test_db");
    std::string filename = "append.dat";
    std::vector<char> first = {'F', 'i', 'r', 's', 't'};
    std::vector<char> second = {'S', 'e', 'c', 'o', 'n', 'd'};
    
    fm.writeFile(filename, first);
    fm.appendToFile(filename, second);
    
    auto readData = fm.readFile(filename);
    std::vector<char> expected;
    expected.insert(expected.end(), first.begin(), first.end());
    expected.insert(expected.end(), second.begin(), second.end());
    
    assert(readData.size() == expected.size());
    assert(std::equal(expected.begin(), expected.end(), readData.begin()));
    
    fm.deleteFile(filename);
}

// Test: Delete
TEST(delete_file) {
    FileManager fm("test_db");
    std::string filename = "delete.dat";
    
    fm.createFile(filename);
    assert(fm.fileExists(filename));
    
    fm.deleteFile(filename);
    assert(!fm.fileExists(filename));
}

// Test: List files
TEST(list_files) {
    FileManager fm("test_db");
    fm.createFile("file1.txt");
    fm.createFile("file2.txt");
    fm.createFile("file3.dat");
    
    auto files = fm.listFiles();
    assert(files.size() == 3);
    
    // Cleanup
    for (const auto& f : files) {
        fm.deleteFile(f);
    }
}

int main() {
    std::cout << "=== Running MiniDB Tests ===" << std::endl;
    
    RUN_TEST(file_creation);
    RUN_TEST(write_read);
    RUN_TEST(append);
    RUN_TEST(delete_file);
    RUN_TEST(list_files);
    
    std::cout << "\n✅ All tests passed!" << std::endl;
    
    // Cleanup test directory
    std::filesystem::remove_all("test_db");
    
    return 0;
}
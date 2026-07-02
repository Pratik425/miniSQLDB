#include "utils/FileManager.h"
#include <stdexcept>
#include <algorithm>
#include <fstream>

// Constructor
FileManager::FileManager(const std::string& path) 
    : databasePath(path) {
    ensureDirectoryExists();
}

// Ensure directory exists
void FileManager::ensureDirectoryExists() {
    try {
        if (!fs::exists(databasePath)) {
            fs::create_directories(databasePath);
        }
    } catch (const fs::filesystem_error& e) {
        throw std::runtime_error("Failed to create database directory: " + 
                                 std::string(e.what()));
    }
}

// Get full path
std::string FileManager::getFullPath(const std::string& filename) const {
    // Sanitize filename to prevent directory traversal attacks
    if (filename.find("..") != std::string::npos) {
        throw std::runtime_error("Invalid filename: " + filename);
    }
    return databasePath + "/" + filename;
}

// Check if file exists
bool FileManager::fileExists(const std::string& filename) const {
    return fs::exists(getFullPath(filename));
}

// Create file
bool FileManager::createFile(const std::string& filename) {
    if (fileExists(filename)) {
        return false;  // File already exists
    }
    
    std::string fullPath = getFullPath(filename);
    std::ofstream file(fullPath, std::ios::binary);
    
    if (!file) {
        throw std::runtime_error("Failed to create file: " + filename);
    }
    
    return true;
}

// Delete file
bool FileManager::deleteFile(const std::string& filename) {
    if (!fileExists(filename)) {
        return false;
    }
    
    std::string fullPath = getFullPath(filename);
    return fs::remove(fullPath);
}

// Read file
std::vector<char> FileManager::readFile(const std::string& filename) const {
    std::string fullPath = getFullPath(filename);
    
    if (!fileExists(filename)) {
        throw std::runtime_error("File does not exist: " + filename);
    }
    
    // Open file in binary mode and seek to end to get size
    std::ifstream file(fullPath, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    
    // Get file size
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    // Read entire file
    std::vector<char> buffer(fileSize);
    if (fileSize > 0) {
        if (!file.read(buffer.data(), fileSize)) {
            throw std::runtime_error("Failed to read file: " + filename);
        }
    }
    
    return buffer;
}

// Write file (overwrites)
bool FileManager::writeFile(const std::string& filename, 
                            const std::vector<char>& data) {
    std::string fullPath = getFullPath(filename);
    
    // Ensure directory exists (in case it was deleted)
    ensureDirectoryExists();
    
    std::ofstream file(fullPath, std::ios::binary | std::ios::trunc);
    if (!file) {
        throw std::runtime_error("Cannot write to file: " + filename);
    }
    
    if (!data.empty()) {
        file.write(data.data(), data.size());
        return file.good();
    }
    
    return true;  // Empty file created
}

// Append to file
bool FileManager::appendToFile(const std::string& filename, 
                               const std::vector<char>& data) {
    if (!fileExists(filename)) {
        return createFile(filename);
    }
    
    std::string fullPath = getFullPath(filename);
    std::ofstream file(fullPath, std::ios::binary | std::ios::app);
    
    if (!file) {
        throw std::runtime_error("Cannot append to file: " + filename);
    }
    
    if (!data.empty()) {
        file.write(data.data(), data.size());
        return file.good();
    }
    
    return true;
}

// List files
std::vector<std::string> FileManager::listFiles() const {
    std::vector<std::string> files;
    
    try {
        for (const auto& entry : fs::directory_iterator(databasePath)) {
            if (fs::is_regular_file(entry)) {
                files.push_back(entry.path().filename().string());
            }
        }
    } catch (const fs::filesystem_error& e) {
        throw std::runtime_error("Failed to list files: " + 
                                 std::string(e.what()));
    }
    
    std::sort(files.begin(), files.end());
    return files;
}

// Get file size
size_t FileManager::getFileSize(const std::string& filename) const {
    if (!fileExists(filename)) {
        throw std::runtime_error("File does not exist: " + filename);
    }
    
    std::string fullPath = getFullPath(filename);
    return fs::file_size(fullPath);
}
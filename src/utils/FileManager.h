#pragma once
#include <string>
#include <fstream>
#include <vector>
#include <filesystem>
#include <memory>

namespace fs = std::filesystem;

/**
 * @brief Manages all file operations for the database
 * 
 * FileManager provides a unified interface for all file I/O operations.
 * It handles:
 * - File creation and deletion
 * - Reading and writing binary data
 * - Directory management
 * - File existence checking
 * 
 * All paths are relative to the database directory.
 */
class FileManager {
private:
    std::string databasePath;  // Root directory for database files
    
public:
    /**
     * @brief Construct a new FileManager
     * 
     * @param path Path to database directory
     */
    explicit FileManager(const std::string& path);
    
    /**
     * @brief Create a new file
     * 
     * @param filename Name of file to create
     * @return true if created successfully, false if file exists
     */
    bool createFile(const std::string& filename);
    
    /**
     * @brief Delete a file
     * 
     * @param filename Name of file to delete
     * @return true if deleted successfully
     */
    bool deleteFile(const std::string& filename);
    
    /**
     * @brief Check if a file exists
     * 
     * @param filename Name of file to check
     * @return true if file exists
     */
    bool fileExists(const std::string& filename) const;
    
    /**
     * @brief Read entire file into memory
     * 
     * @param filename Name of file to read
     * @return std::vector<char> File contents
     * @throws std::runtime_error if file cannot be read
     */
    std::vector<char> readFile(const std::string& filename) const;
    
    /**
     * @brief Write data to file (overwrites existing)
     * 
     * @param filename Name of file to write
     * @param data Data to write
     * @return true if write successful
     */
    bool writeFile(const std::string& filename, const std::vector<char>& data);
    
    /**
     * @brief Append data to existing file
     * 
     * @param filename Name of file to append to
     * @param data Data to append
     * @return true if append successful
     */
    bool appendToFile(const std::string& filename, const std::vector<char>& data);
    
    /**
     * @brief Get full path for a file
     * 
     * @param filename Name of file
     * @return std::string Full path
     */
    std::string getFullPath(const std::string& filename) const;
    
    /**
     * @brief List all files in database directory
     * 
     * @return std::vector<std::string> List of filenames
     */
    std::vector<std::string> listFiles() const;
    
    /**
     * @brief Get size of a file
     * 
     * @param filename Name of file
     * @return size_t File size in bytes
     */
    size_t getFileSize(const std::string& filename) const;
    
private:
    /**
     * @brief Ensure database directory exists
     */
    void ensureDirectoryExists();
};
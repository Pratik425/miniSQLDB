#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include "models/Table.h"
#include "utils/FileManager.h"

/**
 * @brief Manages all tables in the database
 * 
 * StorageManager is the central component that:
 * - Maintains a collection of tables
 * - Handles database initialization and shutdown
 * - Provides table creation, deletion, and lookup
 * - Manages table metadata and persistence
 * 
 * Each database instance has one StorageManager that
 * manages all tables within that database.
 */
class StorageManager {
private:
    std::string databasePath;                              // Database directory
    std::unique_ptr<FileManager> fileManager;              // File operations
    std::unordered_map<std::string, std::unique_ptr<Table>> tables;  // Table cache
    
public:
    /**
     * @brief Construct a new StorageManager
     * 
     * @param dbPath Path to database directory
     */
    explicit StorageManager(const std::string& dbPath);
    
    /**
     * @brief Destroy the StorageManager and save all tables
     */
    ~StorageManager() = default;
    
    // ========== Table Management ==========
    
    /**
     * @brief Create a new table
     * 
     * @param name Table name
     * @param schema Column definitions
     * @throws std::runtime_error if table already exists
     */
    void createTable(const std::string& name, const std::vector<Column>& schema);
    
    /**
     * @brief Drop an existing table
     * 
     * @param name Table name
     * @throws std::runtime_error if table doesn't exist
     */
    void dropTable(const std::string& name);
    
    /**
     * @brief Get a table by name
     * 
     * @param name Table name
     * @return Table* Pointer to table (nullptr if not found)
     */
    Table* getTable(const std::string& name);
    
    /**
     * @brief Get a table by name (const version)
     * 
     * @param name Table name
     * @return const Table* Pointer to table (nullptr if not found)
     */
    const Table* getTable(const std::string& name) const;
    
    /**
     * @brief Check if a table exists
     * 
     * @param name Table name
     * @return true if table exists
     */
    bool tableExists(const std::string& name) const;
    
    /**
     * @brief List all table names
     * 
     * @return std::vector<std::string> List of table names
     */
    std::vector<std::string> listTables() const;
    
    /**
     * @brief Get number of tables
     */
    size_t getTableCount() const { return tables.size(); }
    
    // ========== Database Operations ==========
    
    /**
     * @brief Load all tables from disk
     * 
     * Scans the database directory and loads all tables
     */
    void loadDatabase();
    
    /**
     * @brief Save all tables to disk
     */
    void saveDatabase();
    
    /**
     * @brief Check if database exists
     */
    bool databaseExists() const;
    
    /**
     * @brief Get database path
     */
    const std::string& getDatabasePath() const { return databasePath; }
    
    // ========== Utility Methods ==========
    
    /**
     * @brief Print all tables and their records (for debugging)
     */
    void printDatabase() const;
    
    /**
     * @brief Clear all tables (in-memory only)
     */
    void clearAllTables();
    
private:
    /**
     * @brief Load a single table from disk
     * 
     * @param tableName Name of table to load
     */
    void loadTable(const std::string& tableName);
    
    /**
     * @brief Save a single table to disk
     * 
     * @param tableName Name of table to save
     */
    void saveTable(const std::string& tableName);
    
    /**
     * @brief Parse table schema from metadata file
     * 
     * @param tableName Name of table
     * @return std::vector<Column> Schema definition
     */
    std::vector<Column> loadTableSchema(const std::string& tableName) const;
    
    /**
     * @brief Get table data file name
     */
    std::string getDataFileName(const std::string& tableName) const;
    
    /**
     * @brief Get table metadata file name
     */
    std::string getMetaFileName(const std::string& tableName) const;
};
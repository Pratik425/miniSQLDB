#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>
#include "models/Column.h"
#include "models/Record.h"
#include "utils/FileManager.h"

/**
 * @brief Represents a database table with schema and data
 * 
 * Table manages:
 * - Schema definition (columns with types and constraints)
 * - Record storage and retrieval
 * - Persistence to disk using FileManager
 * - Basic CRUD operations
 * 
 * Each table is stored as:
 * - {table_name}.db : Binary file containing all records
 * - {table_name}_meta.txt : Metadata with schema definition
 */
class Table {
private:
    std::string name;                      // Table name
    std::vector<Column> schema;            // Column definitions
    std::vector<Record> records;           // In-memory records
    std::unordered_map<std::string, size_t> columnIndexMap; // Name -> Index mapping
    std::unique_ptr<FileManager> fileManager; // File operations
    std::string dataFile;                  // Data file name
    std::string metaFile;                  // Metadata file name
    
    // Statistics
    size_t recordCount;
    size_t pageSize;                       // For future page-based storage
    
public:
    /**
     * @brief Construct a new Table
     * 
     * @param name Table name
     * @param schema Column definitions
     * @param databasePath Path to database directory
     */
    Table(const std::string& name, 
          const std::vector<Column>& schema, 
          const std::string& databasePath);
    
    /**
     * @brief Destroy the Table and save if needed
     */
    ~Table() = default;
    
    // ========== DDL Operations ==========
    
    /**
     * @brief Create the table on disk
     * 
     * Creates data file and metadata file
     * @throws std::runtime_error if table already exists
     */
    void createTable();
    
    /**
     * @brief Drop the table (delete all files)
     */
    void dropTable();
    
    /**
     * @brief Load table data from disk
     */
    void loadFromDisk();
    
    /**
     * @brief Save table data to disk
     */
    void saveToDisk();
    
    // ========== DML Operations ==========
    
    /**
     * @brief Insert a new record
     * 
     * @param record Record to insert
     * @throws std::runtime_error if record doesn't match schema
     */
    void insertRecord(const Record& record);
    
    /**
     * @brief Insert multiple records
     * 
     * @param records Vector of records to insert
     */
    void insertRecords(const std::vector<Record>& records);
    
    /**
     * @brief Select all records
     * 
     * @return std::vector<Record> All records in table
     */
    std::vector<Record> selectAll() const;
    
    /**
     * @brief Select records matching a predicate
     * 
     * @param predicate Function that returns true for matching records
     * @return std::vector<Record> Matching records
     */
    std::vector<Record> selectWhere(
        const std::function<bool(const Record&)>& predicate) const;
    
    /**
     * @brief Update records matching a predicate
     * 
     * @param predicate Function that returns true for records to update
     * @param updater Function that modifies the record
     * @return size_t Number of records updated
     */
    size_t updateWhere(
        const std::function<bool(const Record&)>& predicate,
        const std::function<void(Record&)>& updater);
    
    /**
     * @brief Delete records matching a predicate
     * 
     * @param predicate Function that returns true for records to delete
     * @return size_t Number of records deleted
     */
    size_t deleteWhere(const std::function<bool(const Record&)>& predicate);
    
    /**
     * @brief Get a record by index
     * 
     * @param index Record index
     * @return const Record& Reference to record
     * @throws std::out_of_range if index invalid
     */
    const Record& getRecord(size_t index) const;
    
    // ========== Utility Methods ==========
    
    /**
     * @brief Get column index by name
     * 
     * @param columnName Name of column
     * @return size_t Column index
     * @throws std::runtime_error if column not found
     */
    size_t getColumnIndex(const std::string& columnName) const;
    
    /**
     * @brief Check if column exists
     * 
     * @param columnName Name of column
     * @return true if column exists
     */
    bool hasColumn(const std::string& columnName) const;
    
    /**
     * @brief Get table name
     */
    const std::string& getName() const { return name; }
    
    /**
     * @brief Get table schema
     */
    const std::vector<Column>& getSchema() const { return schema; }
    
    /**
     * @brief Get number of records
     */
    size_t getRecordCount() const { return records.size(); }
    
    /**
     * @brief Check if table is empty
     */
    bool isEmpty() const { return records.empty(); }
    
    /**
     * @brief Clear all records (in-memory only, doesn't affect disk)
     */
    void clearRecords();
    
    /**
     * @brief Print table to console (for debugging)
     */
    void printTable() const;
    
private:
    // ========== Serialization Helpers ==========
    
    /**
     * @brief Serialize a record to binary format
     */
    std::vector<char> serializeRecord(const Record& record) const;
    
    /**
     * @brief Deserialize a record from binary format
     */
    Record deserializeRecord(const std::vector<char>& data) const;
    
    /**
     * @brief Save schema to metadata file
     */
    void saveSchema();
    
    /**
     * @brief Load schema from metadata file
     */
    void loadSchema();
    
    /**
     * @brief Validate a record against schema
     */
    void validateRecord(const Record& record) const;
};
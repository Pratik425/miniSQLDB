#pragma once
#include <string>
#include <vector>
#include <unordered_map>

/**
 * @brief Enumeration of supported SQL data types
 * 
 * This enum defines all data types that MiniDB supports.
 * Each type maps to a specific C++ type for storage.
 */
enum class DataType {
    INT,        // Maps to int
    VARCHAR,    // Maps to std::string with max length
    BOOLEAN,    // Maps to bool
    FLOAT,      // Maps to float
    DATE        // Maps to time_t
};

/**
 * @brief Represents a column definition in a table
 * 
 * The Column class encapsulates all metadata about a database column,
 * including its name, data type, constraints, and storage requirements.
 */
class Column {
public:
    // Public members for simplicity (can be made private with getters)
    std::string name;
    DataType type;
    size_t maxLength;      // Only used for VARCHAR type
    bool isPrimaryKey;
    bool isNullable;
    
    /**
     * @brief Construct a new Column object
     * 
     * @param name Column name
     * @param type Data type
     * @param maxLength Maximum length (for VARCHAR)
     * @param isPrimaryKey Whether this is a primary key
     * @param isNullable Whether NULL values are allowed
     */
    Column(const std::string& name, 
           DataType type, 
           size_t maxLength = 255, 
           bool isPrimaryKey = false, 
           bool isNullable = true);
    
    /**
     * @brief Convert column definition to SQL-like string
     * 
     * Example: "name VARCHAR(50) NOT NULL"
     */
    std::string toString() const;
    
    /**
     * @brief Convert string to DataType enum
     * 
     * @param str Type name (e.g., "INT", "VARCHAR(50)")
     * @return DataType enum value
     * @throws std::runtime_error if type not recognized
     */
    static DataType fromString(const std::string& str);
    
    /**
     * @brief Get the storage size required for this column
     * 
     * For fixed-size types (INT, BOOLEAN, FLOAT), returns sizeof(type)
     * For VARCHAR, returns maxLength
     * 
     * @return size_t Number of bytes needed
     */
    size_t getStorageSize() const;
};
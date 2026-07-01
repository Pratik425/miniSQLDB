#pragma once
#include <vector>
#include <string>
#include <variant>
#include <memory>
#include <ctime>
#include "models/Column.h"

/**
 * @brief Represents a single row/record in a table
 * 
 * A Record contains values for each column defined in the table schema.
 * Values are stored in a type-safe manner using std::variant.
 */
class Record {
public:
    /**
     * @brief Type-safe value container supporting all SQL data types
     * 
     * Uses std::variant to store different types safely
     */
    using Value = std::variant<int, std::string, bool, float, time_t>;
    
private:
    std::vector<Value> values;          // Actual data values
    std::vector<Column> schema;         // Column definitions
    
public:
    /**
     * @brief Construct a new Record object
     * 
     * @param schema Column definitions for this record
     */
    explicit Record(const std::vector<Column>& schema);
    
    /**
     * @brief Set a value at a specific column index
     * 
     * @param index Column index (0-based)
     * @param value Value to set
     * @throws std::out_of_range if index is invalid
     */
    void setValue(size_t index, const Value& value);
    
    /**
     * @brief Get a value from a specific column index
     * 
     * @param index Column index (0-based)
     * @return Value at the specified column
     * @throws std::out_of_range if index is invalid
     */
    Value getValue(size_t index) const;
    
    /**
     * @brief Get value as a specific type (with type checking)
     * 
     * @tparam T Desired type
     * @param index Column index
     * @return T The value converted to type T
     * @throws std::bad_variant_access if type mismatch
     */
    template<typename T>
    T getValueAs(size_t index) const {
        return std::get<T>(getValue(index));
    }
    
    /**
     * @brief Convert record to human-readable string
     * 
     * @return std::string Formatted record representation
     */
    std::string toString() const;
    
    /**
     * @brief Get number of columns in this record
     */
    size_t getColumnCount() const { return values.size(); }
    
    /**
     * @brief Serialize record to binary format for storage
     * 
     * @return std::vector<char> Binary serialized data
     */
    std::vector<char> serialize() const;
    
    /**
     * @brief Deserialize from binary format
     * 
     * @param data Binary data to deserialize
     * @throws std::runtime_error if data is corrupt
     */
    void deserialize(const std::vector<char>& data);
    
    /**
     * @brief Get the schema for this record
     */
    const std::vector<Column>& getSchema() const { return schema; }
    
private:
    /**
     * @brief Serialize a single value to binary
     */
    std::vector<char> serializeValue(const Value& value, const Column& column) const;
    
    /**
     * @brief Deserialize a single value from binary
     */
    Value deserializeValue(const std::vector<char>& data, 
                           size_t& offset, 
                           const Column& column) const;
};
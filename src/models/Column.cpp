#include "models/Column.h"
#include <stdexcept>
#include <algorithm>
#include <cctype>

// Constructor implementation
Column::Column(const std::string& name, 
               DataType type, 
               size_t maxLength, 
               bool isPrimaryKey, 
               bool isNullable)
    : name(name)
    , type(type)
    , maxLength(maxLength)
    , isPrimaryKey(isPrimaryKey)
    , isNullable(isNullable) {
    
    // Validate VARCHAR max length
    if (type == DataType::VARCHAR && maxLength == 0) {
        throw std::runtime_error("VARCHAR must have a positive max length");
    }
}

// Convert column to SQL string
std::string Column::toString() const {
    std::string result = name + " ";
    
    // Add type
    switch(type) {
        case DataType::INT: 
            result += "INT"; 
            break;
        case DataType::VARCHAR: 
            result += "VARCHAR(" + std::to_string(maxLength) + ")"; 
            break;
        case DataType::BOOLEAN: 
            result += "BOOLEAN"; 
            break;
        case DataType::FLOAT: 
            result += "FLOAT"; 
            break;
        case DataType::DATE: 
            result += "DATE"; 
            break;
    }
    
    // Add constraints
    if(isPrimaryKey) {
        result += " PRIMARY KEY";
    }
    if(!isNullable) {
        result += " NOT NULL";
    }
    
    return result;
}

// Convert string to DataType
DataType Column::fromString(const std::string& str) {
    std::string upper = str;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
    
    // Handle VARCHAR specially (remove length part)
    if(upper.find("VARCHAR") != std::string::npos) {
        return DataType::VARCHAR;
    }
    
    // Check other types
    if(upper == "INT") return DataType::INT;
    if(upper == "BOOLEAN") return DataType::BOOLEAN;
    if(upper == "FLOAT") return DataType::FLOAT;
    if(upper == "DATE") return DataType::DATE;
    
    throw std::runtime_error("Unknown data type: " + str);
}

// Get storage size
size_t Column::getStorageSize() const {
    switch(type) {
        case DataType::INT: 
            return sizeof(int);
        case DataType::BOOLEAN: 
            return sizeof(bool);
        case DataType::FLOAT: 
            return sizeof(float);
        case DataType::DATE: 
            return sizeof(time_t);
        case DataType::VARCHAR: 
            return maxLength;
        default: 
            return 0;
    }
}
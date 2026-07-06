#include "models/Record.h"
#include <sstream>
#include <cstring>
#include <stdexcept>
#include <iomanip>


// Constructor
Record::Record(const std::vector<Column>& schema) 
    : schema(schema) {
    // Initialize values with empty variants
    values.resize(schema.size());
}

// Set value with validation
void Record::setValue(size_t index, const Value& value) {
    if(index >= values.size()) {
        throw std::out_of_range("Column index out of range");
    }
    
    // Type checking 
    const auto& column = schema[index];
    bool typeValid = false;
    
    std::visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int>) {
            typeValid = (column.type == DataType::INT);
        } else if constexpr (std::is_same_v<T, std::string>) {
            typeValid = (column.type == DataType::VARCHAR);
            // Check length constraint
            if(typeValid && arg.length() > column.maxLength) {
                throw std::runtime_error("String exceeds max length: " + 
                                        std::to_string(column.maxLength));
            }
        } else if constexpr (std::is_same_v<T, bool>) {
            typeValid = (column.type == DataType::BOOLEAN);
        } else if constexpr (std::is_same_v<T, float>) {
            typeValid = (column.type == DataType::FLOAT);
        } else if constexpr (std::is_same_v<T, time_t>) {
            typeValid = (column.type == DataType::DATE);
        }
    }, value);
    
    if(!typeValid) {
        throw std::runtime_error("Type mismatch for column: " + column.name);
    }
    
    values[index] = value;
}

// Get value
Record::Value Record::getValue(size_t index) const {
    if(index >= values.size()) {
        throw std::out_of_range("Column index out of range");
    }
    return values[index];
}

// Convert to string
std::string Record::toString() const {
    std::stringstream ss;
    ss << "(";
    for(size_t i = 0; i < values.size(); ++i) {
        if(i > 0) ss << ", ";
        
        std::visit([&ss](auto&& arg) {
            ss << arg;
        }, values[i]);
    }
    ss << ")";
    return ss.str();
}

// Serialization
std::vector<char> Record::serialize() const {
    std::vector<char> data;
    
    // Calculate total size
    size_t totalSize = 0;
    for(size_t i = 0; i < schema.size(); ++i) {
        totalSize += schema[i].getStorageSize();
    }
    
    data.reserve(totalSize);
    
    // Serialize each value
    for(size_t i = 0; i < values.size(); ++i) {
        auto serialized = serializeValue(values[i], schema[i]);
        data.insert(data.end(), serialized.begin(), serialized.end());
    }
    
    return data;
}

// Deserialize entire record
void Record::deserialize(const std::vector<char>& data) {
    size_t offset = 0;
    
    for(size_t i = 0; i < schema.size(); ++i) {
        Value val = deserializeValue(data, offset, schema[i]);
        values[i] = val;
    }
}

// Helper: Serialize single value
std::vector<char> Record::serializeValue(const Value& value, const Column& column) const {
    std::vector<char> data(column.getStorageSize(), 0);
    
    std::visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        
        if constexpr (std::is_same_v<T, int>) {
            memcpy(data.data(), &arg, sizeof(int));
        } else if constexpr (std::is_same_v<T, std::string>) {
            // Fixed-width string storage
            size_t copySize = std::min(arg.length(), column.maxLength);
            memcpy(data.data(), arg.c_str(), copySize);
            // Rest is already zero-initialized
        } else if constexpr (std::is_same_v<T, bool>) {
            char val = arg ? 1 : 0;
            memcpy(data.data(), &val, 1);
        } else if constexpr (std::is_same_v<T, float>) {
            memcpy(data.data(), &arg, sizeof(float));
        } else if constexpr (std::is_same_v<T, time_t>) {
            memcpy(data.data(), &arg, sizeof(time_t));
        }
    }, value);
    
    return data;
}

// Helper: Deserialize a single value
Record::Value Record::deserializeValue(const std::vector<char>& data, 
                                       size_t& offset, 
                                       const Column& column) const {
    Value result;
    
    switch(column.type) {
        case DataType::INT: {
            int val = 0;
            memcpy(&val, data.data() + offset, sizeof(int));
            offset += sizeof(int);
            result = val;
            break;
        }
        case DataType::VARCHAR: {
            std::string str(column.maxLength, '\0');
            memcpy(str.data(), data.data() + offset, column.maxLength);
            // Trim null characters
            size_t len = str.find('\0');
            if(len != std::string::npos) {
                str.resize(len);
            }
            offset += column.maxLength;
            result = str;
            break;
        }
        case DataType::BOOLEAN: {
            char val = 0;
            memcpy(&val, data.data() + offset, 1);
            offset += 1;
            result = (val != 0);
            break;
        }
        case DataType::FLOAT: {
            float val = 0.0f;
            memcpy(&val, data.data() + offset, sizeof(float));
            offset += sizeof(float);
            result = val;
            break;
        }
        case DataType::DATE: {
            time_t val = 0;
            memcpy(&val, data.data() + offset, sizeof(time_t));
            offset += sizeof(time_t);
            result = val;
            break;
        }
        default:
            throw std::runtime_error("Unknown data type for deserialization");
    }
    
    return result;
}
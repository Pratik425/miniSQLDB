#include "models/Table.h"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <iomanip>
#include <cstring>

// ========== Constructor ==========

Table::Table(const std::string& name, 
             const std::vector<Column>& schema, 
             const std::string& databasePath)
    : name(name)
    , schema(schema)
    , recordCount(0)
    , pageSize(4096) {  // Default page size
    
    // Build column index map for fast lookup
    for (size_t i = 0; i < schema.size(); ++i) {
        columnIndexMap[schema[i].name] = i;
    }
    
    // Set file names
    dataFile = name + ".db";
    metaFile = name + "_meta.txt";
    
    // Initialize FileManager
    fileManager = std::make_unique<FileManager>(databasePath);
}

// ========== DDL Operations ==========

void Table::createTable() {
    // Check if table already exists
    if (fileManager->fileExists(dataFile)) {
        throw std::runtime_error("Table already exists: " + name);
    }
    
    // Create data file (empty)
    fileManager->createFile(dataFile);
    
    // Save schema to metadata file
    saveSchema();
    
    // Update statistics
    recordCount = 0;
    
    // Log success
    std::cout << "✓ Table '" << name << "' created successfully" << std::endl;
}

void Table::dropTable() {
    // Delete data file
    if (fileManager->fileExists(dataFile)) {
        fileManager->deleteFile(dataFile);
    }
    
    // Delete metadata file
    if (fileManager->fileExists(metaFile)) {
        fileManager->deleteFile(metaFile);
    }
    
    // Clear in-memory data
    records.clear();
    recordCount = 0;
    
    std::cout << "✓ Table '" << name << "' dropped successfully" << std::endl;
}

void Table::saveToDisk() {
    // Validate we have records to save
    if (records.empty()) {
        // Write empty file
        fileManager->writeFile(dataFile, std::vector<char>());
        return;
    }
    
    // Serialize all records
    std::vector<char> allData;
    for (const auto& record : records) {
        auto serialized = serializeRecord(record);
        allData.insert(allData.end(), serialized.begin(), serialized.end());
    }
    
    // Write to file
    if (!fileManager->writeFile(dataFile, allData)) {
        throw std::runtime_error("Failed to save table: " + name);
    }
    
    // Save schema (in case it changed)
    saveSchema();
    
    // Update count
    recordCount = records.size();
}

void Table::loadFromDisk() {
    // Check if data file exists
    if (!fileManager->fileExists(dataFile)) {
        throw std::runtime_error("Table data file not found: " + dataFile);
    }
    
    // Clear existing records
    records.clear();
    
    // Read data file
    auto data = fileManager->readFile(dataFile);
    
    if (data.empty()) {
        recordCount = 0;
        return;
    }
    
    // Deserialize records
    size_t offset = 0;
    size_t recordSize = 0;
    
    // Calculate record size from schema
    for (const auto& column : schema) {
        recordSize += column.getStorageSize();
    }
    
    // Ensure data size is multiple of record size
    if (data.size() % recordSize != 0) {
        throw std::runtime_error("Corrupted data file for table: " + name);
    }
    
    // Deserialize each record
    while (offset < data.size()) {
        std::vector<char> recordData(data.begin() + offset, 
                                     data.begin() + offset + recordSize);
        Record record = deserializeRecord(recordData);
        records.push_back(record);
        offset += recordSize;
    }
    
    recordCount = records.size();
}

// ========== DML Operations ==========

void Table::insertRecord(const Record& record) {
    // Validate record against schema
    validateRecord(record);
    
    // Add to in-memory cache
    records.push_back(record);
    recordCount++;
    
    // Optionally save to disk immediately (can be optimized later)
    saveToDisk();
}

void Table::insertRecords(const std::vector<Record>& newRecords) {
    for (const auto& record : newRecords) {
        validateRecord(record);
        records.push_back(record);
    }
    recordCount += newRecords.size();
    
    // Save all at once
    saveToDisk();
}

std::vector<Record> Table::selectAll() const {
    return records;
}

std::vector<Record> Table::selectWhere(
    const std::function<bool(const Record&)>& predicate) const {
    
    std::vector<Record> result;
    std::copy_if(records.begin(), records.end(), 
                 std::back_inserter(result), predicate);
    return result;
}

size_t Table::updateWhere(
    const std::function<bool(const Record&)>& predicate,
    const std::function<void(Record&)>& updater) {
    
    size_t updatedCount = 0;
    
    for (auto& record : records) {
        if (predicate(record)) {
            updater(record);
            updatedCount++;
        }
    }
    
    // Save changes if any updates were made
    if (updatedCount > 0) {
        saveToDisk();
    }
    
    return updatedCount;
}

size_t Table::deleteWhere(const std::function<bool(const Record&)>& predicate) {
    // Find records to delete
    auto it = std::remove_if(records.begin(), records.end(), predicate);
    size_t deletedCount = std::distance(it, records.end());
    
    // Remove them
    records.erase(it, records.end());
    recordCount = records.size();
    
    // Save changes if any deletions were made
    if (deletedCount > 0) {
        saveToDisk();
    }
    
    return deletedCount;
}

const Record& Table::getRecord(size_t index) const {
    if (index >= records.size()) {
        throw std::out_of_range("Record index out of range: " + 
                                std::to_string(index));
    }
    return records[index];
}

// ========== Utility Methods ==========

size_t Table::getColumnIndex(const std::string& columnName) const {
    auto it = columnIndexMap.find(columnName);
    if (it == columnIndexMap.end()) {
        throw std::runtime_error("Column not found: " + columnName);
    }
    return it->second;
}

bool Table::hasColumn(const std::string& columnName) const {
    return columnIndexMap.find(columnName) != columnIndexMap.end();
}

void Table::clearRecords() {
    records.clear();
    recordCount = 0;
}

void Table::printTable() const {
    if (records.empty()) {
        std::cout << "Table '" << name << "' is empty" << std::endl;
        return;
    }
    
    // Print header
    std::cout << "\n=== Table: " << name << " ===" << std::endl;
    std::cout << "Records: " << records.size() << std::endl;
    std::cout << "Columns: " << schema.size() << std::endl;
    std::cout << std::endl;
    
    // Print column names
    for (const auto& col : schema) {
        std::cout << std::setw(15) << col.name << " ";
    }
    std::cout << std::endl;
    
    // Print separator
    for (size_t i = 0; i < schema.size(); ++i) {
        std::cout << std::setw(15) << "---------------" << " ";
    }
    std::cout << std::endl;
    
    // Print each record
    for (const auto& record : records) {
        for (size_t i = 0; i < schema.size(); ++i) {
            auto value = record.getValue(i);
            std::visit([&](auto&& arg) {
                std::cout << std::setw(15) << arg << " ";
            }, value);
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

// ========== Serialization Helpers ==========

std::vector<char> Table::serializeRecord(const Record& record) const {
    return record.serialize();
}

Record Table::deserializeRecord(const std::vector<char>& data) const {
    Record record(schema);
    record.deserialize(data);
    return record;
}

void Table::saveSchema() {
    std::stringstream ss;
    for (const auto& column : schema) {
        ss << column.toString() << "\n";
    }
    
    std::string schemaStr = ss.str();
    std::vector<char> schemaData(schemaStr.begin(), schemaStr.end());
    
    if (!fileManager->writeFile(metaFile, schemaData)) {
        throw std::runtime_error("Failed to save schema for table: " + name);
    }
}

void Table::loadSchema() {
    if (!fileManager->fileExists(metaFile)) {
        throw std::runtime_error("Schema file not found: " + metaFile);
    }
    
    auto data = fileManager->readFile(metaFile);
    std::string schemaStr(data.begin(), data.end());
    
    // Parse schema (format: "name TYPE constraints\n")
    std::vector<Column> loadedSchema;
    std::stringstream ss(schemaStr);
    std::string line;
    
    while (std::getline(ss, line)) {
        if (line.empty()) continue;
        
        // Simple parsing - in production, use proper parser
        std::stringstream lineStream(line);
        std::string colName, colType, constraint;
        
        lineStream >> colName >> colType;
        
        // Determine if it's VARCHAR with length
        DataType type = DataType::INT;
        size_t maxLength = 255;
        bool isPrimaryKey = false;
        bool isNullable = true;
        
        if (colType.find("VARCHAR") != std::string::npos) {
            type = DataType::VARCHAR;
            // Extract length from VARCHAR(50)
            size_t start = colType.find('(');
            size_t end = colType.find(')');
            if (start != std::string::npos && end != std::string::npos) {
                maxLength = std::stoi(colType.substr(start + 1, end - start - 1));
            }
        } else if (colType == "INT") {
            type = DataType::INT;
        } else if (colType == "BOOLEAN") {
            type = DataType::BOOLEAN;
        } else if (colType == "FLOAT") {
            type = DataType::FLOAT;
        } else if (colType == "DATE") {
            type = DataType::DATE;
        }
        
        // Check for constraints in remaining tokens
        std::string remaining;
        while (lineStream >> remaining) {
            if (remaining == "PRIMARY" || remaining == "KEY") {
                isPrimaryKey = true;
            } else if (remaining == "NOT" || remaining == "NULL") {
                isNullable = false;
            }
        }
        
        loadedSchema.emplace_back(colName, type, maxLength, isPrimaryKey, isNullable);
    }
    
    // Use loaded schema
    schema = loadedSchema;
    
    // Rebuild column index map
    columnIndexMap.clear();
    for (size_t i = 0; i < schema.size(); ++i) {
        columnIndexMap[schema[i].name] = i;
    }
}

void Table::validateRecord(const Record& record) const {
    // Check column count
    if (record.getColumnCount() != schema.size()) {
        throw std::runtime_error("Record column count does not match schema. "
                                 "Expected: " + std::to_string(schema.size()) + 
                                 ", Got: " + std::to_string(record.getColumnCount()));
    }
    
    // Check each value against schema
    for (size_t i = 0; i < schema.size(); ++i) {
        auto value = record.getValue(i);
        const auto& column = schema[i];
        
        // Type checking is done by Record class
        // Additional constraints can be checked here
    }
}
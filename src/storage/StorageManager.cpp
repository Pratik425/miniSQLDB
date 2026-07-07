#include "storage/StorageManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <stdexcept>
#include <algorithm>

namespace fs = std::filesystem;

// ========== Constructor ==========

StorageManager::StorageManager(const std::string& dbPath) 
    : databasePath(dbPath) {
    
    // Initialize FileManager
    fileManager = std::make_unique<FileManager>(dbPath);
    
    // Load existing tables if any
    if (databaseExists()) {
        loadDatabase();
    }
}

// ========== Table Management ==========

void StorageManager::createTable(const std::string& name, 
                                 const std::vector<Column>& schema) {
    // Check if table already exists
    if (tableExists(name)) {
        throw std::runtime_error("Table already exists: " + name);
    }
    
    // Create table object
    auto table = std::make_unique<Table>(name, schema, databasePath);
    table->createTable();
    
    // Add to cache
    tables[name] = std::move(table);
    
    std::cout << "✓ Table '" << name << "' created in database" << std::endl;
}

void StorageManager::dropTable(const std::string& name) {
    // Check if table exists
    auto it = tables.find(name);
    if (it == tables.end()) {
        throw std::runtime_error("Table does not exist: " + name);
    }
    
    // Drop table (deletes files)
    it->second->dropTable();
    
    // Remove from cache
    tables.erase(it);
    
    std::cout << "✓ Table '" << name << "' dropped from database" << std::endl;
}

Table* StorageManager::getTable(const std::string& name) {
    auto it = tables.find(name);
    if (it != tables.end()) {
        return it->second.get();
    }
    return nullptr;
}

const Table* StorageManager::getTable(const std::string& name) const {
    auto it = tables.find(name);
    if (it != tables.end()) {
        return it->second.get();
    }
    return nullptr;
}

bool StorageManager::tableExists(const std::string& name) const {
    return tables.find(name) != tables.end();
}

std::vector<std::string> StorageManager::listTables() const {
    std::vector<std::string> tableNames;
    tableNames.reserve(tables.size());
    
    for (const auto& [name, table] : tables) {
        tableNames.push_back(name);
    }
    
    std::sort(tableNames.begin(), tableNames.end());
    return tableNames;
}

// ========== Database Operations ==========

bool StorageManager::databaseExists() const {
    // Check if database directory exists and has metadata file
    std::string metaFile = databasePath + "/.db_meta";
    return fs::exists(databasePath) && fs::exists(metaFile);
}

void StorageManager::loadDatabase() {
    if (!fs::exists(databasePath)) {
        return;  // No database to load
    }
    
    // Find all .db files in directory
    std::vector<std::string> tableFiles;
    for (const auto& entry : fs::directory_iterator(databasePath)) {
        if (entry.path().extension() == ".db") {
            std::string filename = entry.path().filename().string();
            // Remove .db extension
            std::string tableName = filename.substr(0, filename.find_last_of('.'));
            tableFiles.push_back(tableName);
        }
    }
    
    // Load each table
    for (const auto& tableName : tableFiles) {
        try {
            loadTable(tableName);
        } catch (const std::exception& e) {
            std::cerr << "Warning: Failed to load table '" << tableName 
                      << "': " << e.what() << std::endl;
        }
    }
    
    std::cout << "✓ Loaded " << tables.size() << " tables from database" << std::endl;
}

void StorageManager::saveDatabase() {
    // Save all tables
    for (auto& [name, table] : tables) {
        try {
            table->saveToDisk();
        } catch (const std::exception& e) {
            std::cerr << "Warning: Failed to save table '" << name 
                      << "': " << e.what() << std::endl;
        }
    }
    
    // Create database metadata file
    std::string metaFile = databasePath + "/.db_meta";
    std::ofstream meta(metaFile);
    if (meta) {
        meta << "# MiniDB Database Metadata" << std::endl;
        meta << "# Tables: " << tables.size() << std::endl;
        for (const auto& [name, table] : tables) {
            meta << "table: " << name 
                 << " (" << table->getRecordCount() << " records)" << std::endl;
        }
        meta.close();
    }
    
    std::cout << "✓ Database saved (" << tables.size() << " tables)" << std::endl;
}

void StorageManager::loadTable(const std::string& tableName) {
    // Load schema from metadata file
    std::vector<Column> schema = loadTableSchema(tableName);
    
    // Create table object
    auto table = std::make_unique<Table>(tableName, schema, databasePath);
    
    // Load data from disk
    table->loadFromDisk();
    
    // Add to cache
    tables[tableName] = std::move(table);
}

void StorageManager::saveTable(const std::string& tableName) {
    auto it = tables.find(tableName);
    if (it == tables.end()) {
        throw std::runtime_error("Table not found: " + tableName);
    }
    it->second->saveToDisk();
}

std::vector<Column> StorageManager::loadTableSchema(const std::string& tableName) const {
    std::string metaFile = databasePath + "/" + tableName + "_meta.txt";
    
    if (!fs::exists(metaFile)) {
        throw std::runtime_error("Metadata file not found: " + metaFile);
    }
    
    std::ifstream file(metaFile);
    if (!file) {
        throw std::runtime_error("Cannot open metadata file: " + metaFile);
    }
    
    std::vector<Column> schema;
    std::string line;
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        // Parse column definition
        std::stringstream ss(line);
        std::string colName, colType, token;
        
        ss >> colName >> colType;
        
        // Parse type and constraints
        DataType type = DataType::INT;
        size_t maxLength = 255;
        bool isPrimaryKey = false;
        bool isNullable = true;
        
        // Handle VARCHAR(length)
        if (colType.find("VARCHAR") != std::string::npos) {
            type = DataType::VARCHAR;
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
        } else {
            continue;  // Skip unknown types
        }
        
        // Parse constraints from remaining tokens
        while (ss >> token) {
            if (token == "PRIMARY" || token == "KEY") {
                isPrimaryKey = true;
            } else if (token == "NOT" || token == "NULL") {
                isNullable = false;
            }
        }
        
        schema.emplace_back(colName, type, maxLength, isPrimaryKey, isNullable);
    }
    
    return schema;
}

std::string StorageManager::getDataFileName(const std::string& tableName) const {
    return tableName + ".db";
}

std::string StorageManager::getMetaFileName(const std::string& tableName) const {
    return tableName + "_meta.txt";
}

// ========== Utility Methods ==========

void StorageManager::printDatabase() const {
    std::cout << "\n========================================" << std::endl;
    std::cout << "  Database: " << databasePath << std::endl;
    std::cout << "  Tables: " << tables.size() << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    for (const auto& [name, table] : tables) {
        table->printTable();
    }
}

void StorageManager::clearAllTables() {
    tables.clear();
    std::cout << "✓ All tables cleared from memory" << std::endl;
}
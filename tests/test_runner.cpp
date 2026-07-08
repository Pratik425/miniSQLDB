#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <filesystem>
#include "utils/FileManager.h"
#include "models/Column.h"
#include "models/Record.h"
#include "models/Table.h"
#include "storage/StorageManager.h"

// Unified test macro that works for all tests
#define TEST(name) void test_##name()

// This macro runs a test and handles exceptions
#define RUN_TEST(name) \
    do { \
        std::cout << "Running test: " #name " ... "; \
        try { \
            test_##name(); \
            std::cout << "✅ PASSED" << std::endl; \
        } catch(const std::exception& e) { \
            std::cout << "❌ FAILED" << std::endl; \
            std::cout << "  Error: " << e.what() << std::endl; \
        } catch(...) { \
            std::cout << "❌ FAILED (unknown error)" << std::endl; \
        } \
    } while(0)

// Alternative: Test with counting
#define RUN_TEST_WITH_COUNT(name) \
    do { \
        total++; \
        try { \
            std::cout << "Test " << total << ": " #name " ... "; \
            test_##name(); \
            std::cout << "✅ PASSED" << std::endl; \
            passed++; \
        } catch(const std::exception& e) { \
            std::cout << "❌ FAILED" << std::endl; \
            std::cout << "  Error: " << e.what() << std::endl; \
        } catch(...) { \
            std::cout << "❌ FAILED (unknown error)" << std::endl; \
        } \
    } while(0)

// ========== Test 1: Column Creation ==========
TEST(column_creation) {
    Column id("id", DataType::INT, 0, true, false);
    Column name("name", DataType::VARCHAR, 50, false, true);
    Column age("age", DataType::INT, 0, false, true);
    
    assert(id.name == "id");
    assert(id.type == DataType::INT);
    assert(id.isPrimaryKey == true);
    assert(id.isNullable == false);
    
    assert(name.name == "name");
    assert(name.type == DataType::VARCHAR);
    assert(name.maxLength == 50);
    assert(name.isNullable == true);
    
    // Test storage sizes
    assert(id.getStorageSize() == sizeof(int));
    assert(name.getStorageSize() == 50);
    assert(age.getStorageSize() == sizeof(int));
    
    // Test toString
    std::cout << "\n  Column definitions:";
    std::cout << "\n    " << id.toString();
    std::cout << "\n    " << name.toString();
    std::cout << "\n    " << age.toString();
    std::cout << std::endl;
}

// ========== Test 2: Record Operations ==========
TEST(record_operations) {
    std::vector<Column> schema = {
        Column("id", DataType::INT, 0, true, false),
        Column("name", DataType::VARCHAR, 50, false, true),
        Column("age", DataType::INT, 0, false, true)
    };
    
    Record record(schema);
    
    // Set values
    record.setValue(0, 1);
    record.setValue(1, std::string("John Doe"));
    record.setValue(2, 30);
    
    // Get and verify values
    assert(std::get<int>(record.getValue(0)) == 1);
    assert(std::get<std::string>(record.getValue(1)) == "John Doe");
    assert(std::get<int>(record.getValue(2)) == 30);
    
    // Test serialization
    auto serialized = record.serialize();
    assert(serialized.size() > 0);
    
    std::cout << "\n  Record: " << record.toString();
    std::cout << "\n  Serialized size: " << serialized.size() << " bytes" << std::endl;
}

// ========== Test 3: FileManager - Create and Check ==========
TEST(filemanager_create) {
    FileManager fm("test_db");
    std::string filename = "test.dat";
    
    // Should not exist initially
    assert(!fm.fileExists(filename));
    
    // Create file
    assert(fm.createFile(filename));
    assert(fm.fileExists(filename));
    
    // Create again should fail
    assert(!fm.createFile(filename));
    
    // Cleanup
    fm.deleteFile(filename);
    assert(!fm.fileExists(filename));
    
    std::cout << "\n  ✓ File creation and deletion working" << std::endl;
}

// ========== Test 4: FileManager - Write and Read ==========
TEST(filemanager_write_read) {
    FileManager fm("test_db");
    std::string filename = "data.dat";
    std::vector<char> writeData = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd'};
    
    fm.writeFile(filename, writeData);
    auto readData = fm.readFile(filename);
    
    assert(writeData.size() == readData.size());
    assert(std::equal(writeData.begin(), writeData.end(), readData.begin()));
    
    std::cout << "\n  ✓ Write/Read working. Data: ";
    for (char c : readData) std::cout << c;
    std::cout << std::endl;
    
    fm.deleteFile(filename);
}

// ========== Test 5: FileManager - Append ==========
TEST(filemanager_append) {
    FileManager fm("test_db");
    std::string filename = "append.dat";
    std::vector<char> first = {'F', 'i', 'r', 's', 't'};
    std::vector<char> second = {'S', 'e', 'c', 'o', 'n', 'd'};
    
    fm.writeFile(filename, first);
    fm.appendToFile(filename, second);
    
    auto readData = fm.readFile(filename);
    std::vector<char> expected;
    expected.insert(expected.end(), first.begin(), first.end());
    expected.insert(expected.end(), second.begin(), second.end());
    
    assert(readData.size() == expected.size());
    assert(std::equal(expected.begin(), expected.end(), readData.begin()));
    
    std::cout << "\n  ✓ Append working. Data: ";
    for (char c : readData) std::cout << c;
    std::cout << std::endl;
    
    fm.deleteFile(filename);
}

// ========== Test 6: FileManager - List Files ==========
TEST(filemanager_list) {
    FileManager fm("test_db");
    
    // Create test files
    fm.createFile("file1.txt");
    fm.createFile("file2.txt");
    fm.createFile("file3.dat");
    
    auto files = fm.listFiles();
    assert(files.size() >= 3);
    
    std::cout << "\n  ✓ List files working. Found: ";
    for (const auto& f : files) std::cout << f << " ";
    std::cout << std::endl;
    
    // Cleanup
    for (const auto& f : files) {
        fm.deleteFile(f);
    }
}

// ========== Test 7: FileManager - Get Size ==========
TEST(filemanager_size) {
    FileManager fm("test_db");
    std::string filename = "size.dat";
    std::vector<char> data(100, 'A');  // 100 bytes of 'A'
    
    fm.writeFile(filename, data);
    size_t size = fm.getFileSize(filename);
    
    assert(size == 100);
    
    std::cout << "\n  ✓ File size working. Size: " << size << " bytes" << std::endl;
    
    fm.deleteFile(filename);
}

// ========== Test 8: Table Creation ==========
TEST(table_creation) {
    std::string dbPath = "test_db";
    std::vector<Column> schema = {
        Column("id", DataType::INT, 0, true, false),
        Column("name", DataType::VARCHAR, 50, false, true),
        Column("age", DataType::INT, 0, false, true)
    };
    
    Table table("users", schema, dbPath);
    table.createTable();
    
    // Verify files were created
    FileManager fm(dbPath);
    assert(fm.fileExists("users.db"));
    assert(fm.fileExists("users_meta.txt"));
    
    std::cout << "\n  ✓ Table created successfully";
    std::cout << "\n  Schema: ";
    for (const auto& col : table.getSchema()) {
        std::cout << col.toString() << " ";
    }
    std::cout << std::endl;
    
    // Cleanup
    table.dropTable();
    
    // Verify cleanup
    assert(!fm.fileExists("users.db"));
    assert(!fm.fileExists("users_meta.txt"));
}

// ========== Test 9: Insert and Select ==========
TEST(table_insert_select) {
    std::string dbPath = "test_db";
    std::vector<Column> schema = {
        Column("id", DataType::INT, 0, true, false),
        Column("name", DataType::VARCHAR, 50),
        Column("age", DataType::INT)
    };
    
    Table table("users", schema, dbPath);
    table.createTable();
    
    // Insert records
    std::vector<Record> records;
    
    Record r1(schema);
    r1.setValue(0, 1);
    r1.setValue(1, std::string("Alice"));
    r1.setValue(2, 25);
    records.push_back(r1);
    
    Record r2(schema);
    r2.setValue(0, 2);
    r2.setValue(1, std::string("Bob"));
    r2.setValue(2, 30);
    records.push_back(r2);
    
    Record r3(schema);
    r3.setValue(0, 3);
    r3.setValue(1, std::string("Charlie"));
    r3.setValue(2, 35);
    records.push_back(r3);
    
    table.insertRecords(records);
    
    // Select all
    auto allRecords = table.selectAll();
    assert(allRecords.size() == 3);
    
    // Select where age > 28
    auto filtered = table.selectWhere([](const Record& r) {
        return std::get<int>(r.getValue(2)) > 28;
    });
    assert(filtered.size() == 2); // Bob and Charlie
    
    std::cout << "\n  ✓ Insert and select working";
    std::cout << "\n  All records: " << allRecords.size();
    std::cout << "\n  Filtered (age > 28): " << filtered.size();
    
    // Test selectAll returns correct data
    auto firstRecord = allRecords[0];
    assert(std::get<int>(firstRecord.getValue(0)) == 1);
    assert(std::get<std::string>(firstRecord.getValue(1)) == "Alice");
    assert(std::get<int>(firstRecord.getValue(2)) == 25);
    
    std::cout << std::endl;
    
    // Cleanup
    table.dropTable();
}

// ========== Test 10: Update ==========
TEST(table_update) {
    std::string dbPath = "test_db";
    std::vector<Column> schema = {
        Column("id", DataType::INT, 0, true, false),
        Column("name", DataType::VARCHAR, 50),
        Column("age", DataType::INT)
    };
    
    Table table("users", schema, dbPath);
    table.createTable();
    
    // Insert test data
    Record r1(schema);
    r1.setValue(0, 1);
    r1.setValue(1, std::string("Alice"));
    r1.setValue(2, 25);
    table.insertRecord(r1);
    
    Record r2(schema);
    r2.setValue(0, 2);
    r2.setValue(1, std::string("Bob"));
    r2.setValue(2, 30);
    table.insertRecord(r2);
    
    // Update: Increment age of all users
    size_t updated = table.updateWhere(
        [](const Record&) { return true; },  // All records
        [](Record& r) {
            int age = std::get<int>(r.getValue(2));
            r.setValue(2, age + 1);
        }
    );
    
    assert(updated == 2);
    
    // Verify update
    auto allRecords = table.selectAll();
    assert(std::get<int>(allRecords[0].getValue(2)) == 26);
    assert(std::get<int>(allRecords[1].getValue(2)) == 31);
    
    std::cout << "\n  ✓ Update working. Updated " << updated << " records";
    std::cout << "\n  Alice's age: " << std::get<int>(allRecords[0].getValue(2));
    std::cout << "\n  Bob's age: " << std::get<int>(allRecords[1].getValue(2));
    std::cout << std::endl;
    
    // Cleanup
    table.dropTable();
}

// ========== Test 11: Delete ==========
TEST(table_delete) {
    std::string dbPath = "test_db";
    std::vector<Column> schema = {
        Column("id", DataType::INT, 0, true, false),
        Column("name", DataType::VARCHAR, 50),
        Column("age", DataType::INT)
    };
    
    Table table("users", schema, dbPath);
    table.createTable();
    
    // Insert test data
    std::vector<Record> records;
    for (int i = 1; i <= 5; ++i) {
        Record r(schema);
        r.setValue(0, i);
        r.setValue(1, std::string("User") + std::to_string(i));
        r.setValue(2, 20 + i);
        records.push_back(r);
    }
    table.insertRecords(records);
    
    assert(table.getRecordCount() == 5);
    
    // Delete users with age > 23
    size_t deleted = table.deleteWhere([](const Record& r) {
        return std::get<int>(r.getValue(2)) > 23;
    });
    
    assert(deleted == 2); // Users with age 24 and 25
    
    auto remaining = table.selectAll();
    assert(remaining.size() == 3);
    
    std::cout << "\n  ✓ Delete working. Deleted " << deleted << " records";
    std::cout << "\n  Remaining records: " << remaining.size();
    std::cout << std::endl;
    
    // Cleanup
    table.dropTable();
}

// ========== Test 12: Persistence ==========
TEST(table_persistence) {
    std::string dbPath = "test_db";
    std::vector<Column> schema = {
        Column("id", DataType::INT, 0, true, false),
        Column("name", DataType::VARCHAR, 50),
        Column("age", DataType::INT)
    };
    
    // Create and populate table
    {
        Table table("users", schema, dbPath);
        table.createTable();
        
        Record r1(schema);
        r1.setValue(0, 1);
        r1.setValue(1, std::string("Alice"));
        r1.setValue(2, 25);
        table.insertRecord(r1);
        
        Record r2(schema);
        r2.setValue(0, 2);
        r2.setValue(1, std::string("Bob"));
        r2.setValue(2, 30);
        table.insertRecord(r2);
        
        // Table destructor saves automatically
    }
    
    // Load table in new instance
    {
        Table table("users", schema, dbPath);
        table.loadFromDisk();
        
        auto records = table.selectAll();
        assert(records.size() == 2);
        
        assert(std::get<int>(records[0].getValue(0)) == 1);
        assert(std::get<std::string>(records[0].getValue(1)) == "Alice");
        assert(std::get<int>(records[0].getValue(2)) == 25);
        
        assert(std::get<int>(records[1].getValue(0)) == 2);
        assert(std::get<std::string>(records[1].getValue(1)) == "Bob");
        assert(std::get<int>(records[1].getValue(2)) == 30);
        
        std::cout << "\n  ✓ Persistence working. Loaded " << records.size() << " records";
        std::cout << std::endl;
        
        // Cleanup
        table.dropTable();
    }
}

// ========== Test 13: StorageManager ==========
TEST(storage_manager) {
    std::string dbPath = "test_db";
    
    // Create storage manager
    StorageManager db(dbPath);
    
    // Create tables
    std::vector<Column> userSchema = {
        Column("id", DataType::INT, 0, true, false),
        Column("name", DataType::VARCHAR, 50),
        Column("age", DataType::INT)
    };
    
    std::vector<Column> productSchema = {
        Column("id", DataType::INT, 0, true, false),
        Column("name", DataType::VARCHAR, 100),
        Column("price", DataType::FLOAT),
        Column("quantity", DataType::INT)
    };
    
    db.createTable("users", userSchema);
    db.createTable("products", productSchema);
    
    assert(db.tableExists("users"));
    assert(db.tableExists("products"));
    assert(!db.tableExists("nonexistent"));
    
    // Get table and insert
    auto* users = db.getTable("users");
    assert(users != nullptr);
    
    Record r1(userSchema);
    r1.setValue(0, 1);
    r1.setValue(1, std::string("Alice"));
    r1.setValue(2, 25);
    users->insertRecord(r1);
    
    Record r2(userSchema);
    r2.setValue(0, 2);
    r2.setValue(1, std::string("Bob"));
    r2.setValue(2, 30);
    users->insertRecord(r2);
    
    assert(users->getRecordCount() == 2);
    
    // List tables and verify
    auto tables = db.listTables();
    assert(tables.size() == 2);
    
    // Check table names using std::string
    bool hasUsers = false;
    bool hasProducts = false;
    for (const auto& name : tables) {
        if (name == "users") hasUsers = true;
        if (name == "products") hasProducts = true;
    }
    assert(hasUsers);
    assert(hasProducts);
    
    // Test persistence - save and reload
    db.saveDatabase();
    
    // Create new instance and load
    StorageManager db2(dbPath);
    auto* loadedUsers = db2.getTable("users");
    assert(loadedUsers != nullptr);
    assert(loadedUsers->getRecordCount() == 2);
    
    // Verify data
    auto records = loadedUsers->selectAll();
    assert(records.size() == 2);
    assert(std::get<int>(records[0].getValue(0)) == 1);
    assert(std::get<std::string>(records[0].getValue(1)) == "Alice");
    assert(std::get<int>(records[0].getValue(2)) == 25);
    
    std::cout << "\n  ✓ StorageManager working";
    std::cout << "\n  Tables: " << db2.listTables().size();
    std::cout << "\n  Users: " << loadedUsers->getRecordCount();
    std::cout << std::endl;
    
    // Cleanup
    db2.dropTable("users");
    db2.dropTable("products");
}

// ========== Main Test Runner ==========
int main() {
    std::cout << "\n========================================";
    std::cout << "\n  MiniDB Test Suite";
    std::cout << "\n========================================\n" << std::endl;
    
    int passed = 0;
    int total = 0;
    
    // Run all tests with counting
    RUN_TEST_WITH_COUNT(column_creation);
    RUN_TEST_WITH_COUNT(record_operations);
    RUN_TEST_WITH_COUNT(filemanager_create);
    RUN_TEST_WITH_COUNT(filemanager_write_read);
    RUN_TEST_WITH_COUNT(filemanager_append);
    RUN_TEST_WITH_COUNT(filemanager_list);
    RUN_TEST_WITH_COUNT(filemanager_size);
    RUN_TEST_WITH_COUNT(table_creation);
    RUN_TEST_WITH_COUNT(table_insert_select);
    RUN_TEST_WITH_COUNT(table_update);
    RUN_TEST_WITH_COUNT(table_delete);
    RUN_TEST_WITH_COUNT(table_persistence);
    RUN_TEST_WITH_COUNT(storage_manager);
    
    std::cout << "\n========================================";
    std::cout << "\n  Results: " << passed << "/" << total << " tests passed";
    if (passed == total) {
        std::cout << " 🎉 All tests passed!";
    }
    std::cout << "\n========================================\n" << std::endl;
    
    // Cleanup test directory
    try {
        std::filesystem::remove_all("test_db");
    } catch(...) {
        // Ignore cleanup errors
    }
    
    return (passed == total) ? 0 : 1;
}
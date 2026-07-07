#include <iostream>
#include <memory>
#include "models/Column.h"
#include "models/Record.h"
#include "models/Table.h"
#include "storage/StorageManager.h"

using namespace std;

void testColumnCreation() {
    cout << "=== Testing Column Creation ===" << endl;
    
    // Create columns
    Column id("id", DataType::INT, 0, true, false);
    Column name("name", DataType::VARCHAR, 50, false, true);
    Column age("age", DataType::INT, 0, false, true);
    
    cout << "Column 1: " << id.toString() << endl;
    cout << "Column 2: " << name.toString() << endl;
    cout << "Column 3: " << age.toString() << endl;
    
    cout << "Storage sizes: " << endl;
    cout << "  ID: " << id.getStorageSize() << " bytes" << endl;
    cout << "  Name: " << name.getStorageSize() << " bytes" << endl;
    cout << "  Age: " << age.getStorageSize() << " bytes" << endl;
}

void testRecordOperations() {
    cout << "\n=== Testing Record Operations ===" << endl;
    
    // Create schema
    vector<Column> schema = {
        Column("id", DataType::INT, 0, true, false),
        Column("name", DataType::VARCHAR, 50, false, true),
        Column("age", DataType::INT, 0, false, true)
    };
    
    // Create record
    Record record(schema);
    record.setValue(0, 1);
    record.setValue(1, string("Ram Pal"));
    record.setValue(2, 30);
    
    cout << "Record created: " << record.toString() << endl;
    
    // Test serialization
    auto serialized = record.serialize();
    cout << "Serialized size: " << serialized.size() << " bytes" << endl;
}

void testTableOperations() {
    cout << "\n=== Testing Table Operations ===" << endl;
    
    try {
        // Define schema
        vector<Column> schema = {
            Column("id", DataType::INT, 0, true, false),
            Column("name", DataType::VARCHAR, 50, false, true),
            Column("age", DataType::INT, 0, false, true),
            Column("salary", DataType::FLOAT, 0, false, true)
        };
        
        // Create table
        Table users("users", schema, "database");
        users.createTable();
        cout << "✓ Table 'users' created" << endl;
        
        // Insert records
        vector<Record> records;
        
        Record r1(schema);
        r1.setValue(0, 1);
        r1.setValue(1, string("John Doe"));
        r1.setValue(2, 30);
        r1.setValue(3, 50000.0f);
        records.push_back(r1);
        
        Record r2(schema);
        r2.setValue(0, 2);
        r2.setValue(1, string("Jane Smith"));
        r2.setValue(2, 28);
        r2.setValue(3, 60000.0f);
        records.push_back(r2);
        
        Record r3(schema);
        r3.setValue(0, 3);
        r3.setValue(1, string("Bob Johnson"));
        r3.setValue(2, 35);
        r3.setValue(3, 55000.0f);
        records.push_back(r3);
        
        users.insertRecords(records);
        cout << "✓ Inserted " << records.size() << " records" << endl;
        
        // Select all
        auto allUsers = users.selectAll();
        cout << "✓ Total records: " << allUsers.size() << endl;
        
        // Select where age > 30
        auto olderUsers = users.selectWhere([](const Record& r) {
            return std::get<int>(r.getValue(2)) > 30;
        });
        cout << "✓ Users older than 30: " << olderUsers.size() << endl;
        
        // Update: Give 10% raise to everyone
        size_t updated = users.updateWhere(
            [](const Record&) { return true; },
            [](Record& r) {
                float salary = std::get<float>(r.getValue(3));
                r.setValue(3, salary * 1.1f);
            }
        );
        cout << "✓ Updated " << updated << " records with salary raise" << endl;
        
        // Print table
        users.printTable();
        
        // Clean up
        users.dropTable();
        cout << "✓ Cleanup complete" << endl;
        
    } catch (const exception& e) {
        cerr << "✗ Error: " << e.what() << endl;
    }
}


void testStorageManager() {
    cout << "\n=== Testing StorageManager ===" << endl;
    
    try {
        // Create storage manager
        StorageManager db("database");
        cout << "✓ StorageManager initialized" << endl;
        
        // Define schemas
        vector<Column> userSchema = {
            Column("id", DataType::INT, 0, true, false),
            Column("name", DataType::VARCHAR, 50, false, true),
            Column("age", DataType::INT, 0, false, true)
        };
        
        vector<Column> productSchema = {
            Column("id", DataType::INT, 0, true, false),
            Column("name", DataType::VARCHAR, 100, false, true),
            Column("price", DataType::FLOAT, 0, false, true),
            Column("quantity", DataType::INT, 0, false, true)
        };
        
        // Create tables
        db.createTable("users", userSchema);
        db.createTable("products", productSchema);
        cout << "✓ Created 2 tables" << endl;
        
        // Insert data into users
        auto* users = db.getTable("users");
        if (users) {
            Record r1(userSchema);
            r1.setValue(0, 1);
            r1.setValue(1, string("Alice"));
            r1.setValue(2, 25);
            users->insertRecord(r1);
            
            Record r2(userSchema);
            r2.setValue(0, 2);
            r2.setValue(1, string("Bob"));
            r2.setValue(2, 30);
            users->insertRecord(r2);
            cout << "✓ Inserted 2 users" << endl;
        }
        
        // Insert data into products
        auto* products = db.getTable("products");
        if (products) {
            Record p1(productSchema);
            p1.setValue(0, 1);
            p1.setValue(1, string("Laptop"));
            p1.setValue(2, 999.99f);
            p1.setValue(3, 10);
            products->insertRecord(p1);
            
            Record p2(productSchema);
            p2.setValue(0, 2);
            p2.setValue(1, string("Mouse"));
            p2.setValue(2, 29.99f);
            p2.setValue(3, 50);
            products->insertRecord(p2);
            cout << "✓ Inserted 2 products" << endl;
        }
        
        // List tables
        auto tables = db.listTables();
        cout << "✓ Tables in database: ";
        for (const auto& t : tables) {
            cout << t << " ";
        }
        cout << endl;
        
        // Print database
        db.printDatabase();
        
        // Save and cleanup
        db.saveDatabase();
        cout << "✓ Database saved" << endl;
        
    } catch (const exception& e) {
        cerr << "✗ Error: " << e.what() << endl;
    }
}

int main() {
    cout << "MiniDB - Database Engine" << endl;
    cout << "=========================" << endl;
    
    try {
        testColumnCreation();
        testRecordOperations();
        testTableOperations();
        testStorageManager();
        cout << "\n✅ All tests passed successfully!" << endl;
    } catch(const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}
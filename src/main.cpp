#include <iostream>
#include <memory>
#include "models/Column.h"
#include "models/Record.h"

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

int main() {
    cout << "MiniDB - Database Engine" << endl;
    cout << "=========================" << endl;
    
    try {
        testColumnCreation();
        testRecordOperations();
    } catch(const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}
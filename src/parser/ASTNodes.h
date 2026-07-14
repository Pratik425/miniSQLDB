#pragma once
#include <string>
#include <vector>
#include <memory>
#include <variant>
#include "parser/SQLTokenizer.h"

/**
 * @brief Abstract base class for all AST nodes
 * 
 * The AST (Abstract Syntax Tree) represents the structure of
 * a SQL query in memory. Each node type corresponds to a
 * SQL statement or clause.
 */
struct ASTNode {
    enum class Type {
        // Statements
        CREATE_TABLE,
        DROP_TABLE,
        INSERT,
        SELECT,
        UPDATE,
        DELETE,
        
        // Clauses
        WHERE_CLAUSE,
        SET_CLAUSE,
        COLUMN_DEF,
        
        // Expressions
        COMPARISON,
        BINARY_OP,
        LITERAL,
        IDENTIFIER,
        
        // Other
        COLUMN_LIST,
        VALUE_LIST
    };
    
    Type type;
    size_t line;
    size_t column;
    
    ASTNode(Type type, size_t line = 0, size_t column = 0)
        : type(type), line(line), column(column) {}
    
    virtual ~ASTNode() = default;
    virtual std::string toString() const = 0;
};

// ========== Literal Node ==========

struct LiteralNode : ASTNode {
    std::string value;
    
    LiteralNode(const std::string& value, size_t line = 0, size_t column = 0)
        : ASTNode(Type::LITERAL, line, column), value(value) {}
    
    std::string toString() const override {
        return "Literal(" + value + ")";
    }
};

// ========== Identifier Node ==========

struct IdentifierNode : ASTNode {
    std::string name;
    
    IdentifierNode(const std::string& name, size_t line = 0, size_t column = 0)
        : ASTNode(Type::IDENTIFIER, line, column), name(name) {}
    
    std::string toString() const override {
        return "Identifier(" + name + ")";
    }
};

// ========== Column Definition Node ==========

struct ColumnDefNode : ASTNode {
    std::string name;
    std::string dataType;
    size_t maxLength;
    bool isPrimaryKey;
    bool isNullable;
    
    ColumnDefNode(const std::string& name, 
                  const std::string& dataType,
                  size_t maxLength = 255,
                  bool isPrimaryKey = false,
                  bool isNullable = true,
                  size_t line = 0, 
                  size_t column = 0)
        : ASTNode(Type::COLUMN_DEF, line, column)
        , name(name)
        , dataType(dataType)
        , maxLength(maxLength)
        , isPrimaryKey(isPrimaryKey)
        , isNullable(isNullable) {}
    
    std::string toString() const override {
        std::string result = name + " " + dataType;
        if (maxLength > 0) {
            result += "(" + std::to_string(maxLength) + ")";
        }
        if (isPrimaryKey) result += " PRIMARY KEY";
        if (!isNullable) result += " NOT NULL";
        return result;
    }
};

// ========== CREATE TABLE Node ==========

struct CreateTableNode : ASTNode {
    std::string tableName;
    std::vector<ColumnDefNode> columns;
    
    CreateTableNode(const std::string& tableName, size_t line = 0, size_t column = 0)
        : ASTNode(Type::CREATE_TABLE, line, column), tableName(tableName) {}
    
    std::string toString() const override {
        std::string result = "CREATE TABLE " + tableName + " (";
        for (size_t i = 0; i < columns.size(); ++i) {
            if (i > 0) result += ", ";
            result += columns[i].toString();
        }
        result += ")";
        return result;
    }
};

// ========== DROP TABLE Node ==========

struct DropTableNode : ASTNode {
    std::string tableName;
    
    DropTableNode(const std::string& tableName, size_t line = 0, size_t column = 0)
        : ASTNode(Type::DROP_TABLE, line, column), tableName(tableName) {}
    
    std::string toString() const override {
        return "DROP TABLE " + tableName;
    }
};

// ========== WHERE Clause Node ==========

struct WhereClauseNode : ASTNode {
    std::string left;
    std::string op;
    std::string right;
    
    WhereClauseNode(const std::string& left,
                    const std::string& op,
                    const std::string& right,
                    size_t line = 0,
                    size_t column = 0)
        : ASTNode(Type::WHERE_CLAUSE, line, column)
        , left(left), op(op), right(right) {}
    
    std::string toString() const override {
        return left + " " + op + " " + right;
    }
};

// ========== SET Clause Node (for UPDATE) ==========

struct SetClauseNode : ASTNode {
    std::string column;
    std::string value;
    
    SetClauseNode(const std::string& column,
                  const std::string& value,
                  size_t line = 0,
                  size_t columnPos = 0)
        : ASTNode(Type::SET_CLAUSE, line, columnPos)
        , column(column), value(value) {}
    
    std::string toString() const override {
        return column + " = " + value;
    }
};

// ========== INSERT Node ==========

struct InsertNode : ASTNode {
    std::string tableName;
    std::vector<std::string> columns;
    std::vector<std::vector<std::string>> values;
    
    InsertNode(const std::string& tableName, size_t line = 0, size_t column = 0)
        : ASTNode(Type::INSERT, line, column), tableName(tableName) {}
    
    std::string toString() const override {
        std::string result = "INSERT INTO " + tableName;
        if (!columns.empty()) {
            result += " (";
            for (size_t i = 0; i < columns.size(); ++i) {
                if (i > 0) result += ", ";
                result += columns[i];
            }
            result += ")";
        }
        result += " VALUES (";
        if (!values.empty() && !values[0].empty()) {
            for (size_t i = 0; i < values[0].size(); ++i) {
                if (i > 0) result += ", ";
                result += values[0][i];
            }
        }
        result += ")";
        return result;
    }
};

// ========== SELECT Node ==========

struct SelectNode : ASTNode {
    std::string tableName;
    std::vector<std::string> columns;  // Empty = SELECT *
    std::unique_ptr<WhereClauseNode> whereClause;
    
    SelectNode(const std::string& tableName, size_t line = 0, size_t column = 0)
        : ASTNode(Type::SELECT, line, column), tableName(tableName) {}
    
    std::string toString() const override {
        std::string result = "SELECT ";
        if (columns.empty()) {
            result += "*";
        } else {
            for (size_t i = 0; i < columns.size(); ++i) {
                if (i > 0) result += ", ";
                result += columns[i];
            }
        }
        result += " FROM " + tableName;
        if (whereClause) {
            result += " WHERE " + whereClause->toString();
        }
        return result;
    }
};

// ========== UPDATE Node ==========

struct UpdateNode : ASTNode {
    std::string tableName;
    std::vector<SetClauseNode> setClauses;
    std::unique_ptr<WhereClauseNode> whereClause;
    
    UpdateNode(const std::string& tableName, size_t line = 0, size_t column = 0)
        : ASTNode(Type::UPDATE, line, column), tableName(tableName) {}
    
    std::string toString() const override {
        std::string result = "UPDATE " + tableName + " SET ";
        for (size_t i = 0; i < setClauses.size(); ++i) {
            if (i > 0) result += ", ";
            result += setClauses[i].toString();
        }
        if (whereClause) {
            result += " WHERE " + whereClause->toString();
        }
        return result;
    }
};

// ========== DELETE Node ==========

struct DeleteNode : ASTNode {
    std::string tableName;
    std::unique_ptr<WhereClauseNode> whereClause;
    
    DeleteNode(const std::string& tableName, size_t line = 0, size_t column = 0)
        : ASTNode(Type::DELETE, line, column), tableName(tableName) {}
    
    std::string toString() const override {
        std::string result = "DELETE FROM " + tableName;
        if (whereClause) {
            result += " WHERE " + whereClause->toString();
        }
        return result;
    }
};

// ========== Helper: Convert Token to AST Node ==========

class ASTFactory {
public:
    static std::unique_ptr<LiteralNode> createLiteral(const Token& token);
    static std::unique_ptr<IdentifierNode> createIdentifier(const Token& token);
    static std::unique_ptr<ColumnDefNode> createColumnDef(const std::string& name,
                                                          const std::string& type);
    static std::unique_ptr<WhereClauseNode> createWhereClause(const std::string& left,
                                                              const std::string& op,
                                                              const std::string& right);
};
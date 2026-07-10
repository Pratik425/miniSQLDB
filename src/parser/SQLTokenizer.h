#pragma once
#include <string>
#include <vector>
#include <unordered_map>

/**
 * @brief Token types for SQL parsing
 */
enum class TokenType {
    // Keywords
    SELECT,
    INSERT,
    UPDATE,
    DELETE,
    CREATE,
    DROP,
    TABLE,
    FROM,
    WHERE,
    INTO,
    VALUES,
    SET,
    PRIMARY,
    KEY,
    NOT,
    NULL_VALUE,   
    AND,
    OR,
    ORDER,
    BY,
    ASC,
    DESC,
    LIMIT,
    OFFSET,
    
    // Data types
    INT,
    VARCHAR,
    BOOLEAN,
    FLOAT,
    DATE,
    
    // Operators
    EQUALS,
    NOT_EQUALS,
    LESS_THAN,
    GREATER_THAN,
    LESS_EQUALS,
    GREATER_EQUALS,
    PLUS,
    MINUS,
    STAR,
    SLASH,
    
    // Special characters
    LPAREN,
    RPAREN,
    COMMA,
    SEMICOLON,
    DOT,
    
    // Identifiers and literals
    IDENTIFIER,
    STRING_LITERAL,
    NUMBER_LITERAL,
    FLOAT_LITERAL,
    BOOLEAN_LITERAL,
    STAR_SELECT,  // SELECT *
    
    // End of file
    EOF_TOKEN,
    UNKNOWN
};

/**
 * @brief Represents a single token
 */
struct Token {
    TokenType type;
    std::string value;
    size_t line;
    size_t column;
    
    Token(TokenType type = TokenType::UNKNOWN, 
          const std::string& value = "", 
          size_t line = 0, 
          size_t column = 0)
        : type(type), value(value), line(line), column(column) {}
    
    std::string toString() const;
    bool isKeyword() const;
    bool isOperator() const;
    bool isLiteral() const;
};

/**
 * @brief Tokenizes SQL queries into tokens
 * 
 * The SQLTokenizer breaks a SQL query string into
 * meaningful tokens for the parser to consume.
 */
class SQLTokenizer {
private:
    std::string input;
    size_t position;
    size_t line;
    size_t column;
    std::vector<Token> tokens;
    
    // Keyword lookup table
    static const std::unordered_map<std::string, TokenType> keywords;
    
public:
    /**
     * @brief Construct a new SQLTokenizer
     * 
     * @param query SQL query string
     */
    explicit SQLTokenizer(const std::string& query);
    
    /**
     * @brief Tokenize the entire query
     * 
     * @return std::vector<Token> List of tokens
     * @throws std::runtime_error on invalid syntax
     */
    std::vector<Token> tokenize();
    
    /**
     * @brief Get the next token (streaming)
     * 
     * @return Token The next token
     */
    Token nextToken();
    
    /**
     * @brief Check if there are more tokens
     */
    bool hasNext() const;
    
    /**
     * @brief Peek at the next token without consuming
     */
    Token peekToken();
    
    /**
     * @brief Get current position for error reporting
     */
    std::string getPosition() const;
    
private:
    /**
     * @brief Skip whitespace and comments
     */
    void skipWhitespaceAndComments();
    
    /**
     * @brief Read a number literal
     */
    Token readNumber();
    
    /**
     * @brief Read a string literal
     */
    Token readString();
    
    /**
     * @brief Read an identifier or keyword
     */
    Token readIdentifier();
    
    /**
     * @brief Read an operator or special character
     */
    Token readOperator();
    
    /**
     * @brief Check if a character is whitespace
     */
    bool isWhitespace(char c) const;
    
    /**
     * @brief Check if a character is a digit
     */
    bool isDigit(char c) const;
    
    /**
     * @brief Check if a character is a letter or underscore
     */
    bool isAlpha(char c) const;
    
    /**
     * @brief Check if a character is alphanumeric or underscore
     */
    bool isAlphaNumeric(char c) const;
    
    /**
     * @brief Convert string to keyword type
     */
    TokenType getKeywordType(const std::string& word) const;
    
    /**
     * @brief Get current character
     */
    char currentChar() const;
    
    /**
     * @brief Advance to next character
     */
    void advance();
    
    /**
     * @brief Report error with position
     */
    void error(const std::string& message) const;
};
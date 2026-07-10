#include "parser/SQLTokenizer.h"
#include <cctype>
#include <stdexcept>
#include <sstream>

// Initialize keyword map
const std::unordered_map<std::string, TokenType> SQLTokenizer::keywords = {
    {"SELECT", TokenType::SELECT},
    {"INSERT", TokenType::INSERT},
    {"UPDATE", TokenType::UPDATE},
    {"DELETE", TokenType::DELETE},
    {"CREATE", TokenType::CREATE},
    {"DROP", TokenType::DROP},
    {"TABLE", TokenType::TABLE},
    {"FROM", TokenType::FROM},
    {"WHERE", TokenType::WHERE},
    {"INTO", TokenType::INTO},
    {"VALUES", TokenType::VALUES},
    {"SET", TokenType::SET},
    {"PRIMARY", TokenType::PRIMARY},
    {"KEY", TokenType::KEY},
    {"NOT", TokenType::NOT},
    {"NULL", TokenType::NULL_VALUE},
    {"AND", TokenType::AND},
    {"OR", TokenType::OR},
    {"ORDER", TokenType::ORDER},
    {"BY", TokenType::BY},
    {"ASC", TokenType::ASC},
    {"DESC", TokenType::DESC},
    {"LIMIT", TokenType::LIMIT},
    {"OFFSET", TokenType::OFFSET},
    {"INT", TokenType::INT},
    {"VARCHAR", TokenType::VARCHAR},
    {"BOOLEAN", TokenType::BOOLEAN},
    {"FLOAT", TokenType::FLOAT},
    {"DATE", TokenType::DATE}
};

// ========== Constructor ==========

SQLTokenizer::SQLTokenizer(const std::string& query)
    : input(query), position(0), line(1), column(1) {}

// ========== Tokenization ==========

std::vector<Token> SQLTokenizer::tokenize() {
    tokens.clear();
    tokens.reserve(100);  // Pre-allocate memory
    
    while (hasNext()) {
        Token token = nextToken();
        if (token.type != TokenType::EOF_TOKEN) {
            tokens.push_back(token);
        }
    }
    
    return tokens;
}

Token SQLTokenizer::nextToken() {
    skipWhitespaceAndComments();
    
    // Check for end of input
    if (position >= input.length()) {
        return Token(TokenType::EOF_TOKEN, "", line, column);
    }
    
    char c = currentChar();
    
    // Numbers
    if (isDigit(c)) {
        return readNumber();
    }
    
    // Strings
    if (c == '\'' || c == '"') {
        return readString();
    }
    
    // Identifiers and keywords
    if (isAlpha(c) || c == '_') {
        return readIdentifier();
    }
    
    // Operators and special characters
    return readOperator();
}

bool SQLTokenizer::hasNext() const {
    return position < input.length();
}

Token SQLTokenizer::peekToken() {
    // Save state
    size_t savedPos = position;
    size_t savedLine = line;
    size_t savedCol = column;
    
    Token token = nextToken();
    
    // Restore state
    position = savedPos;
    line = savedLine;
    column = savedCol;
    
    return token;
}

std::string SQLTokenizer::getPosition() const {
    return "line " + std::to_string(line) + ", column " + std::to_string(column);
}

// ========== Private Methods ==========

void SQLTokenizer::skipWhitespaceAndComments() {
    while (position < input.length()) {
        char c = input[position];
        
        // Skip whitespace
        if (isWhitespace(c)) {
            if (c == '\n') {
                line++;
                column = 1;
            } else {
                column++;
            }
            position++;
            continue;
        }
        
        // Skip comments (-- single line)
        if (c == '-' && position + 1 < input.length() && input[position + 1] == '-') {
            // Skip to end of line
            while (position < input.length() && input[position] != '\n') {
                position++;
            }
            line++;
            column = 1;
            continue;
        }
        
        // Skip comments (/* multi-line */)
        if (c == '/' && position + 1 < input.length() && input[position + 1] == '*') {
            position += 2;  // Skip /*
            while (position < input.length()) {
                if (input[position] == '*' && position + 1 < input.length() && 
                    input[position + 1] == '/') {
                    position += 2;
                    break;
                }
                if (input[position] == '\n') {
                    line++;
                    column = 1;
                } else {
                    column++;
                }
                position++;
            }
            continue;
        }
        
        break;
    }
}

Token SQLTokenizer::readNumber() {
    std::string value;
    bool isFloat = false;
    size_t startLine = line;
    size_t startCol = column;
    
    while (position < input.length() && (isDigit(input[position]) || input[position] == '.')) {
        if (input[position] == '.') {
            isFloat = true;
        }
        value += input[position];
        column++;
        position++;
    }
    
    if (isFloat) {
        return Token(TokenType::FLOAT_LITERAL, value, startLine, startCol);
    } else {
        return Token(TokenType::NUMBER_LITERAL, value, startLine, startCol);
    }
}

Token SQLTokenizer::readString() {
    char quote = input[position];  // ' or "
    position++;
    column++;
    
    std::string value;
    size_t startLine = line;
    size_t startCol = column;
    
    while (position < input.length()) {
        char c = input[position];
        
        if (c == quote) {
            position++;
            column++;
            break;
        }
        
        if (c == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
        
        value += c;
        position++;
    }
    
    return Token(TokenType::STRING_LITERAL, value, startLine, startCol);
}

Token SQLTokenizer::readIdentifier() {
    std::string value;
    size_t startLine = line;
    size_t startCol = column;
    
    // Special case: * is a star selector (for SELECT *)
    if (currentChar() == '*') {
        position++;
        column++;
        return Token(TokenType::STAR_SELECT, "*", startLine, startCol);
    }
    
    while (position < input.length() && isAlphaNumeric(input[position])) {
        value += input[position];
        position++;
        column++;
    }
    
    // Check if it's a keyword
    TokenType type = getKeywordType(value);
    if (type != TokenType::UNKNOWN) {
        return Token(type, value, startLine, startCol);
    }
    
    return Token(TokenType::IDENTIFIER, value, startLine, startCol);
}

Token SQLTokenizer::readOperator() {
    char c = input[position];
    size_t startLine = line;
    size_t startCol = column;
    std::string value(1, c);
    TokenType type = TokenType::UNKNOWN;
    
    switch (c) {
        case '=':
            type = TokenType::EQUALS;
            break;
        case '!':
            if (position + 1 < input.length() && input[position + 1] == '=') {
                value = "!=";
                position++;
                column++;
                type = TokenType::NOT_EQUALS;
            } else {
                error("Unexpected character: '!'");
            }
            break;
        case '<':
            if (position + 1 < input.length() && input[position + 1] == '=') {
                value = "<=";
                position++;
                column++;
                type = TokenType::LESS_EQUALS;
            } else {
                type = TokenType::LESS_THAN;
            }
            break;
        case '>':
            if (position + 1 < input.length() && input[position + 1] == '=') {
                value = ">=";
                position++;
                column++;
                type = TokenType::GREATER_EQUALS;
            } else {
                type = TokenType::GREATER_THAN;
            }
            break;
        case '+':
            type = TokenType::PLUS;
            break;
        case '-':
            type = TokenType::MINUS;
            break;
        case '*':
            type = TokenType::STAR;
            break;
        case '/':
            type = TokenType::SLASH;
            break;
        case '(':
            type = TokenType::LPAREN;
            break;
        case ')':
            type = TokenType::RPAREN;
            break;
        case ',':
            type = TokenType::COMMA;
            break;
        case ';':
            type = TokenType::SEMICOLON;
            break;
        case '.':
            type = TokenType::DOT;
            break;
        default:
            error("Unexpected character: '" + std::string(1, c) + "'");
    }
    
    position++;
    column++;
    
    return Token(type, value, startLine, startCol);
}

bool SQLTokenizer::isWhitespace(char c) const {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

bool SQLTokenizer::isDigit(char c) const {
    return c >= '0' && c <= '9';
}

bool SQLTokenizer::isAlpha(char c) const {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool SQLTokenizer::isAlphaNumeric(char c) const {
    return isAlpha(c) || isDigit(c) || c == '_' || c == '*';
}

char SQLTokenizer::currentChar() const {
    if (position >= input.length()) {
        return '\0';
    }
    return input[position];
}

void SQLTokenizer::advance() {
    if (position < input.length()) {
        if (input[position] == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
        position++;
    }
}

TokenType SQLTokenizer::getKeywordType(const std::string& word) const {
    // Convert to uppercase for lookup
    std::string upper = word;
    for (char& c : upper) {
        c = std::toupper(c);
    }
    
    auto it = keywords.find(upper);
    if (it != keywords.end()) {
        return it->second;
    }
    
    return TokenType::UNKNOWN;
}

void SQLTokenizer::error(const std::string& message) const {
    throw std::runtime_error("Tokenizer error at " + getPosition() + ": " + message);
}

// ========== Token Implementation ==========

std::string Token::toString() const {
    std::string typeStr;
    switch (type) {
        case TokenType::SELECT: typeStr = "SELECT"; break;
        case TokenType::INSERT: typeStr = "INSERT"; break;
        case TokenType::UPDATE: typeStr = "UPDATE"; break;
        case TokenType::DELETE: typeStr = "DELETE"; break;
        case TokenType::CREATE: typeStr = "CREATE"; break;
        case TokenType::DROP: typeStr = "DROP"; break;
        case TokenType::TABLE: typeStr = "TABLE"; break;
        case TokenType::FROM: typeStr = "FROM"; break;
        case TokenType::WHERE: typeStr = "WHERE"; break;
        case TokenType::INTO: typeStr = "INTO"; break;
        case TokenType::VALUES: typeStr = "VALUES"; break;
        case TokenType::SET: typeStr = "SET"; break;
        case TokenType::PRIMARY: typeStr = "PRIMARY"; break;
        case TokenType::KEY: typeStr = "KEY"; break;
        case TokenType::NOT: typeStr = "NOT"; break;
        case TokenType::NULL_VALUE: typeStr = "NULL"; break;  // Changed
        case TokenType::AND: typeStr = "AND"; break;
        case TokenType::OR: typeStr = "OR"; break;
        case TokenType::IDENTIFIER: typeStr = "IDENTIFIER"; break;
        case TokenType::STRING_LITERAL: typeStr = "STRING"; break;
        case TokenType::NUMBER_LITERAL: typeStr = "NUMBER"; break;
        case TokenType::EOF_TOKEN: typeStr = "EOF"; break;
        default: typeStr = "UNKNOWN"; break;
    }
    return typeStr + "(" + value + ")";
}
bool Token::isKeyword() const {
    return type >= TokenType::SELECT && type <= TokenType::OFFSET;
}

bool Token::isOperator() const {
    return type >= TokenType::EQUALS && type <= TokenType::SLASH;
}

bool Token::isLiteral() const {
    return type == TokenType::STRING_LITERAL || 
           type == TokenType::NUMBER_LITERAL ||
           type == TokenType::FLOAT_LITERAL;
}
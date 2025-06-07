#include "lexer.h"
#include <cctype>
#include <unordered_map>
#include <iostream>

namespace MyCustomLang {

static const std::unordered_map<std::string, TokenType> keywords = {
    {"let", TokenType::LET},
    {"set", TokenType::SET},
    {"be", TokenType::BE},
    {"as", TokenType::AS},
    {"say", TokenType::SAY},
    {"when", TokenType::WHEN},
    {"then", TokenType::THEN},
    {"otherwise", TokenType::OTHERWISE},
    {"match", TokenType::MATCH},
    {"case", TokenType::CASE},
    {"repeat", TokenType::REPEAT},
    {"while", TokenType::WHILE},
    {"for", TokenType::FOR},
    {"from", TokenType::FROM},
    {"to", TokenType::TO},
    {"until", TokenType::UNTIL},
    {"step", TokenType::STEP},
    {"starting", TokenType::STARTING},
    {"in", TokenType::IN},
    {"at", TokenType::AT},
    {"define", TokenType::DEFINE},
    {"function", TokenType::FUNCTION},
    {"call", TokenType::CALL},
    {"return", TokenType::RETURN},
    {"throw", TokenType::THROW},
    {"end", TokenType::END},
    {"increase", TokenType::INCREASE},
    {"by", TokenType::BY},
    {"with", TokenType::WITH},
    {"create", TokenType::CREATE},
    {"model", TokenType::MODEL},
    {"try", TokenType::TRY},
    {"catch", TokenType::CATCH},
    {"open", TokenType::OPEN},
    {"file", TokenType::FILE},
    {"block", TokenType::BLOCK},
    {"Integer", TokenType::INTEGER},
    {"long", TokenType::LONG}
};

static const std::unordered_map<char, TokenType> singleCharTokens = {
    {'+', TokenType::PLUS},
    {'-', TokenType::MINUS},
    {'*', TokenType::STAR},
    {'/', TokenType::SLASH},
    {'=', TokenType::EQUAL},
    {'(', TokenType::LEFT_PAREN},
    {')', TokenType::RIGHT_PAREN},
    {'{', TokenType::LEFT_BRACE},
    {'}', TokenType::RIGHT_BRACE},
    {'[', TokenType::LEFT_BRACKET},
    {']', TokenType::RIGHT_BRACKET},
    {';', TokenType::SEMICOLON},
    {',', TokenType::COMMA},
    {':', TokenType::COLON}
};

Lexer::Lexer(const std::string& source)
    : source(source), current(0), line(1), indent_level(0), pendingDedents(0) {
    indent_stack.push_back(0);
    previousToken = Token(TokenType::UNKNOWN, "", 1); // Initialize previousToken
}

char Lexer::peek() const {
    return isAtEnd() ? '\0' : source[current];
}

char Lexer::peekNext() const {
    return current + 1 < source.length() ? source[current + 1] : '\0';
}

char Lexer::advance() {
    return isAtEnd() ? '\0' : source[current++];
}

bool Lexer::isAtEnd() const {
    return current >= source.length();
}

bool Lexer::isAlpha(char c) const {
    return std::isalpha(c) || c == '_';
}

bool Lexer::isDigit(char c) const {
    return std::isdigit(c);
}

bool Lexer::isAlphaNumeric(char c) const {
    return isAlpha(c) || isDigit(c);
}

Token Lexer::identifier() {
    std::string value;
    size_t start = current - 1;
    while (isAlphaNumeric(peek())) {
        if (value.size() >= MAX_TOKEN_LENGTH) {
            std::cerr << "Identifier too long at line " << line << std::endl;
            value = source.substr(start, current - start);
            Token token(TokenType::UNKNOWN, value, line);
            previousToken = token;
            return token;
        }
        value += advance();
    }
    auto keyword = keywords.find(value);
    Token token = (keyword != keywords.end()) ? 
        Token(keyword->second, value, line) : 
        Token(TokenType::IDENTIFIER, value, line);
    previousToken = token;
    return token;
}

Token Lexer::number() {
    std::string value;
    bool hasDecimal = false;
    size_t start = current - 1;
    while (isDigit(peek()) || peek() == '.') {
        char c = peek();
        if (c == '.') {
            if (hasDecimal) break;
            hasDecimal = true;
        }
        value += advance();
    }
    if (peek() == 'L') {
        value += advance();
    }
    Token token(TokenType::NUMBER, value, line);
    previousToken = token;
    return token;
}

Token Lexer::stringLiteral() {
    char quote = advance(); // Consume opening quote
    std::string value;
    int start_line = line;
    while (!isAtEnd() && peek() != quote) {
        if (peek() == '\n') line++;
        value += advance();
    }
    if (isAtEnd()) {
        std::cerr << "Unterminated string starting at line " << start_line << std::endl;
        Token token(TokenType::UNKNOWN, value, line);
        previousToken = token;
        return token;
    }
    advance(); // Consume closing quote
    Token token(TokenType::STRING, value, line);
    previousToken = token;
    return token;
}

Token Lexer::getNextToken() {
    // Handle pending dedents
    if (pendingDedents > 0) {
        pendingDedents--;
        indent_stack.pop_back();
        indent_level--;
        Token token(TokenType::DEDENT, "", line);
        previousToken = token;
        return token;
    }

    // Skip whitespace (spaces, tabs, carriage returns)
    while (!isAtEnd() && (peek() == ' ' || peek() == '\t' || peek() == '\r')) {
        advance();
    }

    // Handle end of file
    if (isAtEnd()) {
        if (indent_stack.size() > 1 && indent_stack.back() != 0) {
            indent_stack.pop_back();
            indent_level--;
            Token token(TokenType::DEDENT, "", line);
            previousToken = token;
            return token;
        }
        Token token(TokenType::END_OF_FILE, "", line);
        previousToken = token;
        return token;
    }

    char c = peek();

    // Handle newlines and indentation
    if (c == '\n') {
        advance();
        line++;
        size_t new_indent = current;
        while (peek() == ' ' || peek() == '\t') {
            advance();
        }
        if (peek() == '\n' || peek() == '#' || isAtEnd()) {
            if (peek() == '#') {
                while (!isAtEnd() && peek() != '\n') {
                    advance();
                }
            }
            current = new_indent;
            return getNextToken();
        }
        int indent_count = static_cast<int>(current - new_indent);
        current = new_indent;
        if (indent_count > indent_stack.back()) {
            indent_stack.push_back(indent_count);
            indent_level++;
            Token token(TokenType::INDENT, "", line);
            previousToken = token;
            return token;
        } else if (indent_count < indent_stack.back()) {
            indent_stack.pop_back();
            indent_level--;
            pendingDedents = 0;
            while (indent_count < indent_stack.back() && indent_stack.size() > 1) {
                indent_stack.pop_back();
                indent_level--;
                pendingDedents++;
            }
            Token token(TokenType::DEDENT, "", line);
            previousToken = token;
            return token;
        } else {
            if (!indent_stack.empty() && indent_count > 0 &&
                (previousToken.type == TokenType::CATCH || previousToken.type == TokenType::THEN)) {
                indent_stack.push_back(indent_count);
                indent_level++;
                Token token(TokenType::INDENT, "", line);
                previousToken = token;
                return token;
            }
            return getNextToken();
        }
    }

    // Handle multiline comments
    if (c == '/' && peekNext() == '*') {
        advance();
        advance();
        int start_line = line;
        while (!isAtEnd()) {
            if (peek() == '*' && peekNext() == '/') {
                advance();
                advance();
                return getNextToken();
            }
            if (peek() == '\n') line++;
            advance();
        }
        std::cerr << "Unterminated multiline comment starting at line " << start_line << std::endl;
        Token token(TokenType::UNKNOWN, "", line);
        previousToken = token;
        return token;
    }

    // Handle single-line comments
    if (c == '#') {
        while (!isAtEnd() && peek() != '\n') {
            advance();
        }
        return getNextToken();
    }

    // Handle strings
    if (c == '"' || c == '\'') {
        return stringLiteral();
    }

    // Handle numbers
    if (isDigit(c)) {
        return number();
    }

    // Handle identifiers and keywords
    if (isAlpha(c)) {
        return identifier();
    }

    // Handle operators and punctuation
    auto singleChar = singleCharTokens.find(c);
    if (singleChar != singleCharTokens.end()) {
        std::string lexeme(1, advance());
        Token token(singleChar->second, lexeme, line);
        previousToken = token;
        return token;
    }

    // Handle multi-character operators
    if (c == '>' && peekNext() == '=') {
        advance();
        advance();
        Token token(TokenType::GREATER_EQUAL, ">=", line);
        previousToken = token;
        return token;
    }
    if (c == '<' && peekNext() == '=') {
        advance();
        advance();
        Token token(TokenType::LESS_EQUAL, "<=", line);
        previousToken = token;
        return token;
    }
    if (c == '!' && peekNext() == '=') {
        advance();
        advance();
        Token token(TokenType::NOT_EQUAL, "!=", line);
        previousToken = token;
        return token;
    }
    if (c == '=' && peekNext() == '=') {
        advance();
        advance();
        Token token(TokenType::EQUAL_EQUAL, "==", line);
        previousToken = token;
        return token;
    }
    if (c == '=') {
        advance();
        Token token(TokenType::EQUAL, "=", line);
        previousToken = token;
        return token;
    }
    if (c == '>') {
        advance();
        Token token(TokenType::GREATER, ">", line);
        previousToken = token;
        return token;
    }
    if (c == '<') {
        advance();
        Token token(TokenType::LESS, "<", line);
        previousToken = token;
        return token;
    }

    // Unknown character
    std::string lexeme(1, advance());
    std::cerr << "Unknown character '" << lexeme << "' at line " << line << std::endl;
    Token token(TokenType::UNKNOWN, lexeme, line);
    previousToken = token;
    return token;
}

} // namespace MyCustomLang
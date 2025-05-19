#include "../include/Lexer.h"
#include <cctype>
#include <unordered_map>
#include <iostream>

namespace MyCustomLang {

static const int MAX_TOKEN_LENGTH = 256;

// Mapping of keywords to their TokenType
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
    {"file", TokenType::FILE}
};

// Single-character tokens: operators and punctuation
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
    {',', TokenType::COMMA}
};

Lexer::Lexer(std::string src) 
    : source(std::move(src)), current(0), line(1), indent_level(0) {
    indent_stack.push_back(0);
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

Token Lexer::getNextToken() {
    while (!isAtEnd()) {
        char c = advance();

        // Handle whitespace (outside indentation)
        if (c == ' ' || c == '\t') continue;

        // Handle newlines and indentation
        if (c == '\n') {
            line++;
            size_t temp_current = current;
            int spaces = 0;
            while (!isAtEnd() && (peek() == ' ' || peek() == '\t')) {
                char ws = advance();
                spaces += (ws == '\t' ? 4 : 1);
            }
            // Emit NEWLINE for the previous line
            if (!isAtEnd() && peek() != '\n' && peek() != '#') {
                int current_indent = indent_stack.back();
                if (spaces > current_indent) {
                    indent_stack.push_back(spaces);
                    indent_level++;
                    return Token(TokenType::INDENT, "", line);
                } else if (spaces < current_indent) {
                    indent_stack.pop_back();
                    if (spaces != indent_stack.back()) {
                        std::cerr << "Indentation error at line " << line << ": expected " 
                                  << indent_stack.back() << " spaces, got " << spaces << std::endl;
                        return Token(TokenType::UNKNOWN, "", line);
                    }
                    return Token(TokenType::DEDENT, "", line);
                }
            }
            current = temp_current;
            return Token(TokenType::NEWLINE, "", line - 1);
        }

        // Handle comments
        if (c == '#') {
            while (!isAtEnd() && peek() != '\n') advance();
            continue;
        }
        if (c == '/' && peek() == '/') {
            while (!isAtEnd() && peek() != '\n') advance();
            continue;
        }
        if (c == '/' && peek() == '*') {
            advance(); // Consume '*'
            while (!isAtEnd()) {
                if (peek() == '*' && peekNext() == '/') {
                    advance(); // Consume '*'
                    advance(); // Consume '/'
                    break;
                }
                if (peek() == '\n') line++;
                advance();
            }
            if (isAtEnd()) {
                std::cerr << "Unterminated multi-line comment at line " << line << std::endl;
                return Token(TokenType::UNKNOWN, "", line);
            }
            continue;
        }

        // Operators and punctuation
        if (c == '<') return Token(TokenType::LESS, "<", line);
        if (c == '>') return Token(TokenType::GREATER, ">", line);
        if (c == '_') return Token(TokenType::UNDERSCORE, "_", line);

        auto op = singleCharTokens.find(c);
        if (op != singleCharTokens.end()) {
            return Token(op->second, std::string(1, c), line);
        }

        // Identifiers and keywords
        if (isAlpha(c)) {
            current--; // Backtrack to read full identifier
            return identifier();
        }

        // Numbers
        if (isDigit(c)) {
            current--; // Backtrack to read full number
            return number();
        }

        // Strings
        if (c == '"') {
            return stringLiteral();
        }

        // Unknown character
        std::cerr << "Unknown character '" << c << "' at line " << line << std::endl;
        return Token(TokenType::UNKNOWN, std::string(1, c), line);
    }

    // Emit DEDENT tokens to close all open indentation levels at EOF
    if (indent_stack.size() > 1) {
        indent_stack.pop_back();
        return Token(TokenType::DEDENT, "", line);
    }

    return Token(TokenType::END_OF_FILE, "", line);
}

Token Lexer::identifier() {
    std::string value;
    while (isAlphaNumeric(peek())) {
        if (value.size() >= MAX_TOKEN_LENGTH) {
            std::cerr << "Identifier too long at line " << line << std::endl;
            return Token(TokenType::UNKNOWN, value, line);
        }
        value += advance();
    }

    auto keyword = keywords.find(value);
    if (keyword != keywords.end()) {
        return Token(keyword->second, value, line);
    }
    return Token(TokenType::IDENTIFIER, value, line);
}

Token Lexer::number() {
    std::string value;
    bool hasDecimal = false;

    while (isDigit(peek()) || peek() == '.') {
        char c = peek();
        if (c == '.') {
            if (hasDecimal) break; // Only one decimal point
            hasDecimal = true;
        }
        value += advance();
    }

    return Token(TokenType::NUMBER, value, line);
}

Token Lexer::stringLiteral() {
    std::string value;
    size_t start_line = line;
    while (!isAtEnd() && peek() != '"') {
        if (peek() == '\n') line++;
        value += advance();
    }

    if (isAtEnd()) {
        std::cerr << "Unterminated string starting at line " << start_line << std::endl;
        return Token(TokenType::UNKNOWN, value, line);
    }

    advance(); // Consume closing quote
    return Token(TokenType::STRING, value, line);
}

} // namespace MyCustomLang
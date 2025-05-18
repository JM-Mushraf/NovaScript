#include "../include/Lexer.h"
#include <unordered_map>

using namespace std;
using MyCustomLang::Token;
using MyCustomLang::TokenType;

// Mapping of keywords to their TokenType (unchanged from your version, duplicates removed)
static const unordered_map<string, TokenType> keywords = {
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
static const unordered_map<char, TokenType> singleCharTokens = {
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

Lexer::Lexer(const string& src) : source(src), current(0), line(1), indent_stack({0}) {
    // Initialize indent_stack with 0 (no indentation)
}

char Lexer::peek() const {
    return isAtEnd() ? '\0' : source[current];
}

char Lexer::advance() {
    return isAtEnd() ? '\0' : source[current++];
}

bool Lexer::isAtEnd() const {
    return current >= source.size();
}

Token Lexer::getNextToken() {
    while (!isAtEnd()) {
        char c = advance();

        // Handle comments
        if (c == '/' && peek() == '/') {
            while (!isAtEnd() && peek() != '\n') advance();
            continue;
        }
        if (c == '/' && peek() == '*') {
            advance(); // Consume '*'
            while (!isAtEnd()) {
                if (peek() == '*' && source[current + 1] == '/') {
                    advance(); // Consume '*'
                    advance(); // Consume '/'
                    break;
                }
                if (peek() == '\n') line++;
                advance();
            }
            continue;
        }

        // Handle newlines and indentation
        if (c == '\n') {
            line++;
            // Check indentation at the start of the next line
            int indent_level = 0;
            while (!isAtEnd() && (peek() == ' ' || peek() == '\t')) {
                char ws = advance();
                // Assume 1 tab = 4 spaces for simplicity (adjust as needed)
                indent_level += (ws == '\t' ? 4 : 1);
            }

            // Compare with current indentation level
            int current_indent = indent_stack.back();
            if (indent_level > current_indent) {
                indent_stack.push_back(indent_level);
                return Token(TokenType::INDENT, "", line);
            } else if (indent_level < current_indent) {
                indent_stack.pop_back();
                // Emit DEDENT for each level decrease
                if (indent_level != indent_stack.back()) {
                    cerr << "Indentation error at line " << line << endl;
                    return Token(TokenType::UNKNOWN, "", line);
                }
                return Token(TokenType::DEDENT, "", line);
            }
            // If indentation is the same, return NEWLINE
            return Token(TokenType::NEWLINE, "", line);
        }

        // Skip other whitespace (outside indentation)
        if (isspace(c)) continue;

        // Operators and punctuation
        if (c == '<') return Token(TokenType::LESS, "<", line);
        if (c == '>') return Token(TokenType::GREATER, ">", line);
        if (c == '_') return Token(TokenType::UNDERSCORE, "_", line);

        auto op = singleCharTokens.find(c);
        if (op != singleCharTokens.end()) {
            return Token(op->second, string(1, c), line);
        }

        // Identifiers and keywords
        if (isalpha(c) || c == '_') {
            current--; // Backtrack to read full identifier
            return identifier();
        }

        // Numbers
        if (isdigit(c)) {
            current--; // Backtrack to read full number
            return number();
        }

        // Strings
        if (c == '"') {
            return stringLiteral();
        }

        // Unknown character
        cerr << "Unknown character '" << c << "' at line " << line << endl;
        return Token(TokenType::UNKNOWN, string(1, c), line);
    }

    // Emit DEDENT tokens to close all open indentation levels at EOF
    if (indent_stack.size() > 1) {
        indent_stack.pop_back();
        return Token(TokenType::DEDENT, "", line);
    }

    return Token(TokenType::END_OF_FILE, "", line);
}

// Update identifier to use line member
Token Lexer::identifier() {
    string value;
    while (isalnum(peek()) || peek() == '_') {
        if (value.size() >= MAX_TOKEN_LENGTH) {
            cerr << "Identifier too long at line " << line << endl;
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

// Update number to use line member and handle decimals
Token Lexer::number() {
    string value;
    bool hasDecimal = false;

    while (isdigit(peek()) || peek() == '.') {
        char c = peek();
        if (c == '.') {
            if (hasDecimal) break; // Only one decimal point
            hasDecimal = true;
        }
        value += advance();
    }

    return Token(TokenType::NUMBER, value, line);
}

// Update stringLiteral to use line member
Token Lexer::stringLiteral() {
    string value;
    while (!isAtEnd() && peek() != '"') {
        if (peek() == '\n') line++;
        value += advance();
    }

    if (isAtEnd()) {
        cerr << "Unterminated string at line " << line << endl;
        return Token(TokenType::UNKNOWN, value, line);
    }

    advance(); // Consume closing quote
    return Token(TokenType::STRING, value, line);
}
#include "Lexer.h"
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
            if (hasDecimal) break;
            hasDecimal = true;
        }
        value += advance();
    }

    if (peek() == 'L') {
        value += advance();
    }

    return Token(TokenType::NUMBER, value, line);
}

Token Lexer::stringLiteral() {
    char quote = advance();
    std::string value;
    int start_line = line;

    while (!isAtEnd() && peek() != quote) {
        if (peek() == '\n') line++;
        value += advance();
    }

    if (isAtEnd()) {
        std::cerr << "Unterminated string starting at line " << start_line << std::endl;
        return Token(TokenType::UNKNOWN, value, line);
    }

    advance();
    return Token(TokenType::STRING, value, line);
}

Token Lexer::getNextToken() {
    // Skip whitespace (spaces, tabs, carriage returns)
    while (!isAtEnd()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r') {
            advance();
        } else {
            break;
        }
    }

    // Handle pending dedents
    if (pendingDedents > 0) {
        pendingDedents--;
        indent_stack.pop_back();
        indent_level--;
        return Token(TokenType::DEDENT, "", line);
    }

    if (isAtEnd()) {
        // Only emit dedents if stack has unclosed levels
        if (indent_stack.size() > 1 && indent_stack.back() != 0) {
            indent_stack.pop_back();
            indent_level--;
            return Token(TokenType::DEDENT, "", line);
        }
        return Token(TokenType::END_OF_FILE, "", line);
    }

    char c = peek();

    // Handle newlines and indentation
    if (c == '\n') {
        advance();
        line++;
        // Skip whitespace after newline to check indentation
        size_t new_indent = current;
        while (peek() == ' ' || peek() == '\t') {
            advance();
        }
        int indent_count = static_cast<int>(current - new_indent);

        // Skip empty lines or lines with only comments
        if (peek() == '\n' || peek() == '#' || isAtEnd()) {
            if (peek() == '#') {
                while (!isAtEnd() && peek() != '\n') {
                    advance();
                }
            }
            return getNextToken();
        }

        // Handle indentation
        if (indent_count > indent_stack.back()) {
            indent_stack.push_back(indent_count);
            indent_level++;
            return Token(TokenType::INDENT, "", line);
        } else if (indent_count < indent_stack.back()) {
            // Pop stack until indent_count matches or is greater
            while (!indent_stack.empty() && indent_count < indent_stack.back()) {
                indent_stack.pop_back();
                indent_level--;
                return Token(TokenType::DEDENT, "", line);
            }
            // If indent_count matches an earlier level, no dedent needed
            if (!indent_stack.empty() && indent_count == indent_stack.back()) {
                return getNextToken();
            }
            // Invalid indentation
            std::cerr << "Invalid indentation at line " << line << std::endl;
            return Token(TokenType::UNKNOWN, "", line);
        }
        return getNextToken();
    }

    // Handle comments
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
        return Token(singleChar->second, lexeme, line);
    }

    // Handle multi-character operators
    if (c == '>' && peekNext() == '=') {
        advance(); advance();
        return Token(TokenType::GREATER_EQUAL, ">=", line);
    }
    if (c == '<' && peekNext() == '=') {
        advance(); advance();
        return Token(TokenType::LESS_EQUAL, "<=", line);
    }
    if (c == '!' && peekNext() == '=') {
        advance(); advance();
        return Token(TokenType::NOT_EQUAL, "!=", line);
    }
    if (c == '=' && peekNext() == '=') {
        advance(); advance();
        return Token(TokenType::EQUAL_EQUAL, "==", line);
    }
    if (c == '>') {
        advance();
        return Token(TokenType::GREATER, ">", line);
    }
    if (c == '<') {
        advance();
        return Token(TokenType::LESS, "<", line);
    }

    // Unknown character
    std::string lexeme(1, advance());
    std::cerr << "Unknown character '" << lexeme << "' at line " << line << std::endl;
    return Token(TokenType::UNKNOWN, lexeme, line);
}

} // namespace MyCustomLang
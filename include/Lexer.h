#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include "Token.h"

namespace MyCustomLang {

class Lexer {
public:
    explicit Lexer(const std::string& source);
    Token getNextToken();

private:
    std::string source;
    size_t current;
    int line;
    int indent_level;
    std::vector<int> indent_stack;
    int pendingDedents;

    static const size_t MAX_TOKEN_LENGTH = 256;

    char peek() const;
    char peekNext() const;
    char advance();
    bool isAtEnd() const;
    bool isAlpha(char c) const;
    bool isDigit(char c) const;
    bool isAlphaNumeric(char c) const;
    Token identifier();
    Token number();
    Token stringLiteral();
};

} // namespace MyCustomLang

#endif
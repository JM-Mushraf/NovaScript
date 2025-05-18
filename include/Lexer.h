#ifndef LEXER_H
#define LEXER_H

#include "Token.h"
#include "Common.h"
#include <string>
#include <vector>

class Lexer {
public:
    explicit Lexer(const std::string& source);
    MyCustomLang::Token getNextToken();

private:
    std::string source;
    size_t current;
    int line; // Track line number
    std::vector<int> indent_stack; // Track indentation levels
    char peek() const;
    char advance();
    bool isAtEnd() const;
    MyCustomLang::Token identifier();
    MyCustomLang::Token number();
    MyCustomLang::Token stringLiteral();
};

#endif
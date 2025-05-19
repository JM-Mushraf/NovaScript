#ifndef PARSER_ERROR_H
#define PARSERANDE

#include <string>
#include "Token.h"

namespace MyCustomLang {

class ParserError : public std::exception {
public:
    ParserError(const Token& token, std::string message)
        : msg("Parse error at line " + std::to_string(token.line) + ": " + message +
              " (token: " + (token.lexeme.empty() ? tokenTypeToString(token.type) : token.lexeme) + ")") {}
    const char* what() const noexcept override { return msg.c_str(); }
private:
    std::string msg;
};

} // namespace MyCustomLang

#endif
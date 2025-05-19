#ifndef PARSE_ERROR_H
#define PARSE_ERROR_H

#include <stdexcept>
#include <string>
#include "Token.h"

namespace MyCustomLang {

class ParserError : public std::runtime_error {
public:
    Token token;
    explicit ParserError(const Token& t, const std::string& message)
        : std::runtime_error(
              "Parse error at line " + std::to_string(t.line) + ": " + message +
              " (token: " + (t.lexeme.empty() ? tokenTypeToString(t.type) : t.lexeme) + ")"),
          token(t) {}
};

} // namespace MyCustomLang

#endif
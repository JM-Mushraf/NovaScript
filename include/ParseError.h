#ifndef PARSER_ERROR_H
#define PARSER_ERROR_H

#include <stdexcept>
#include <string>
#include "Token.h"

// Custom exception class for parsing errors.
// Thrown when the parser encounters invalid syntax, including the token
// that caused the error and its line number for better error reporting.

namespace MyCustomLang {

class ParserError : public std::runtime_error {
public:
    ParserError(const Token& token, const std::string& message)
        : std::runtime_error(
              "Parse error at line " + std::to_string(token.line) +
              ": " + message + " (token: " + token.lexeme + ")") {}
};

} // namespace MyCustomLang

#endif
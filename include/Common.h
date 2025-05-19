#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cctype>
#include <memory>

// Note: MAX_TOKEN_LENGTH is defined in Lexer.h as a static const member (Lexer::MAX_TOKEN_LENGTH)
// to avoid macro redefinition. Use Lexer::MAX_TOKEN_LENGTH where needed.

namespace MyCustomLang {

// Forward declarations for Expr and Stmt (used in AST.h)
class Expr;
class Stmt;

// Type aliases for smart pointers (guarded to avoid视察, included in `AST.h` to prevent redefinition
#ifndef AST_H
using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;
#endif

} // namespace MyCustomLang

#endif
#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "SymbolTable.h"
#include "Type.h" // Include Type.h for Type enum
#include <stdexcept>
#include <string>

// Forward declarations for Program, Stmt, and Expr
namespace MyCustomLang {
class Program;
class Stmt;
class Expr;
}

// Include AST.h after forward declarations to avoid circular dependency
#include "AST.h"

namespace MyCustomLang {

// Semantic error exception
class SemanticError : public std::runtime_error {
public:
    SemanticError(const Token& token, const std::string& message)
        : std::runtime_error("Semantic error at line " + std::to_string(token.line) + ": " + message) {}
};

// Semantic analyzer class
class SemanticAnalyzer {
public:
    SemanticAnalyzer(SymbolTable& symTable) : symbolTable(symTable) {}
    void analyze(Program& program);

private:
    SymbolTable& symbolTable;

    void analyzeStmt(Stmt* stmt);
    void analyzeExpr(Expr* expr);
    Type inferExprType(Expr* expr);
    void checkTypeCompatibility(Type expected, Type actual, const Token& token);
    void updateFunctionReturnType(const std::string& funcName, Type returnType);
};

} // namespace MyCustomLang

#endif // SEMANTIC_ANALYZER_H
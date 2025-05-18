#ifndef AST_H
#define AST_H

#include <memory>
#include <string>
#include <variant>
#include <vector>
#include "Token.h"

// Abstract Syntax Tree (AST) node definitions for the parser.
// The AST represents the syntactic structure of the program, with nodes for
// expressions (e.g., numbers, binary operations) and statements (e.g., variable
// declarations, when-then statements). We use std::variant for polymorphic node
// types and std::unique_ptr for memory management.

namespace MyCustomLang {

// Forward declarations for AST node types
struct Expr;
struct Stmt;

// Alias for unique_ptr to AST nodes
using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;

// --- Expression Nodes ---

// Base struct for expressions, used for polymorphic inheritance
struct Expr {
    virtual ~Expr() = default;
};

// Literal expression (e.g., number "42", string "Hello")
struct LiteralExpr : Expr {
    Token value; // Token containing the literal value (NUMBER, STRING)
    LiteralExpr(Token v) : value(std::move(v)) {}
};

// Variable reference (e.g., identifier "x")
struct VariableExpr : Expr {
    Token name; // Token containing the identifier (IDENTIFIER)
    VariableExpr(Token n) : name(std::move(n)) {}
};

// Binary operation (e.g., "x + y", "x > y")
struct BinaryExpr : Expr {
    ExprPtr left; // Left operand
    Token op;     // Operator (PLUS, MINUS, GREATER, etc.)
    ExprPtr right;// Right operand
    BinaryExpr(ExprPtr l, Token o, ExprPtr r)
        : left(std::move(l)), op(std::move(o)), right(std::move(r)) {}
};

// Parenthesized expression (e.g., "(x + y)")
struct ParenExpr : Expr {
    ExprPtr expr; // Inner expression
    ParenExpr(ExprPtr e) : expr(std::move(e)) {}
};

// --- Statement Nodes ---

// Base struct for statements
struct Stmt {
    virtual ~Stmt() = default;
};

// Variable declaration (e.g., "let x = 42")
struct VarDeclStmt : Stmt {
    Token name;   // Identifier (IDENTIFIER)
    ExprPtr init; // Initializer expression
    VarDeclStmt(Token n, ExprPtr i) : name(std::move(n)), init(std::move(i)) {}
};

// Say statement (e.g., "say x")
struct SayStmt : Stmt {
    ExprPtr expr; // Expression to output
    SayStmt(ExprPtr e) : expr(std::move(e)) {}
};

// When-then-otherwise statement
struct WhenStmt : Stmt {
    struct Branch {
        ExprPtr condition;      // Condition expression (null for final "otherwise")
        std::vector<StmtPtr> body; // List of statements in the branch
        Branch(ExprPtr c, std::vector<StmtPtr> b)
            : condition(std::move(c)), body(std::move(b)) {}
    };
    std::vector<Branch> branches; // List of branches (when/otherwise)
    WhenStmt(std::vector<Branch> b) : branches(std::move(b)) {}
};

// Program (top-level AST node)
struct Program {
    std::vector<StmtPtr> statements; // List of statements
    Program(std::vector<StmtPtr> s) : statements(std::move(s)) {}
};

} // namespace MyCustomLang

#endif
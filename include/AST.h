#ifndef AST_H
#define AST_H

#include <vector>
#include <memory>
#include "Token.h"
#include "Common.h"

namespace MyCustomLang {

class Expr {
public:
    virtual ~Expr() = default;
};

class LiteralExpr : public Expr {
public:
    Token value;
    explicit LiteralExpr(Token v) : value(std::move(v)) {}
};

class VariableExpr : public Expr {
public:
    Token name;
    explicit VariableExpr(Token n) : name(std::move(n)) {}
};

class BinaryExpr : public Expr {
public:
    ExprPtr left;
    Token op;
    ExprPtr right;
    BinaryExpr(ExprPtr l, Token o, ExprPtr r)
        : left(std::move(l)), op(std::move(o)), right(std::move(r)) {}
};

class ParenExpr : public Expr {
public:
    ExprPtr expr;
    explicit ParenExpr(ExprPtr e) : expr(std::move(e)) {}
};

class ListLiteralExpr : public Expr {
public:
    std::vector<ExprPtr> elements;
    explicit ListLiteralExpr(std::vector<ExprPtr> e) : elements(std::move(e)) {}
};

class DictLiteralExpr : public Expr {
public:
    std::vector<std::pair<ExprPtr, ExprPtr>> pairs;
    explicit DictLiteralExpr(std::vector<std::pair<ExprPtr, ExprPtr>> p) : pairs(std::move(p)) {}
};

class Stmt {
public:
    virtual ~Stmt() = default;
};

class VarDeclStmt : public Stmt {
public:
    Token name;
    ExprPtr init;
    VarDeclStmt(Token n, ExprPtr i) : name(std::move(n)), init(std::move(i)) {}
};

class AssignmentStmt : public Stmt {
public:
    Token name;
    ExprPtr value;
    AssignmentStmt(Token n, ExprPtr v) : name(std::move(n)), value(std::move(v)) {}
};

class WhenStmt : public Stmt {
public:
    struct Branch {
        ExprPtr condition; // nullptr for 'otherwise' without condition
        std::vector<StmtPtr> body;
        Branch(ExprPtr c, std::vector<StmtPtr> b) : condition(std::move(c)), body(std::move(b)) {}
    };
    std::vector<Branch> branches;
    explicit WhenStmt(std::vector<Branch> b) : branches(std::move(b)) {}
};

class SayStmt : public Stmt {
public:
    ExprPtr expr;
    explicit SayStmt(ExprPtr e) : expr(std::move(e)) {}
};

class Program {
public:
    std::vector<StmtPtr> statements;
    explicit Program(std::vector<StmtPtr> s) : statements(std::move(s)) {}
};

} // namespace MyCustomLang

#endif
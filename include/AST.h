#ifndef MYCUSTOMLANG_AST_H
#define MYCUSTOMLANG_AST_H

#include "Token.h"
#include <memory>
#include <vector>
#include <string>

namespace MyCustomLang {

class Expr;
class Stmt;

using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;

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
    std::vector<std::pair<ExprPtr, ExprPtr>> entries;
    explicit DictLiteralExpr(std::vector<std::pair<ExprPtr, ExprPtr>> e) : entries(std::move(e)) {}
};

class IndexExpr : public Expr {
public:
    ExprPtr base;
    ExprPtr index;
    IndexExpr(ExprPtr b, ExprPtr i) : base(std::move(b)), index(std::move(i)) {}
};

class AssignExpr : public Expr {
public:
    Token name;
    ExprPtr value;
    AssignExpr(Token n, ExprPtr v) : name(std::move(n)), value(std::move(v)) {}
};

class IndexAssignExpr : public Expr {
public:
    ExprPtr target;
    ExprPtr value;
    IndexAssignExpr(ExprPtr t, ExprPtr v) : target(std::move(t)), value(std::move(v)) {}
};

class Stmt {
public:
    virtual ~Stmt() = default;
};

class VarDeclStmt : public Stmt {
public:
    Token name;
    ExprPtr init;
    Token typeHint;
    bool isLong;
    VarDeclStmt(Token n, ExprPtr i, Token t = Token(TokenType::NONE, "", 0), bool l = false)
        : name(std::move(n)), init(std::move(i)), typeHint(std::move(t)), isLong(l) {}
};

class IndexAssignStmt : public Stmt {
public:
    ExprPtr target;
    ExprPtr value;
    IndexAssignStmt(ExprPtr t, ExprPtr v) : target(std::move(t)), value(std::move(v)) {}
};

class SayStmt : public Stmt {
public:
    ExprPtr expr;
    explicit SayStmt(ExprPtr e) : expr(std::move(e)) {}
};

class WhenStmt : public Stmt {
public:
    struct Branch {
        ExprPtr condition; // nullptr for 'otherwise' clauses
        std::vector<StmtPtr> body;
        Branch(ExprPtr c, std::vector<StmtPtr> b)
            : condition(std::move(c)), body(std::move(b)) {}
    };
    std::vector<Branch> branches;
    explicit WhenStmt(std::vector<Branch> b) : branches(std::move(b)) {}
};

class MatchStmt : public Stmt {
public:
    struct Case {
        ExprPtr pattern;
        std::vector<StmtPtr> body;
        Case(ExprPtr p, std::vector<StmtPtr> b)
            : pattern(std::move(p)), body(std::move(b)) {}
    };
    ExprPtr condition;
    std::vector<Case> cases;
    MatchStmt(ExprPtr c, std::vector<Case> cs)
        : condition(std::move(c)), cases(std::move(cs)) {}
};

class WhileStmt : public Stmt {
public:
    ExprPtr condition;
    std::vector<StmtPtr> body;
    WhileStmt(ExprPtr c, std::vector<StmtPtr> b)
        : condition(std::move(c)), body(std::move(b)) {}
};

class ForStmt : public Stmt {
public:
    Token iterator;
    ExprPtr start;
    ExprPtr end;
    ExprPtr step;
    std::vector<StmtPtr> body;
    ForStmt(Token i, ExprPtr s, ExprPtr e, ExprPtr st, std::vector<StmtPtr> b)
        : iterator(std::move(i)), start(std::move(s)), end(std::move(e)),
          step(std::move(st)), body(std::move(b)) {}
};

class WithStmt : public Stmt {
public:
    Token iterator;
    ExprPtr start;
    ExprPtr end;
    ExprPtr step;
    std::vector<StmtPtr> body;
    WithStmt(Token i, ExprPtr s, ExprPtr e, ExprPtr st, std::vector<StmtPtr> b)
        : iterator(std::move(i)), start(std::move(s)), end(std::move(e)),
          step(std::move(st)), body(std::move(b)) {}
};

class Program {
public:
    std::vector<StmtPtr> statements;
    explicit Program(std::vector<StmtPtr> s) : statements(std::move(s)) {}
};

} // namespace MyCustomLang

#endif // MYCUSTOMLANG_AST_H
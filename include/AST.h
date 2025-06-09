#ifndef MYCUSTOMLANG_AST_H
#define MYCUSTOMLANG_AST_H

#include "Token.h"
#include "Type.h" // Include Type.h for Type enum
#include <memory>
#include <vector>
#include <string>
#include <ostream>

namespace MyCustomLang {

class Expr;
class Stmt;

using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;

// Helper function for indentation
namespace {
    void printIndent(std::ostream& os, int indent) {
        os << std::string(indent * 2, ' ');
    }
}

class Expr {
public:
    virtual ~Expr() = default;
    virtual void print(std::ostream& os, int indent) const = 0;
    virtual Token getToken() const = 0; // Added for semantic analyzer error reporting
    Type inferredType = Type::NONE; // Added for type inference
};

class LiteralExpr : public Expr {
public:
    Token value;
    Type inferredType = Type::NONE; // Added

    explicit LiteralExpr(Token v) : value(std::move(v)) {}
    void print(std::ostream& os, int indent) const override {
        printIndent(os, indent);
        os << "LiteralExpr: " << value.lexeme << " (" << tokenTypeToString(value.type) << ")\n";
    }
    Token getToken() const override { return value; } // Added
private:
    static std::string tokenTypeToString(TokenType type) {
        switch (type) {
            case TokenType::NUMBER: return "NUMBER";
            case TokenType::STRING: return "STRING";
            default: return "UNKNOWN";
        }
    }
};

class VariableExpr : public Expr {
public:
    Token name;
    Type inferredType = Type::NONE; // Added

    explicit VariableExpr(Token n) : name(std::move(n)) {}
    void print(std::ostream& os, int indent) const override {
        printIndent(os, indent);
        os << "VariableExpr: " << name.lexeme << "\n";
    }
    Token getToken() const override { return name; } // Added
};

class BinaryExpr : public Expr {
public:
    ExprPtr left;
    Token op;
    ExprPtr right;
    Type inferredType = Type::NONE; // Added

    BinaryExpr(ExprPtr l, Token o, ExprPtr r)
        : left(std::move(l)), op(std::move(o)), right(std::move(r)) {}
    void print(std::ostream& os, int indent) const override {
        printIndent(os, indent);
        os << "BinaryExpr: " << op.lexeme << " (" << tokenTypeToString(op.type) << ")\n";
        left->print(os, indent + 1);
        right->print(os, indent + 1);
    }
    Token getToken() const override { return op; } // Added
private:
    static std::string tokenTypeToString(TokenType type) {
        switch (type) {
            case TokenType::PLUS: return "PLUS";
            case TokenType::MINUS: return "MINUS";
            case TokenType::STAR: return "STAR";
            case TokenType::SLASH: return "SLASH";
            case TokenType::GREATER: return "GREATER";
            case TokenType::LESS: return "LESS";
            case TokenType::GREATER_EQUAL: return "GREATER_EQUAL";
            case TokenType::LESS_EQUAL: return "LESS_EQUAL";
            case TokenType::NOT_EQUAL: return "NOT_EQUAL";
            case TokenType::EQUAL_EQUAL: return "EQUAL_EQUAL";
            default: return "UNKNOWN";
        }
    }
};

class ParenExpr : public Expr {
public:
    ExprPtr expr;
    Type inferredType = Type::NONE; // Added

    explicit ParenExpr(ExprPtr e) : expr(std::move(e)) {}
    void print(std::ostream& os, int indent) const override {
        printIndent(os, indent);
        os << "ParenExpr:\n";
        expr->print(os, indent + 1);
    }
    Token getToken() const override { return expr->getToken(); } // Added
};

class ListLiteralExpr : public Expr {
public:
    std::vector<ExprPtr> elements;
    Type inferredType = Type::NONE; // Added

    explicit ListLiteralExpr(std::vector<ExprPtr> e) : elements(std::move(e)) {}
    void print(std::ostream& os, int indent) const override {
        printIndent(os, indent);
        os << "ListLiteralExpr:\n";
        for (size_t i = 0; i < elements.size(); ++i) {
            printIndent(os, indent + 1);
            os << "Element " << i << ":\n";
            elements[i]->print(os, indent + 2);
        }
    }
    Token getToken() const override { // Added
        return elements.empty() ? Token(TokenType::LEFT_BRACKET, "[", 0) : elements[0]->getToken();
    }
};

class DictLiteralExpr : public Expr {
public:
    std::vector<std::pair<ExprPtr, ExprPtr>> entries;
    Type inferredType = Type::NONE; // Added

    explicit DictLiteralExpr(std::vector<std::pair<ExprPtr, ExprPtr>> e) : entries(std::move(e)) {}
    void print(std::ostream& os, int indent) const override {
        printIndent(os, indent);
        os << "DictLiteralExpr:\n";
        for (size_t i = 0; i < entries.size(); ++i) {
            printIndent(os, indent + 1);
            os << "Pair " << i << ":\n";
            printIndent(os, indent + 2);
            os << "Key:\n";
            entries[i].first->print(os, indent + 3);
            printIndent(os, indent + 2);
            os << "Value:\n";
            entries[i].second->print(os, indent + 3);
        }
    }
    Token getToken() const override { // Added
        return entries.empty() ? Token(TokenType::LEFT_BRACE, "{", 0) : entries[0].first->getToken();
    }
};

class IndexExpr : public Expr {
public:
    ExprPtr base;
    ExprPtr index;
    Type inferredType = Type::NONE; // Added

    IndexExpr(ExprPtr b, ExprPtr i) : base(std::move(b)), index(std::move(i)) {}
    void print(std::ostream& os, int indent) const override {
        printIndent(os, indent);
        os << "IndexExpr:\n";
        printIndent(os, indent + 1);
        os << "Base:\n";
        base->print(os, indent + 2);
        printIndent(os, indent + 1);
        os << "Index:\n";
        index->print(os, indent + 2);
    }
    Token getToken() const override { return base->getToken(); } // Added
};

class AssignExpr : public Expr {
public:
    Token name;
    ExprPtr value;
    Type inferredType = Type::NONE; // Added

    AssignExpr(Token n, ExprPtr v) : name(std::move(n)), value(std::move(v)) {}
    void print(std::ostream& os, int indent) const override {
        printIndent(os, indent);
        os << "AssignExpr: " << name.lexeme << "\n";
        printIndent(os, indent + 1);
        os << "Value:\n";
        value->print(os, indent + 2);
    }
    Token getToken() const override { return name; } // Added
};

class IndexAssignExpr : public Expr {
public:
    ExprPtr target;
    ExprPtr value;
    Type inferredType = Type::NONE; // Added

    IndexAssignExpr(ExprPtr t, ExprPtr v) : target(std::move(t)), value(std::move(v)) {}
    void print(std::ostream& os, int indent) const override {
        printIndent(os, indent);
        os << "IndexAssignStmt:\n";
        printIndent(os, indent + 1);
        os << "Target:\n";
        target->print(os, indent + 2);
        printIndent(os, indent + 1);
        os << "Value:\n";
        value->print(os, indent + 2);
    }
    Token getToken() const override { return target->getToken(); } // Added
};

class CallExpr : public Expr {
public:
    Token name;
    std::vector<ExprPtr> arguments;
    Type inferredType = Type::NONE; // Added

    CallExpr(Token n, std::vector<ExprPtr> args)
        : name(std::move(n)), arguments(std::move(args)) {}
    void print(std::ostream& os, int indent) const override {
        printIndent(os, indent);
        os << "CallExpr: " << name.lexeme << "\n";
        printIndent(os, indent + 1);
        os << "Arguments:\n";
        for (size_t i = 0; i < arguments.size(); ++i) {
            printIndent(os, indent + 2);
            os << "Arg " << i << ":\n";
            arguments[i]->print(os, indent + 3);
        }
    }
    Token getToken() const override { return name; } // Added
};

class Stmt {
public:
    virtual ~Stmt() = default;
    virtual void print(std::ostream& os, int indent) const = 0;
};

class VarDeclStmt : public Stmt {
public:
    Token name;
    ExprPtr init;
    Token typeHint;
    bool isLong;
    Type declaredType = Type::NONE; // Added for type checking

    VarDeclStmt(Token n, ExprPtr i, Token t = Token(TokenType::NONE, "", 0), bool l = false)
        : name(std::move(n)), init(std::move(i)), typeHint(std::move(t)), isLong(l) {}
    void print(std::ostream& os, int indent) const override {
        printIndent(os, indent);
        os << "VarDeclStmt: " << name.lexeme;
        if (typeHint.type != TokenType::NONE) {
            os << " (Type: " << tokenTypeToString(typeHint.type);
            if (isLong) os << " LONG";
            os << ")";
        }
        os << "\n";
        if (init) {
            printIndent(os, indent + 1);
            os << "Init:\n";
            init->print(os, indent + 2);
        }
    }
private:
    static std::string tokenTypeToString(TokenType type) {
        switch (type) {
            case TokenType::INTEGER: return "INTEGER";
            case TokenType::NONE: return "NONE";
            default: return "UNKNOWN";
        }
    }
};

class SetStmt : public Stmt {
public:
    Token name;
    ExprPtr value;
    SetStmt(Token n, ExprPtr v) : name(std::move(n)), value(std::move(v)) {}
    void print(std::ostream& os, int indent) const override {
        printIndent(os, indent);
        os << "SetStmt: " << name.lexeme << "\n";
        printIndent(os, indent + 1);
        os << "Value:\n";
        value->print(os, indent + 2);
    }
};

class IndexAssignStmt : public Stmt {
public:
    ExprPtr target;
    ExprPtr value;
    IndexAssignStmt(ExprPtr t, ExprPtr v) : target(std::move(t)), value(std::move(v)) {}
    void print(std::ostream& os, int indent) const override {
        printIndent(os, indent);
        os << "IndexAssignStmt:\n";
        printIndent(os, indent + 1);
        os << "Target:\n";
        target->print(os, indent + 2);
        printIndent(os, indent + 1);
        os << "Value:\n";
        value->print(os, indent + 2);
    }
};

class SayStmt : public Stmt {
public:
    ExprPtr expr;
    explicit SayStmt(ExprPtr e) : expr(std::move(e)) {}
    void print(std::ostream& os, int indent) const override {
        printIndent(os, indent);
        os << "SayStmt:\n";
        expr->print(os, indent + 1);
    }
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
    void print(std::ostream& os, int indent) const override {
        printIndent(os, indent);
        os << "WhenStmt:\n";
        for (size_t i = 0; i < branches.size(); ++i) {
            printIndent(os, indent + 1);
            os << "Branch " << (i + 1) << ":\n";
            if (branches[i].condition) {
                printIndent(os, indent + 2);
                os << "Condition:\n";
                branches[i].condition->print(os, indent + 3);
            } else {
                printIndent(os, indent + 2);
                os << "Otherwise:\n";
            }
            printIndent(os, indent + 2);
            os << "Body:\n";
            for (const auto& stmt : branches[i].body) {
                stmt->print(os, indent + 3);
            }
        }
    }
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
    void print(std::ostream& os, int indent) const override {
        printIndent(os, indent);
        os << "MatchStmt:\n";
        printIndent(os, indent + 1);
        os << "Condition:\n";
        condition->print(os, indent + 2);
        for (size_t i = 0; i < cases.size(); ++i) {
            printIndent(os, indent + 1);
            os << "Case " << (i + 1) << ":\n";
            printIndent(os, indent + 2);
            os << "Pattern:\n";
            cases[i].pattern->print(os, indent + 3);
            printIndent(os, indent + 2);
            os << "Body:\n";
            for (const auto& stmt : cases[i].body) {
                stmt->print(os, indent + 3);
            }
        }
    }
};

class WhileStmt : public Stmt {
public:
    ExprPtr condition;
    std::vector<StmtPtr> body;
    WhileStmt(ExprPtr c, std::vector<StmtPtr> b)
        : condition(std::move(c)), body(std::move(b)) {}
    void print(std::ostream& os, int indent) const override {
        printIndent(os, indent);
        os << "WhileStmt:\n";
        printIndent(os, indent + 1);
        os << "Condition:\n";
        condition->print(os, indent + 2);
        printIndent(os, indent + 1);
        os << "Body:\n";
        for (const auto& stmt : body) {
            stmt->print(os, indent + 2);
        }
    }
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
    void print(std::ostream& os, int indent) const override {
        printIndent(os, indent);
        os << "ForStmt: " << iterator.lexeme << "\n";
        printIndent(os, indent + 1);
        os << "Start:\n";
        start->print(os, indent + 2);
        printIndent(os, indent + 1);
        os << "End:\n";
        end->print(os, indent + 2);
        if (step) {
            printIndent(os, indent + 1);
            os << "Step:\n";
            step->print(os, indent + 2);
        }
        printIndent(os, indent + 1);
        os << "Body:\n";
        for (const auto& stmt : body) {
            stmt->print(os, indent + 2);
        }
    }
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
    void print(std::ostream& os, int indent) const override {
        printIndent(os, indent);
        os << "WithStmt: " << iterator.lexeme << "\n";
        printIndent(os, indent + 1);
        os << "Start:\n";
        start->print(os, indent + 2);
        printIndent(os, indent + 1);
        os << "End:\n";
        end->print(os, indent + 2);
        if (step) {
            printIndent(os, indent + 1);
            os << "Step:\n";
            step->print(os, indent + 2);
        }
        printIndent(os, indent + 1);
        os << "Body:\n";
        for (const auto& stmt : body) {
            stmt->print(os, indent + 2);
        }
    }
};

class FunctionDefStmt : public Stmt {
public:
    Token name;
    std::vector<Token> parameters;
    std::vector<StmtPtr> body;
    Type returnType = Type::NONE; // Added for return type inference

    FunctionDefStmt(Token n, std::vector<Token> params, std::vector<StmtPtr> b)
        : name(std::move(n)), parameters(std::move(params)), body(std::move(b)) {}
    void print(std::ostream& os, int indent) const override {
        printIndent(os, indent);
        os << "FunctionDefStmt: " << name.lexeme << "\n";
        printIndent(os, indent + 1);
        os << "Parameters:\n";
        for (size_t i = 0; i < parameters.size(); ++i) {
            printIndent(os, indent + 2);
            os << "Param " << i << ": " << parameters[i].lexeme << "\n";
        }
        printIndent(os, indent + 1);
        os << "Body:\n";
        for (const auto& stmt : body) {
            stmt->print(os, indent + 2);
        }
    }
};

class CallStmt : public Stmt {
public:
    Token name;
    std::vector<ExprPtr> arguments;
    CallStmt(Token n, std::vector<ExprPtr> args)
        : name(std::move(n)), arguments(std::move(args)) {}
    void print(std::ostream& os, int indent) const override {
        printIndent(os, indent);
        os << "CallStmt: " << name.lexeme << "\n";
        if (!arguments.empty()) {
            printIndent(os, indent + 1);
            os << "Arguments:\n";
            for (size_t i = 0; i < arguments.size(); ++i) {
                printIndent(os, indent + 2);
                os << "Arg " << i << ":\n";
                arguments[i].get()->print(os, indent + 3);
            }
        }
    }
};

class Program {
public:
    std::vector<StmtPtr> statements;
    explicit Program(std::vector<StmtPtr> s) : statements(std::move(s)) {}
    void print(std::ostream& os, int indent) const {
        printIndent(os, indent);
        os << "Program with " << statements.size() << " statements:\n";
        for (const auto& stmt : statements) {
            stmt->print(os, indent + 1);
        }
    }
};

class ThrowStmt : public Stmt {
public:
    std::unique_ptr<Expr> expr;
    ThrowStmt(std::unique_ptr<Expr> e) : expr(std::move(e)) {}
    void print(std::ostream& os, int indent) const override {
        printIndent(os, indent);
        os << "ThrowStmt:\n";
        expr->print(os, indent + 2);
    }
};

class TryCatchStmt : public Stmt {
public:
    std::vector<StmtPtr> tryBody;
    Token exceptionVar;
    std::vector<StmtPtr> catchBody;
    TryCatchStmt(std::vector<StmtPtr> t, Token e, std::vector<StmtPtr> c)
        : tryBody(std::move(t)), exceptionVar(std::move(e)), catchBody(std::move(c)) {}
    void print(std::ostream& os, int indent) const override {
        printIndent(os, indent);
        os << "TryCatchStmt:\n";
        printIndent(os, indent + 1);
        os << "Try Body:\n";
        for (const auto& stmt : tryBody) {
            stmt->print(os, indent + 2);
        }
        printIndent(os, indent + 1);
        os << "Catch Variable: " << exceptionVar.lexeme << "\n";
        printIndent(os, indent + 1);
        os << "Catch Body:\n";
        for (const auto& stmt : catchBody) {
            stmt->print(os, indent + 2);
        }
    }
};

class ReturnStmt : public Stmt {
public:
    ExprPtr value;
    Type returnType = Type::NONE; // Added for return type

    ReturnStmt(ExprPtr value) : value(std::move(value)) {}
    void print(std::ostream& os, int indent) const override {
        printIndent(os, indent);
        os << "ReturnStmt:\n";
        if (value) value->print(os, indent + 1);
    }
};

} // namespace MyCustomLang

#endif // MYCUSTOMLANG_AST_H
#ifndef MYCUSTOMLANG_AST_H
#define MYCUSTOMLANG_AST_H

#include "Token.h"
#include "Type.h"
#include <memory>
#include <vector>
#include <string>
#include <ostream>

namespace MyCustomLang {

class Expr;
class Stmt;

using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;

namespace {
    void printIndent(std::ostream& os, int indent) {
        os << std::string(indent * 2, ' ');
    }
}

class Expr {
public:
    virtual ~Expr() = default;
    virtual void print(std::ostream& os, int indent) const = 0;
    virtual Token getToken() const = 0;
    virtual ExprPtr clone() const = 0; // Added
    Type inferredType = Type::NONE;
};

class LiteralExpr : public Expr {
public:
    Token value;
    Type inferredType = Type::NONE;

    explicit LiteralExpr(Token v) : value(std::move(v)) {}
    void print(std::ostream& os, int indent) const override {
        printIndent(os, indent);
        os << "LiteralExpr: " << value.lexeme << " (" << tokenTypeToString(value.type) << ")\n";
    }
    Token getToken() const override { return value; }
    ExprPtr clone() const override {
        return std::make_unique<LiteralExpr>(value);
    }
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
    Type inferredType = Type::NONE;

    explicit VariableExpr(Token n) : name(std::move(n)) {}
    void print(std::ostream& os, int indent) const override {
        printIndent(os, indent);
        os << "VariableExpr: " << name.lexeme << "\n";
    }
    Token getToken() const override { return name; }
    ExprPtr clone() const override {
        return std::make_unique<VariableExpr>(name);
    }
};

class BinaryExpr : public Expr {
public:
    ExprPtr left;
    Token op;
    ExprPtr right;
    Type inferredType = Type::NONE;

    BinaryExpr(ExprPtr l, Token o, ExprPtr r)
        : left(std::move(l)), op(std::move(o)), right(std::move(r)) {}
    void print(std::ostream& os, int indent) const override {
        printIndent(os, indent);
        os << "BinaryExpr: " << op.lexeme << " (" << tokenTypeToString(op.type) << ")\n";
        left->print(os, indent + 1);
        right->print(os, indent + 1);
    }
    Token getToken() const override { return op; }
    ExprPtr clone() const override {
        return std::make_unique<BinaryExpr>(left->clone(), op, right->clone());
    }
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
    Type inferredType = Type::NONE;

    explicit ParenExpr(ExprPtr e) : expr(std::move(e)) {}
    void print(std::ostream& os, int indent) const override {
        printIndent(os, indent);
        os << "ParenExpr:\n";
        expr->print(os, indent + 1);
    }
    Token getToken() const override { return expr->getToken(); }
    ExprPtr clone() const override {
        return std::make_unique<ParenExpr>(expr->clone());
    }
};

class ListLiteralExpr : public Expr {
public:
    std::vector<ExprPtr> elements;
    Type inferredType = Type::NONE;

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
    Token getToken() const override {
        return elements.empty() ? Token(TokenType::LEFT_BRACKET, "[", 0) : elements[0]->getToken();
    }
    ExprPtr clone() const override {
        std::vector<ExprPtr> clonedElements;
        for (const auto& elem : elements) {
            clonedElements.push_back(elem->clone());
        }
        return std::make_unique<ListLiteralExpr>(std::move(clonedElements));
    }
};

class DictLiteralExpr : public Expr {
public:
    std::vector<std::pair<ExprPtr, ExprPtr>> entries;
    Type inferredType = Type::NONE;

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
    Token getToken() const override {
        return entries.empty() ? Token(TokenType::LEFT_BRACE, "{", 0) : entries[0].first->getToken();
    }
    ExprPtr clone() const override {
        std::vector<std::pair<ExprPtr, ExprPtr>> clonedEntries;
        for (const auto& entry : entries) {
            clonedEntries.emplace_back(entry.first->clone(), entry.second->clone());
        }
        return std::make_unique<DictLiteralExpr>(std::move(clonedEntries));
    }
};

class IndexExpr : public Expr {
public:
    ExprPtr base;
    ExprPtr index;
    Type inferredType = Type::NONE;

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
    Token getToken() const override { return base->getToken(); }
    ExprPtr clone() const override {
        return std::make_unique<IndexExpr>(base->clone(), index->clone());
    }
};

class AssignExpr : public Expr {
public:
    Token name;
    ExprPtr value;
    Type inferredType = Type::NONE;

    AssignExpr(Token n, ExprPtr v) : name(std::move(n)), value(std::move(v)) {}
    void print(std::ostream& os, int indent) const override {
        printIndent(os, indent);
        os << "AssignExpr: " << name.lexeme << "\n";
        printIndent(os, indent + 1);
        os << "Value:\n";
        value->print(os, indent + 2);
    }
    Token getToken() const override { return name; }
    ExprPtr clone() const override {
        return std::make_unique<AssignExpr>(name, value->clone());
    }
};

class IndexAssignExpr : public Expr {
public:
    ExprPtr target;
    ExprPtr value;
    Type inferredType = Type::NONE;

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
    Token getToken() const override { return target->getToken(); }
    ExprPtr clone() const override {
        return std::make_unique<IndexAssignExpr>(target->clone(), value->clone());
    }
};

class CallExpr : public Expr {
public:
    Token name;
    std::vector<ExprPtr> arguments;
    Type inferredType = Type::NONE;

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
    Token getToken() const override { return name; }
    ExprPtr clone() const override {
        std::vector<ExprPtr> clonedArgs;
        for (const auto& arg : arguments) {
            clonedArgs.push_back(arg->clone());
        }
        return std::make_unique<CallExpr>(name, std::move(clonedArgs));
    }
};

class Stmt {
public:
    virtual ~Stmt() = default;
    virtual void print(std::ostream& os, int indent) const = 0;
    virtual StmtPtr clone() const = 0;
};

class VarDeclStmt : public Stmt {
public:
    Token name;
    ExprPtr init;
    Token typeHint;
    bool isLong;
    Type declaredType = Type::NONE;

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
    StmtPtr clone() const override {
        return std::make_unique<VarDeclStmt>(name, init ? init->clone() : nullptr, typeHint, isLong);
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
    StmtPtr clone() const override {
        return std::make_unique<SetStmt>(name, value->clone());
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
    StmtPtr clone() const override {
        return std::make_unique<IndexAssignStmt>(target->clone(), value->clone());
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
    StmtPtr clone() const override {
        return std::make_unique<SayStmt>(expr->clone());
    }
};

class WhenStmt : public Stmt {
public:
    struct Branch {
        ExprPtr condition;
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
    StmtPtr clone() const override {
        std::vector<Branch> clonedBranches;
        for (const auto& branch : branches) {
            std::vector<StmtPtr> clonedBody;
            for (const auto& stmt : branch.body) {
                clonedBody.push_back(stmt->clone());
            }
            clonedBranches.emplace_back(branch.condition ? branch.condition->clone() : nullptr, std::move(clonedBody));
        }
        return std::make_unique<WhenStmt>(std::move(clonedBranches));
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
    StmtPtr clone() const override {
        std::vector<Case> clonedCases;
        for (const auto& c : cases) {
            std::vector<StmtPtr> clonedBody;
            for (const auto& stmt : c.body) {
                clonedBody.push_back(stmt->clone());
            }
            clonedCases.emplace_back(c.pattern->clone(), std::move(clonedBody));
        }
        return std::make_unique<MatchStmt>(condition->clone(), std::move(clonedCases));
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
    StmtPtr clone() const override {
        std::vector<StmtPtr> clonedBody;
        for (const auto& stmt : body) {
            clonedBody.push_back(stmt->clone());
        }
        return std::make_unique<WhileStmt>(condition->clone(), std::move(clonedBody));
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
    StmtPtr clone() const override {
        std::vector<StmtPtr> clonedBody;
        for (const auto& stmt : body) {
            clonedBody.push_back(stmt->clone());
        }
        return std::make_unique<ForStmt>(iterator, start->clone(), end->clone(),
                                         step ? step->clone() : nullptr, std::move(clonedBody));
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
    StmtPtr clone() const override {
        std::vector<StmtPtr> clonedBody;
        for (const auto& stmt : body) {
            clonedBody.push_back(stmt->clone());
        }
        return std::make_unique<WithStmt>(iterator, start->clone(), end->clone(),
                                          step ? step->clone() : nullptr, std::move(clonedBody));
    }
};

class FunctionDefStmt : public Stmt {
public:
    Token name;
    std::vector<Token> parameters;
    std::vector<StmtPtr> body;

    FunctionDefStmt(Token n, std::vector<Token> params, std::vector<StmtPtr> b)
        : name(std::move(n)), parameters(std::move(params)), body(std::move(b)) {}

    FunctionDefStmt(const FunctionDefStmt& other)
        : name(other.name), parameters(other.parameters) {
        for (const auto& stmt : other.body) {
            body.push_back(stmt->clone());
        }
    }

    FunctionDefStmt& operator=(const FunctionDefStmt& other) {
        if (this != &other) {
            name = other.name;
            parameters = other.parameters;
            body.clear();
            for (const auto& stmt : other.body) {
                body.push_back(stmt->clone());
            }
        }
        return *this;
    }

    void print(std::ostream& os, int indent) const override {
        os << std::string(indent, ' ') << "FunctionDefStmt: " << name.lexeme << "\n";
        os << std::string(indent, ' ') << "  Parameters:\n";
        for (const auto& param : parameters) {
            os << std::string(indent + 4, ' ') << "Param: " << param.lexeme << "\n";
        }
        os << std::string(indent, ' ') << "  Body:\n";
        for (const auto& stmt : body) {
            stmt->print(os, indent + 4);
        }
    }

    StmtPtr clone() const override {
        std::vector<StmtPtr> clonedBody;
        for (const auto& stmt : body) {
            clonedBody.push_back(stmt->clone());
        }
        return std::make_unique<FunctionDefStmt>(name, parameters, std::move(clonedBody));
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
    StmtPtr clone() const override {
        std::vector<ExprPtr> clonedArgs;
        for (const auto& arg : arguments) {
            clonedArgs.push_back(arg->clone());
        }
        return std::make_unique<CallStmt>(name, std::move(clonedArgs));
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
    StmtPtr clone() const override {
        return std::make_unique<ThrowStmt>(expr->clone());
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
    StmtPtr clone() const override {
        std::vector<StmtPtr> clonedTryBody;
        for (const auto& stmt : tryBody) {
            clonedTryBody.push_back(stmt->clone());
        }
        std::vector<StmtPtr> clonedCatchBody;
        for (const auto& stmt : catchBody) {
            clonedCatchBody.push_back(stmt->clone());
        }
        return std::make_unique<TryCatchStmt>(std::move(clonedTryBody), exceptionVar, std::move(clonedCatchBody));
    }
};

class ReturnStmt : public Stmt {
public:
    ExprPtr value;
    Type returnType = Type::NONE;

    ReturnStmt(ExprPtr value) : value(std::move(value)) {}
    void print(std::ostream& os, int indent) const override {
        printIndent(os, indent);
        os << "ReturnStmt:\n";
        if (value) value->print(os, indent + 1);
    }
    StmtPtr clone() const override {
        return std::make_unique<ReturnStmt>(value ? value->clone() : nullptr);
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

} // namespace MyCustomLang

#endif // MYCUSTOMLANG_AST_H
#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "AST.h"
#include "SymbolTable.h"
#include <stdexcept>
#include <unordered_map>
#include <variant>

namespace MyCustomLang {

using Value = std::variant<std::monostate, int64_t, std::string, std::shared_ptr<FunctionDefStmt>>;

class Environment {
private:
    std::vector<std::unordered_map<std::string, Value>> scopes;
    size_t currentScope;

public:
    Environment() : currentScope(0) {
        scopes.emplace_back(); // Global scope
    }

    void enterScope() {
        scopes.emplace_back();
        currentScope++;
    }

    void exitScope() {
        if (currentScope > 0) {
            currentScope--;
        } else {
            throw std::runtime_error("Cannot exit global scope");
        }
    }

    void define(const std::string& name, Value value) {
        scopes[currentScope][name] = std::move(value);
    }

    Value get(const std::string& name) const {
        for (size_t i = currentScope; ; --i) {
            auto it = scopes[i].find(name);
            if (it != scopes[i].end()) {
                return it->second;
            }
            if (i == 0) break;
        }
        throw std::runtime_error("Undefined variable: " + name);
    }

    void assign(const std::string& name, Value value) {
        for (size_t i = currentScope; ; --i) {
            auto it = scopes[i].find(name);
            if (it != scopes[i].end()) {
                it->second = std::move(value);
                return;
            }
            if (i == 0) break;
        }
        throw std::runtime_error("Undefined variable: " + name);
    }
};

class Interpreter {
private:
    Environment env;
    const SymbolTable& symbolTable;
    Value evaluateExpr(const Expr* expr); // Changed to take const Expr*
    void executeStmt(const Stmt* stmt);   // Changed to take const Stmt*

public:
    Interpreter(const SymbolTable& st) : symbolTable(st) {}
    void interpret(const Program& program);
};

} // namespace MyCustomLang

#endif
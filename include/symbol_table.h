#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>

enum class TypeKind {
    INTEGER,
    NUMBER, // For 100000L, floating-point, etc.
    LIST,
    DICT,
    STRING,
    UNKNOWN // For untyped or inferred types
};

struct Type {
    TypeKind kind;
    // Add fields for complex types (e.g., List<Integer>)
    Type(TypeKind k) : kind(k) {}
};

struct Symbol {
    std::string name;
    Type type;
    int scopeLevel; // For tracking scope
    // Add fields like memory offset for codegen later
    Symbol(const std::string& n, Type t, int sl) : name(n), type(t), scopeLevel(sl) {}
};

class SymbolTable {
public:
    SymbolTable() : scopeLevel(0) {
        scopes.emplace_back(); // Global scope
    }

    // Enter a new scope
    void enterScope() {
        scopes.emplace_back();
        scopeLevel++;
    }

    // Exit the current scope
    void exitScope() {
        if (scopeLevel > 0) {
            scopes.pop_back();
            scopeLevel--;
        }
    }

    // Add a symbol to the current scope
    void addSymbol(const std::string& name, Type type) {
        auto& currentScope = scopes.back();
        if (currentScope.find(name) != currentScope.end()) {
            throw std::runtime_error("Redeclaration of variable: " + name);
        }
        currentScope[name] = Symbol(name, type, scopeLevel);
    }

    // Lookup a symbol by name, searching from innermost to outermost scope
    Symbol* lookup(const std::string& name) {
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end()) {
                return &found->second;
            }
        }
        return nullptr; // Not found
    }

    // Check if a symbol is declared in the current scope
    bool isDeclaredInCurrentScope(const std::string& name) {
        auto& currentScope = scopes.back();
        return currentScope.find(name) != currentScope.end();
    }

private:
    std::vector<std::unordered_map<std::string, Symbol>> scopes;
    int scopeLevel;
};

#endif
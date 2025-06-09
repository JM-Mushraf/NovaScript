#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "Token.h"
#include "Type.h" // Include Type.h instead of forward declaration
#include <string>
#include <map>
#include <vector>
#include <stdexcept>

namespace MyCustomLang {

struct Symbol {
    Token name;
    Type type; // Use Type enum instead of Token typeHint (INTEGER, STRING, FUNCTION, etc.)
    bool isLong; // For long integers
    std::vector<Token> parameters; // For functions
    Type returnType; // Add for function return types

    // Default constructor
    Symbol() : name(TokenType::UNKNOWN, "", 0), type(Type::NONE), isLong(false), parameters(), returnType(Type::NONE) {}

    Symbol(Token n, Type t, bool l = false, std::vector<Token> p = {})
        : name(std::move(n)), type(t), isLong(l), parameters(std::move(p)), returnType(Type::NONE) {}
};

class SymbolTable {
private:
    std::vector<std::map<std::string, Symbol>> scopes;
    size_t currentScope; // Track the active scope

public:
    SymbolTable() : currentScope(0) {
        scopes.emplace_back(); // Global scope
    }

    void enterScope() {
        scopes.emplace_back();
        currentScope++;
    }

    void exitScope() {
        if (currentScope > 0) {
            currentScope--; // Move back to parent scope, but keep the scope in scopes
        } else {
            throw std::runtime_error("Cannot exit global scope");
        }
    }

    void addSymbol(const Token& name, const Token& typeHint, bool isLong, const std::vector<Token>& params = {});

    // Overload addSymbol to accept a Type directly (for semantic analyzer updates)
    void addSymbol(const Token& name, Type type, bool isLong, const std::vector<Token>& params = {});

    bool symbolExists(const std::string& name) const {
        for (size_t i = currentScope; i < scopes.size(); i--) {
            if (scopes[i].find(name) != scopes[i].end()) {
                return true;
            }
            if (i == 0) break; // Stop at global scope
        }
        return false;
    }

    Symbol getSymbol(const std::string& name) const {
        for (size_t i = currentScope; i < scopes.size(); i--) {
            auto sym = scopes[i].find(name);
            if (sym != scopes[i].end()) {
                return sym->second;
            }
            if (i == 0) break;
        }
        throw std::runtime_error("Symbol '" + name + "' not found");
    }

    // Add method to update a symbol's type (used by semantic analyzer)
    void updateSymbolType(const std::string& name, Type type);

    // Add method to update a function's return type (used by semantic analyzer)
    void updateSymbolReturnType(const std::string& name, Type returnType);

    const std::vector<std::map<std::string, Symbol>>& getScopes() const {
        return scopes;
    }
};

} // namespace MyCustomLang

#endif
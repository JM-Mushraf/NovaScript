#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "Token.h"
#include <string>
#include <map>
#include <vector>
#include <stdexcept>

namespace MyCustomLang {

struct Symbol {
    Token name;
    Token typeHint; // INTEGER or NONE
    bool isLong;    // For long integers
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

    void addSymbol(const Token& name, const Token& typeHint, bool isLong) {
        scopes[currentScope][name.lexeme] = {name, typeHint, isLong};
    }

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

    const std::vector<std::map<std::string, Symbol>>& getScopes() const {
        return scopes;
    }
};

} // namespace MyCustomLang

#endif
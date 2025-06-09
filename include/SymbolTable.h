#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "Token.h"
#include "Type.h"
#include <string>
#include <map>
#include <vector>
#include <stdexcept>

namespace MyCustomLang {

struct Symbol {
    Token name;
    Type type;
    bool isLong;
    std::vector<Token> parameters;
    Type returnType;

    Symbol() : name(TokenType::UNKNOWN, "", 0), type(Type::NONE), isLong(false), parameters(), returnType(Type::NONE) {}
    Symbol(Token n, Type t, bool l = false, std::vector<Token> p = {})
        : name(std::move(n)), type(t), isLong(l), parameters(std::move(p)), returnType(Type::NONE) {}
};

class SymbolTable {
private:
    std::vector<std::map<std::string, Symbol>> scopes;
    size_t currentScope;

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
            currentScope--;
        } else {
            throw std::runtime_error("Cannot exit global scope");
        }
    }

    void addSymbol(const Token& name, const Token& typeHint, bool isLong, const std::vector<Token>& params = {});
    void addSymbol(const Token& name, Type type, bool isLong, const std::vector<Token>& params = {});

    bool symbolExists(const std::string& name) const;
    bool symbolExistsInCurrentScope(const std::string& name) const; // New method

    Symbol getSymbol(const std::string& name) const {
        for (size_t i = currentScope; ; --i) {
            auto sym = scopes[i].find(name);
            if (sym != scopes[i].end()) {
                return sym->second;
            }
            if (i == 0) break;
        }
        throw std::runtime_error("Symbol '" + name + "' not found");
    }

    void updateSymbolType(const std::string& name, Type type);
    void updateSymbolReturnType(const std::string& name, Type returnType);

    const std::vector<std::map<std::string, Symbol>>& getScopes() const {
        return scopes;
    }
};

} // namespace MyCustomLang

#endif
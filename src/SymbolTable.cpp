#include "SymbolTable.h"

namespace MyCustomLang {

void SymbolTable::addSymbol(const Token& name, const Token& typeHint, bool isLong, const std::vector<Token>& params) {
    Type type = Type::NONE;
    if (typeHint.type == TokenType::INTEGER) type = Type::INTEGER;
    else if (typeHint.type == TokenType::STRING) type = Type::STRING;
    else if (typeHint.type == TokenType::FUNCTION) type = Type::FUNCTION;
    scopes[currentScope].emplace(name.lexeme, Symbol{name, type, isLong, params});
}

void SymbolTable::addSymbol(const Token& name, Type type, bool isLong, const std::vector<Token>& params) {
    scopes[currentScope].emplace(name.lexeme, Symbol{name, type, isLong, params});
}

void SymbolTable::updateSymbolType(const std::string& name, Type type) {
    for (size_t i = currentScope; i < scopes.size(); i--) {
        auto sym = scopes[i].find(name);
        if (sym != scopes[i].end()) {
            sym->second.type = type;
            return;
        }
        if (i == 0) break;
    }
    throw std::runtime_error("Symbol '" + name + "' not found for type update");
}

void SymbolTable::updateSymbolReturnType(const std::string& name, Type returnType) {
    for (size_t i = 0; i < scopes.size(); i++) { // Start from global scope
        auto sym = scopes[i].find(name);
        if (sym != scopes[i].end()) {
            sym->second.returnType = returnType;
            return;
        }
    }
    throw std::runtime_error("Symbol '" + name + "' not found for return type update");
}

} // namespace MyCustomLang
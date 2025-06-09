#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "Lexer.h"
#include "Parser.h"
#include "SymbolTable.h"
#include "SemanticAnalyzer.h"
#include "Type.h"

namespace MyCustomLang {

void printToken(const Token& token) {
    std::string lexeme = token.lexeme;
    if (token.type == TokenType::INDENT) lexeme = "<indent>";
    else if (token.type == TokenType::DEDENT) lexeme = "<dedent>";
    else if (token.type == TokenType::NEWLINE) lexeme = "<newline>";
    else if (token.type == TokenType::EQUAL_EQUAL) lexeme = token.lexeme;
    else if (lexeme.empty()) lexeme = "''";
    std::cout << "Token: " << std::left << std::setw(20) << lexeme 
              << " (" << tokenTypeToString(token.type) << ") at line " 
              << token.line << "\n";
    if (token.type == TokenType::NEWLINE || token.type == TokenType::UNKNOWN) {
        std::cerr << "Debug: Detected " << tokenTypeToString(token.type) 
                  << " token at line " << token.line << "\n";
    }
}

void printAST(const Program& program) {
    std::cout << "\nAbstract Syntax Tree (AST):\n";
    program.print(std::cout, 0);
    std::cout << "\n";
}

void printSymbolTable(const SymbolTable& symbolTable) {
    std::cout << "Symbol Table:\n";
    const auto& scopes = symbolTable.getScopes();
    for (size_t i = 0; i < scopes.size(); ++i) {
        if (scopes[i].empty()) continue; // Skip empty scopes
        std::cout << "Scope " << i << ":\n";
        for (const auto& [name, symbol] : scopes[i]) {
            std::cout << "  Variable: " << name 
                      << " (Type: " << typeToString(symbol.type); // Fixed: Use symbol.type
            if (symbol.isLong) {
                std::cout << " LONG";
            }
            if (symbol.type == Type::FUNCTION) {
                std::cout << ", Parameters: [";
                for (size_t j = 0; j < symbol.parameters.size(); ++j) {
                    std::cout << symbol.parameters[j].lexeme;
                    if (j < symbol.parameters.size() - 1) std::cout << ", ";
                }
                std::cout << "], Return Type: " << typeToString(symbol.returnType); // Added: Print return type
            }
            std::cout << ", Line: " << symbol.name.line << ")\n";
        }
    }
}

} // namespace MyCustomLang

int main() {
    std::ifstream file("code.ns");
    if (!file.is_open()) {
        std::cerr << "Could not open file 'code.ns'.\n";
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string code = buffer.str();
    file.close();

    MyCustomLang::Lexer lexer(code);
    std::vector<MyCustomLang::Token> tokens;
    MyCustomLang::Token token;
    do {
        token = lexer.getNextToken();
        tokens.push_back(token);
    } while (token.type != MyCustomLang::TokenType::END_OF_FILE);

    std::cout << "Tokens (" << tokens.size() << " total):\n";
    for (const auto& t : tokens) {
        MyCustomLang::printToken(t);
    }
    std::cout << "\n";

    try {
        MyCustomLang::Parser parser(tokens);
        MyCustomLang::Program ast = parser.parse();
        std::cout << "Parsing successful!\n";
        std::cout << "Parsed " << tokens.size() << " tokens into " 
                  << ast.statements.size() << " statements.\n";
        MyCustomLang::printAST(ast);

        // Add semantic analysis
        MyCustomLang::SemanticAnalyzer analyzer(parser.getSymbolTable());
        analyzer.analyze(ast);
        std::cout << "Semantic analysis successful!\n";

        MyCustomLang::printSymbolTable(parser.getSymbolTable());
    } catch (const MyCustomLang::ParserError& e) {
        std::cerr << "Parsing failed at line " << e.token.line << ": " << e.what() << "\n";
        return 1;
    } catch (const MyCustomLang::SemanticError& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "../include/Lexer.h"
#include "../include/Parser.h"

using namespace MyCustomLang;

void printToken(const Token& token) {
    std::string lexeme = token.lexeme;
    if (token.type == TokenType::INDENT) lexeme = "<indent>";
    else if (token.type == TokenType::DEDENT) lexeme = "<dedent>";
    else if (token.type == TokenType::NEWLINE) lexeme = "<newline>";
    else if (lexeme.empty()) lexeme = "''";
    std::cout << "Token: " << std::left << std::setw(20) << lexeme 
              << " (" << tokenTypeToString(token.type) << ") at line " 
              << token.line << "\n";
}

void printExpr(const Expr* expr, int indent = 0) {
    std::string indentStr(indent * 2, ' ');
    if (const auto* literal = dynamic_cast<const LiteralExpr*>(expr)) {
        std::cout << indentStr << "LiteralExpr: " << literal->value.lexeme 
                  << " (" << tokenTypeToString(literal->value.type) << ")\n";
    }
    else if (const auto* variable = dynamic_cast<const VariableExpr*>(expr)) {
        std::cout << indentStr << "VariableExpr: " << variable->name.lexeme << "\n";
    }
    else if (const auto* binary = dynamic_cast<const BinaryExpr*>(expr)) {
        std::cout << indentStr << "BinaryExpr: " << binary->op.lexeme 
                  << " (" << tokenTypeToString(binary->op.type) << ")\n";
        printExpr(binary->left.get(), indent + 1);
        printExpr(binary->right.get(), indent + 1);
    }
    else if (const auto* paren = dynamic_cast<const ParenExpr*>(expr)) {
        std::cout << indentStr << "ParenExpr:\n";
        printExpr(paren->expr.get(), indent + 1);
    }
    else if (const auto* list = dynamic_cast<const ListLiteralExpr*>(expr)) {
        std::cout << indentStr << "ListLiteralExpr:\n";
        for (const auto& elem : list->elements) {
            std::cout << indentStr << "  Element:\n";
            printExpr(elem.get(), indent + 2);
        }
    }
    else if (const auto* dict = dynamic_cast<const DictLiteralExpr*>(expr)) {
        std::cout << indentStr << "DictLiteralExpr:\n";
        for (const auto& pair : dict->pairs) {
            std::cout << indentStr << "  Pair:\n";
            std::cout << indentStr << "    Key:\n";
            printExpr(pair.first.get(), indent + 3);
            std::cout << indentStr << "    Value:\n";
            printExpr(pair.second.get(), indent + 3);
        }
    }
    else {
        std::cout << indentStr << "Unknown Expr\n";
    }
}

void printStmt(const Stmt* stmt, int indent = 0) {
    std::string indentStr(indent * 2, ' ');
    if (const auto* varDecl = dynamic_cast<const VarDeclStmt*>(stmt)) {
        std::cout << indentStr << "VarDeclStmt: " << varDecl->name.lexeme << "\n";
        std::cout << indentStr << "  Init:\n";
        printExpr(varDecl->init.get(), indent + 2);
    }
    else if (const auto* assign = dynamic_cast<const AssignmentStmt*>(stmt)) {
        std::cout << indentStr << "AssignmentStmt: " << assign->name.lexeme << "\n";
        std::cout << indentStr << "  Value:\n";
        printExpr(assign->value.get(), indent + 2);
    }
    else if (const auto* say = dynamic_cast<const SayStmt*>(stmt)) {
        std::cout << indentStr << "SayStmt:\n";
        printExpr(say->expr.get(), indent + 1);
    }
    else if (const auto* when = dynamic_cast<const WhenStmt*>(stmt)) {
        std::cout << indentStr << "WhenStmt:\n";
        for (size_t i = 0; i < when->branches.size(); ++i) {
            const auto& branch = when->branches[i];
            std::cout << indentStr << "  Branch " << (i + 1) << ":\n";
            if (branch.condition) {
                std::cout << indentStr << "    Condition:\n";
                printExpr(branch.condition.get(), indent + 3);
            }
            else {
                std::cout << indentStr << "    Otherwise:\n";
            }
            std::cout << indentStr << "    Body:\n";
            for (const auto& bodyStmt : branch.body) {
                printStmt(bodyStmt.get(), indent + 3);
            }
        }
    }
    else {
        std::cout << indentStr << "Unknown Stmt\n";
    }
}

void printAST(const Program& program) {
    std::cout << "\nAbstract Syntax Tree (AST):\n";
    std::cout << "Program with " << program.statements.size() << " statements:\n";
    for (const auto& stmt : program.statements) {
        printStmt(stmt.get());
    }
    std::cout << "\n";
}

int main() {
    std::ifstream file("code.ns");
    if (!file.is_open()) {
        std::cerr << "Could not open file 'code.ns'.\n";
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string code = buffer.str();

    Lexer lexer(code);
    std::vector<Token> tokens;
    Token token;
    do {
        token = lexer.getNextToken();
        tokens.push_back(token);
    } while (token.type != TokenType::END_OF_FILE);

    std::cout << "Tokens (" << tokens.size() << " total):\n";
    for (const auto& t : tokens) {
        printToken(t);
    }
    std::cout << "\n";

    try {
        Parser parser(tokens);
        Program ast = parser.parse();
        std::cout << "Parsing successful!\n";
        std::cout << "Parsed " << tokens.size() << " tokens into " 
                  << ast.statements.size() << " statements.\n";
        printAST(ast);
    } catch (const ParserError& e) {
        std::cerr << "Parsing failed: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
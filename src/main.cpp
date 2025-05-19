#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "Lexer.h"
#include "Parser.h"

namespace MyCustomLang {

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
        for (const auto& pair : dict->entries) {
            std::cout << indentStr << "  Pair:\n";
            std::cout << indentStr << "    Key:\n";
            printExpr(pair.first.get(), indent + 3);
            std::cout << indentStr << "    Value:\n";
            printExpr(pair.second.get(), indent + 3);
        }
    }
    else if (const auto* index = dynamic_cast<const IndexExpr*>(expr)) {
        std::cout << indentStr << "IndexExpr:\n";
        std::cout << indentStr << "  Base:\n";
        printExpr(index->base.get(), indent + 2);
        std::cout << indentStr << "  Index:\n";
        printExpr(index->index.get(), indent + 2);
    }
    else if (const auto* assign = dynamic_cast<const AssignExpr*>(expr)) {
        std::cout << indentStr << "AssignExpr: " << assign->name.lexeme << "\n";
        std::cout << indentStr << "  Value:\n";
        printExpr(assign->value.get(), indent + 2);
    }
    else if (const auto* indexAssign = dynamic_cast<const IndexAssignExpr*>(expr)) {
        std::cout << indentStr << "IndexAssignExpr:\n";
        std::cout << indentStr << "  Target:\n";
        printExpr(indexAssign->target.get(), indent + 2);
        std::cout << indentStr << "  Value:\n";
        printExpr(indexAssign->value.get(), indent + 2);
    }
    else {
        std::cout << indentStr << "Unknown Expr\n";
    }
}

void printStmt(const Stmt* stmt, int indent = 0) {
    std::string indentStr(indent * 2, ' ');
    if (const auto* varDecl = dynamic_cast<const VarDeclStmt*>(stmt)) {
        std::cout << indentStr << "VarDeclStmt: " << varDecl->name.lexeme;
        if (varDecl->typeHint.type != TokenType::NONE) {
            std::cout << " (Type: " << varDecl->typeHint.lexeme;
            if (varDecl->isLong) std::cout << " long";
            std::cout << ")";
        }
        std::cout << "\n";
        std::cout << indentStr << "  Init:\n";
        printExpr(varDecl->init.get(), indent + 2);
    }
    else if (const auto* indexAssign = dynamic_cast<const IndexAssignStmt*>(stmt)) {
        std::cout << indentStr << "IndexAssignStmt:\n";
        std::cout << indentStr << "  Target:\n";
        printExpr(indexAssign->target.get(), indent + 2);
        std::cout << indentStr << "  Value:\n";
        printExpr(indexAssign->value.get(), indent + 2);
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
    else if (const auto* match = dynamic_cast<const MatchStmt*>(stmt)) {
        std::cout << indentStr << "MatchStmt:\n";
        std::cout << indentStr << "  Condition:\n";
        printExpr(match->condition.get(), indent + 2);
        for (size_t i = 0; i < match->cases.size(); ++i) {
            const auto& case_ = match->cases[i];
            std::cout << indentStr << "  Case " << (i + 1) << ":\n";
            std::cout << indentStr << "    Pattern:\n";
            printExpr(case_.pattern.get(), indent + 3);
            std::cout << indentStr << "    Body:\n";
            for (const auto& bodyStmt : case_.body) {
                printStmt(bodyStmt.get(), indent + 3);
            }
        }
    }
    else if (const auto* whileStmt = dynamic_cast<const WhileStmt*>(stmt)) {
        std::cout << indentStr << "WhileStmt:\n";
        std::cout << indentStr << "  Condition:\n";
        printExpr(whileStmt->condition.get(), indent + 2);
        std::cout << indentStr << "  Body:\n";
        for (const auto& bodyStmt : whileStmt->body) {
            printStmt(bodyStmt.get(), indent + 2);
        }
    }
    else if (const auto* forStmt = dynamic_cast<const ForStmt*>(stmt)) {
        std::cout << indentStr << "ForStmt: Iterator " << forStmt->iterator.lexeme << "\n";
        std::cout << indentStr << "  Start:\n";
        printExpr(forStmt->start.get(), indent + 2);
        std::cout << indentStr << "  End:\n";
        printExpr(forStmt->end.get(), indent + 2);
        if (forStmt->step) {
            std::cout << indentStr << "  Step:\n";
            printExpr(forStmt->step.get(), indent + 2);
        }
        std::cout << indentStr << "  Body:\n";
        for (const auto& bodyStmt : forStmt->body) {
            printStmt(bodyStmt.get(), indent + 2);
        }
    }
    else if (const auto* withStmt = dynamic_cast<const WithStmt*>(stmt)) {
        std::cout << indentStr << "WithStmt: Iterator " << withStmt->iterator.lexeme << "\n";
        std::cout << indentStr << "  Start:\n";
        printExpr(withStmt->start.get(), indent + 2);
        std::cout << indentStr << "  End:\n";
        printExpr(withStmt->end.get(), indent + 2);
        if (withStmt->step) {
            std::cout << indentStr << "  Step:\n";
            printExpr(withStmt->step.get(), indent + 2);
        }
        std::cout << indentStr << "  Body:\n";
        for (const auto& bodyStmt : withStmt->body) {
            printStmt(bodyStmt.get(), indent + 2);
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
    } catch (const MyCustomLang::ParserError& e) {
        std::cerr << "Parsing failed at line " << e.token.line << ": " << e.what() << "\n";
        return 1;
    }

    return 0;
}
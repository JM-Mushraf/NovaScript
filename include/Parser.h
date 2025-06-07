#ifndef PARSER_H
#define PARSER_H

#include "Token.h"
#include "AST.h"
#include "SymbolTable.h"
#include <vector>
#include <memory>

namespace MyCustomLang {

class Parser {
private:
    std::vector<Token> tokens;
    size_t current;
    SymbolTable symbolTable;

    Token peek() const;
    Token peekNext() const;
    Token previous() const;
    Token advance();
    bool isAtEnd() const;
    bool check(TokenType type) const;
    bool checkNext(TokenType type) const;
    bool match(TokenType type);
    void synchronize();

    Program parseProgram();
    StmtPtr parseStmt();
    StmtPtr parseVarDecl();
    StmtPtr parseSetStmt();
    StmtPtr parseWhenStmt();
    StmtPtr parseSayStmt();
    StmtPtr parseMatchStmt();
    StmtPtr parseWhileLoop();
    StmtPtr parseForLoop();
    StmtPtr parseWithLoop();
    StmtPtr parseTryCatchStmt();
    StmtPtr parseFunctionDef();
    StmtPtr parseCallStmt();
    StmtPtr parseReturnStmt();
    ExprPtr parseCallExpr();
    std::vector<StmtPtr> parseStmtList();
    ExprPtr parseExpr();
    ExprPtr parseAssignment();
    ExprPtr parseBinaryExpr();
    ExprPtr parsePrimary();
    ExprPtr parseListLiteral();
    ExprPtr parseDictLiteral();
    ExprPtr parseIndexExpr(ExprPtr base);

public:
    Parser(std::vector<Token> t);
    Program parse();
    const SymbolTable& getSymbolTable() const { return symbolTable; }
};

class ParserError : public std::runtime_error {
public:
    Token token;
    ParserError(const Token& token, const std::string& message)
        : std::runtime_error("Parser error at line " + std::to_string(token.line) + ": " + message),
          token(token) {}
};

} // namespace MyCustomLang

#endif
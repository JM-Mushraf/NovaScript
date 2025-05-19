#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include "Token.h"
#include "ast.h"
#include "ParseError.h"

namespace MyCustomLang {

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    Program parse();

private:
    std::vector<Token> tokens;
    size_t current;

    Token peek() const;
    Token peekNext() const;
    Token previous() const;
    Token advance();
    bool isAtEnd() const;
    bool check(TokenType type) const;
    bool checkNext(TokenType type) const;
    bool match(TokenType type);
    Program parseProgram();
    StmtPtr parseStmt();
    StmtPtr parseVarDecl();
    StmtPtr parseAssignmentStmt();
    StmtPtr parseWhenStmt();
    StmtPtr parseSayStmt();
    std::vector<StmtPtr> parseStmtList();
    ExprPtr parseExpr();
    ExprPtr parseBinaryExpr();
    ExprPtr parsePrimary();
};

} // namespace MyCustomLang

#endif
#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include "Token.h"
#include "ast.h"
#include "ParseError.h"

// Parser class for recursive descent parsing.
// Consumes tokens from the lexer and constructs an Abstract Syntax Tree (AST).
// Supports basic constructs: variable declarations, when-then statements, and say statements.

namespace MyCustomLang {

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    Program parse(); // Parse the entire program and return the AST

private:
    std::vector<Token> tokens; // Token list from the lexer
    size_t current;           // Current token index

    // Helper functions for token processing
    Token peek() const;          // Look at the current token
    Token advance();             // Consume and return the current token
    bool isAtEnd() const;        // Check if at the end of tokens
    Token previous() const;      // Get the previously consumed token
    bool check(TokenType type) const; // Check if current token is of given type
    bool match(TokenType type);  // Consume token if it matches type
    void synchronize();          // Recover from parsing errors

    // Recursive descent parsing functions
    Program parseProgram();      // Parse top-level program
    StmtPtr parseStmt();         // Parse a statement
    StmtPtr parseVarDecl();      // Parse variable declaration
    StmtPtr parseWhenStmt();     // Parse when-then statement
    StmtPtr parseSayStmt();      // Parse say statement
    std::vector<StmtPtr> parseStmtList(); // Parse a list of statements (block)
    ExprPtr parseExpr();         // Parse an expression
    ExprPtr parseBinaryExpr();   // Parse binary expressions
    ExprPtr parsePrimary();      // Parse primary expressions
};

} // namespace MyCustomLang

#endif
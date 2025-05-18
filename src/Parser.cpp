#include "Parser.h"
#include "Lexer.h"

#include <stdexcept>

using namespace MyCustomLang;

// Parser implementation using recursive descent.
// Each parsing function corresponds to a grammar rule, consuming tokens and
// building AST nodes. Errors are thrown as ParserError with detailed messages.

Parser::Parser(std::vector<Token> t) : tokens(std::move(t)), current(0) {}

Token Parser::peek() const
{
    // Return the current token without advancing
    return isAtEnd() ? Token(TokenType::END_OF_FILE, "", 0) : tokens[current];
}

Token Parser::advance()
{
    // Consume and return the current token, advancing the index
    if (!isAtEnd())
        current++;
    return previous();
}

bool Parser::isAtEnd() const
{
    // Check if all tokens are consumed
    return current >= tokens.size();
}

Token Parser::previous() const
{
    // Return the previously consumed token
    return tokens[current - 1];
}

bool Parser::check(TokenType type) const
{
    // Check if the current token matches the given type
    return !isAtEnd() && peek().type == type;
}

bool Parser::match(TokenType type)
{
    // Consume the current token if it matches the type, return true if matched
    if (check(type))
    {
        advance();
        return true;
    }
    return false;
}

void Parser::synchronize()
{
    // Recover from parsing errors by skipping tokens until a statement boundary
    while (!isAtEnd())
    {
        if (peek().type == TokenType::NEWLINE ||
            peek().type == TokenType::END ||
            peek().type == TokenType::LET ||
            peek().type == TokenType::WHEN ||
            peek().type == TokenType::SAY)
        {
            break;
        }
        advance();
    }
}

Program Parser::parse()
{
    // Parse the entire program and return the AST
    try
    {
        return parseProgram();
    }
    catch (const ParserError &e)
    {
        // In a real application, you might collect errors instead of stopping
        throw; // Rethrow for now
    }
}

Program Parser::parseProgram()
{
    // Parse a sequence of statements
    // Grammar: program ::= stmt_list
    std::vector<StmtPtr> statements;
    while (!isAtEnd() && peek().type != TokenType::END_OF_FILE)
    {
        if (match(TokenType::NEWLINE))
        {
            continue; // Skip empty lines
        }
        statements.push_back(parseStmt());
    }
    return Program(std::move(statements));
}

StmtPtr Parser::parseStmt()
{
    // Parse a single statement
    // Grammar: stmt ::= var_decl | when_stmt | say_stmt
    try
    {
        if (match(TokenType::LET))
        {
            return parseVarDecl();
        }
        if (match(TokenType::WHEN))
        {
            return parseWhenStmt();
        }
        if (match(TokenType::SAY))
        {
            return parseSayStmt();
        }
        throw ParserError(peek(), "Expected statement (let, when, or say)");
    }
    catch (const ParserError &e)
    {
        synchronize();
        throw;
    }
}

StmtPtr Parser::parseVarDecl()
{
    // Parse a variable declaration
    // Grammar: var_decl ::= "let" IDENTIFIER "=" expr [NEWLINE]
    Token name = advance();
    if (name.type != TokenType::IDENTIFIER)
    {
        throw ParserError(name, "Expected identifier after 'let'");
    }
    if (!match(TokenType::EQUAL))
    {
        throw ParserError(peek(), "Expected '=' after identifier");
    }
    ExprPtr init = parseExpr();
    // NEWLINE is optional
    match(TokenType::NEWLINE);
    return std::make_unique<VarDeclStmt>(name, std::move(init));
}

StmtPtr Parser::parseWhenStmt()
{
    // Parse a when-then-otherwise statement
    // Grammar: when_stmt ::= "when" expr "then" stmt_list
    //                        ["otherwise" ("when" expr "then" stmt_list | stmt_list)]
    //                        "end" [NEWLINE]
    std::vector<WhenStmt::Branch> branches;

    // Parse the first "when" branch
    ExprPtr condition = parseExpr();
    if (!match(TokenType::THEN))
    {
        throw ParserError(peek(), "Expected 'then' after condition");
    }
    // Expect INDENT for the block
    if (!match(TokenType::INDENT))
    {
        throw ParserError(peek(), "Expected indentation after 'then'");
    }
    auto body = parseStmtList();
    branches.emplace_back(std::move(condition), std::move(body));

    // Parse optional "otherwise" branches
    while (match(TokenType::DEDENT) && match(TokenType::OTHERWISE))
    {
        if (check(TokenType::WHEN))
        {
            // "otherwise when" branch
            advance(); // Consume WHEN
            condition = parseExpr();
            if (!match(TokenType::THEN))
            {
                throw ParserError(peek(), "Expected 'then' after condition");
            }
            if (!match(TokenType::INDENT))
            {
                throw ParserError(peek(), "Expected indentation after 'then'");
            }
            body = parseStmtList();
            branches.emplace_back(std::move(condition), std::move(body));
        }
        else
        {
            // Final "otherwise" branch (no condition)
            if (!match(TokenType::INDENT))
            {
                throw ParserError(peek(), "Expected indentation after 'otherwise'");
            }
            body = parseStmtList();
            branches.emplace_back(nullptr, std::move(body));
        }
    }

    // Expect "end" to close the when statement
    if (!match(TokenType::END))
    {
        throw ParserError(peek(), "Expected 'end' to close 'when' statement");
    }
    match(TokenType::NEWLINE); // Optional NEWLINE
    return std::make_unique<WhenStmt>(std::move(branches));
}

StmtPtr Parser::parseSayStmt()
{
    // Parse a say statement
    // Grammar: say_stmt ::= "say" expr [NEWLINE]
    ExprPtr expr = parseExpr();
    match(TokenType::NEWLINE); // Optional NEWLINE
    return std::make_unique<SayStmt>(std::move(expr));
}

std::vector<StmtPtr> Parser::parseStmtList()
{
    // Parse a list of statements, typically within an INDENT/DEDENT block
    // Grammar: stmt_list ::= stmt | stmt stmt_list | ε
    std::vector<StmtPtr> statements;
    while (!check(TokenType::DEDENT) && !check(TokenType::END) && !isAtEnd())
    {
        if (match(TokenType::NEWLINE))
        {
            continue; // Skip NEWLINE tokens within blocks
        }
        statements.push_back(parseStmt());
    }
    return statements;
}

ExprPtr Parser::parseExpr()
{
    // Parse an expression (delegates to binary_expr)
    // Grammar: expr ::= binary_expr
    return parseBinaryExpr();
}

ExprPtr Parser::parseBinaryExpr()
{
    // Parse a binary expression with precedence
    // Grammar: binary_expr ::= primary (op primary)*
    ExprPtr left = parsePrimary();
    while (check(TokenType::PLUS) || check(TokenType::MINUS) ||
           check(TokenType::STAR) || check(TokenType::SLASH) ||
           check(TokenType::GREATER) || check(TokenType::LESS) ||
           check(TokenType::EQUAL))
    {
        Token op = advance();
        ExprPtr right = parsePrimary();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }
    return left;
}

ExprPtr Parser::parsePrimary()
{
    // Parse a primary expression
    // Grammar: primary ::= NUMBER | STRING | IDENTIFIER | "(" expr ")"
    if (match(TokenType::NUMBER))
    {
        return std::make_unique<LiteralExpr>(previous());
    }
    if (match(TokenType::STRING))
    {
        return std::make_unique<LiteralExpr>(previous());
    }
    if (match(TokenType::IDENTIFIER))
    {
        return std::make_unique<VariableExpr>(previous());
    }
    if (match(TokenType::LEFT_PAREN))
    {
        ExprPtr expr = parseExpr();
        if (!match(TokenType::RIGHT_PAREN))
        {
            throw ParserError(peek(), "Expected ')' after expression");
        }
        return std::make_unique<ParenExpr>(std::move(expr));
    }
    throw ParserError(peek(), "Expected expression");
}
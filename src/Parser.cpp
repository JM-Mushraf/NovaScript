#include "../include/parser.h"
#include <stdexcept>

using namespace MyCustomLang;

Parser::Parser(std::vector<Token> t) : tokens(std::move(t)), current(0) {}

Token Parser::peek() const {
    return isAtEnd() ? Token(TokenType::END_OF_FILE, "", 0) : tokens[current];
}

Token Parser::peekNext() const {
    return current + 1 < tokens.size() ? tokens[current + 1] : Token(TokenType::END_OF_FILE, "", 0);
}

Token Parser::previous() const {
    return tokens[current - 1];
}

Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

bool Parser::isAtEnd() const {
    return current >= tokens.size();
}

bool Parser::check(TokenType type) const {
    return peek().type == type;
}

bool Parser::checkNext(TokenType type) const {
    return peekNext().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

Program Parser::parseProgram() {
    std::vector<StmtPtr> statements;
    while (!isAtEnd() && peek().type != TokenType::END_OF_FILE) {
        if (match(TokenType::NEWLINE)) {
            continue;
        }
        statements.push_back(parseStmt());
    }
    return Program(std::move(statements));
}

StmtPtr Parser::parseStmt() {
    if (match(TokenType::LET)) return parseVarDecl();
    if (match(TokenType::SET)) return parseAssignmentStmt();
    if (match(TokenType::WHEN)) return parseWhenStmt();
    if (match(TokenType::SAY)) return parseSayStmt();
    throw ParserError(peek(), "Expected statement");
}

StmtPtr Parser::parseVarDecl() {
    Token name = advance();
    if (name.type != TokenType::IDENTIFIER) {
        throw ParserError(name, "Expected identifier after 'let'");
    }
    if (!match(TokenType::EQUAL) && !match(TokenType::BE)) {
        throw ParserError(peek(), "Expected '=' or 'be' after identifier");
    }
    ExprPtr init = parseExpr();
    match(TokenType::NEWLINE);
    return std::make_unique<VarDeclStmt>(name, std::move(init));
}

StmtPtr Parser::parseAssignmentStmt() {
    Token name = advance();
    if (name.type != TokenType::IDENTIFIER) {
        throw ParserError(name, "Expected identifier after 'set'");
    }
    if (!match(TokenType::AS)) {
        throw ParserError(peek(), "Expected 'as' after identifier");
    }
    ExprPtr value = parseExpr();
    match(TokenType::NEWLINE);
    return std::make_unique<AssignmentStmt>(name, std::move(value));
}

StmtPtr Parser::parseWhenStmt() {
    std::vector<WhenStmt::Branch> branches;
    ExprPtr condition = parseExpr();
    if (!match(TokenType::THEN)) {
        throw ParserError(peek(), "Expected 'then' after condition");
    }
    if (!match(TokenType::INDENT)) {
        throw ParserError(peek(), "Expected indentation after 'then'");
    }
    auto body = parseStmtList();
    branches.emplace_back(std::move(condition), std::move(body));
    while (match(TokenType::DEDENT) && match(TokenType::OTHERWISE)) {
        if (check(TokenType::WHEN)) {
            advance();
            condition = parseExpr();
            if (!match(TokenType::THEN)) {
                throw ParserError(peek(), "Expected 'then' after condition");
            }
            if (!match(TokenType::INDENT)) {
                throw ParserError(peek(), "Expected indentation after 'then'");
            }
            body = parseStmtList();
            branches.emplace_back(std::move(condition), std::move(body));
        } else {
            if (!match(TokenType::INDENT)) {
                throw ParserError(peek(), "Expected indentation after 'otherwise'");
            }
            body = parseStmtList();
            branches.emplace_back(nullptr, std::move(body));
        }
    }
    if (!match(TokenType::END)) {
        throw ParserError(peek(), "Expected 'end' to close 'when' statement");
    }
    match(TokenType::NEWLINE);
    return std::make_unique<WhenStmt>(std::move(branches));
}

StmtPtr Parser::parseSayStmt() {
    ExprPtr expr = parseExpr();
    match(TokenType::NEWLINE);
    return std::make_unique<SayStmt>(std::move(expr));
}

std::vector<StmtPtr> Parser::parseStmtList() {
    std::vector<StmtPtr> statements;
    while (!check(TokenType::DEDENT) && !check(TokenType::END) && !check(TokenType::END_OF_FILE)) {
        if (match(TokenType::NEWLINE)) {
            continue;
        }
        statements.push_back(parseStmt());
    }
    while (match(TokenType::NEWLINE)) {
        // Consume trailing NEWLINE tokens
    }
    return statements;
}
ExprPtr Parser::parseExpr() {
    return parseBinaryExpr();
}

ExprPtr Parser::parseBinaryExpr() {
    ExprPtr left = parsePrimary();
    while (check(TokenType::PLUS) || check(TokenType::MINUS) ||
           check(TokenType::STAR) || check(TokenType::SLASH) ||
           check(TokenType::GREATER) || check(TokenType::LESS) ||
           check(TokenType::EQUAL)) {
        Token op = advance();
        ExprPtr right = parsePrimary();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }
    return left;
}

ExprPtr Parser::parsePrimary() {
    if (match(TokenType::NUMBER)) return std::make_unique<LiteralExpr>(previous());
    if (match(TokenType::STRING)) return std::make_unique<LiteralExpr>(previous());
    if (match(TokenType::IDENTIFIER)) return std::make_unique<VariableExpr>(previous());
    if (match(TokenType::LEFT_PAREN)) {
        ExprPtr expr = parseExpr();
        if (!match(TokenType::RIGHT_PAREN)) {
            throw ParserError(peek(), "Expected ')' after expression");
        }
        return std::make_unique<ParenExpr>(std::move(expr));
    }
    if (match(TokenType::LEFT_BRACKET)) {
        std::vector<ExprPtr> elements;
        if (!check(TokenType::RIGHT_BRACKET)) {
            do {
                elements.push_back(parseExpr());
            } while (match(TokenType::COMMA));
        }
        if (!match(TokenType::RIGHT_BRACKET)) {
            throw ParserError(peek(), "Expected ']' after list elements");
        }
        return std::make_unique<ListLiteralExpr>(std::move(elements));
    }
    if (match(TokenType::LEFT_BRACE)) {
        std::vector<std::pair<ExprPtr, ExprPtr>> pairs;
        if (!check(TokenType::RIGHT_BRACE)) {
            do {
                ExprPtr key = parseExpr();
                if (!match(TokenType::COMMA)) {
                    throw ParserError(peek(), "Expected ',' after dictionary key");
                }
                ExprPtr value = parseExpr();
                pairs.emplace_back(std::move(key), std::move(value));
            } while (match(TokenType::COMMA));
        }
        if (!match(TokenType::RIGHT_BRACE)) {
            throw ParserError(peek(), "Expected '}' after dictionary pairs");
        }
        return std::make_unique<DictLiteralExpr>(std::move(pairs));
    }
    throw ParserError(peek(), "Expected expression");
}

Program Parser::parse() {
    try {
        return parseProgram();
    } catch (const ParserError& e) {
        throw;
    }
}
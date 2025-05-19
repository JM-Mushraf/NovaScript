#include "Parser.h"
#include <stdexcept>
#include <iostream>

namespace MyCustomLang {

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

void Parser::synchronize() {
    while (!isAtEnd()) {
        if (peek().type == TokenType::NEWLINE ||
            peek().type == TokenType::END ||
            peek().type == TokenType::LET ||
            peek().type == TokenType::SET ||
            peek().type == TokenType::MATCH ||
            peek().type == TokenType::REPEAT ||
            peek().type == TokenType::WHEN ||
            peek().type == TokenType::SAY) {
            break;
        }
        advance();
    }
}

Program Parser::parse() {
    try {
        return parseProgram();
    } catch (const ParserError& e) {
        throw;
    }
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
    try {
        if (match(TokenType::LET)) {
            return parseVarDecl();
        }
        if (match(TokenType::SET)) {
            // Check if this is an assignment or index assignment
            Token next = peek();
            if (next.type == TokenType::IDENTIFIER && checkNext(TokenType::AS)) {
                return parseAssignmentStmt();
            }
            return parseVarDecl();
        }
        if (match(TokenType::WHEN)) {
            return parseWhenStmt();
        }
        if (match(TokenType::SAY)) {
            return parseSayStmt();
        }
        if (match(TokenType::MATCH)) {
            return parseMatchStmt();
        }
        if (match(TokenType::REPEAT)) {
            if (match(TokenType::WHILE)) {
                return parseWhileLoop();
            }
            if (match(TokenType::FOR)) {
                return parseForLoop();
            }
            if (match(TokenType::WITH)) {
                return parseWithLoop();
            }
            throw ParserError(peek(), "Expected 'while', 'for', or 'with' after 'repeat'");
        }
        throw ParserError(peek(), "Expected statement (let, set, when, say, match, or repeat)");
    } catch (const ParserError& e) {
        synchronize();
        throw;
    }
}

StmtPtr Parser::parseVarDecl() {
    bool isSet = previous().type == TokenType::SET;

    if (isSet) {
        ExprPtr target = parsePrimary();
        if (!match(TokenType::AS)) {
            throw ParserError(peek(), "Expected 'as' in 'set' statement");
        }
        ExprPtr value = parseExpr();

        Token typeHint = Token(TokenType::NONE, "", 0);
        bool isLong = false;
        if (match(TokenType::AS)) {
            if (match(TokenType::INTEGER)) {
                typeHint = previous();
                if (match(TokenType::LONG)) {
                    isLong = true;
                }
            } else {
                throw ParserError(peek(), "Expected type hint after 'as'");
            }
        }

        if (!match(TokenType::NEWLINE)) {
            throw ParserError(peek(), "Expected newline after 'set' statement");
        }

        if (auto* indexExpr = dynamic_cast<IndexExpr*>(target.get())) {
            return std::make_unique<IndexAssignStmt>(std::move(target), std::move(value));
        }
        if (auto* varExpr = dynamic_cast<VariableExpr*>(target.get())) {
            return std::make_unique<VarDeclStmt>(varExpr->name, std::move(value), typeHint, isLong);
        }
        throw ParserError(previous(), "Invalid target for 'set' statement");
    } else {
        Token name = advance();
        if (name.type != TokenType::IDENTIFIER) {
            throw ParserError(name, "Expected identifier after 'let'");
        }

        if (!match(TokenType::BE) && !match(TokenType::EQUAL)) {
            throw ParserError(peek(), "Expected 'be' or '=' after identifier in 'let' statement");
        }
        ExprPtr init = parseExpr();

        Token typeHint = Token(TokenType::NONE, "", 0);
        bool isLong = false;
        if (match(TokenType::AS)) {
            if (match(TokenType::INTEGER)) {
                typeHint = previous();
                if (match(TokenType::LONG)) {
                    isLong = true;
                }
            } else {
                throw ParserError(peek(), "Expected type hint after 'as'");
            }
        }

        if (!match(TokenType::NEWLINE)) {
            throw ParserError(peek(), "Expected newline after variable declaration");
        }
        return std::make_unique<VarDeclStmt>(name, std::move(init), typeHint, isLong);
    }
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
    if (!match(TokenType::NEWLINE)) {
        throw ParserError(peek(), "Expected newline after assignment");
    }
    return std::make_unique<VarDeclStmt>(name, std::move(value));
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

    while (!check(TokenType::END)) {
        if (!match(TokenType::DEDENT) || !match(TokenType::OTHERWISE)) {
            throw ParserError(peek(), "Expected 'otherwise' after when block");
        }
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

StmtPtr Parser::parseMatchStmt() {
    ExprPtr condition = parseExpr();
    if (!match(TokenType::INDENT)) {
        throw ParserError(peek(), "Expected indentation after 'match' expression");
    }

    std::vector<MatchStmt::Case> cases;
    while (check(TokenType::CASE)) {
        advance();
        ExprPtr pattern = parseExpr();
        if (!match(TokenType::THEN)) {
            throw ParserError(peek(), "Expected 'then' after case pattern");
        }
        if (!match(TokenType::INDENT)) {
            throw ParserError(peek(), "Expected indentation after 'then'");
        }
        auto body = parseStmtList();
        cases.emplace_back(std::move(pattern), std::move(body));
        if (!match(TokenType::DEDENT) && !(check(TokenType::CASE) || check(TokenType::END))) {
            throw ParserError(peek(), "Expected dedent after case block");
        }
    }

    if (cases.empty()) {
        throw ParserError(peek(), "Expected at least one 'case' clause in 'match' statement");
    }

    if (!match(TokenType::END)) {
        throw ParserError(peek(), "Expected 'end' to close 'match' statement");
    }
    match(TokenType::NEWLINE);
    return std::make_unique<MatchStmt>(std::move(condition), std::move(cases));
}

StmtPtr Parser::parseWhileLoop() {
    ExprPtr condition = parseExpr();
    if (!match(TokenType::INDENT)) {
        throw ParserError(peek(), "Expected indentation after while condition");
    }
    auto body = parseStmtList();
    if (!match(TokenType::DEDENT) && !check(TokenType::END)) {
        throw ParserError(peek(), "Expected dedent after while block");
    }
    if (!match(TokenType::END)) {
        throw ParserError(peek(), "Expected 'end' to close while loop");
    }
    match(TokenType::NEWLINE);
    return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

StmtPtr Parser::parseForLoop() {
    Token iterator = advance();
    if (iterator.type != TokenType::IDENTIFIER) {
        throw ParserError(iterator, "Expected identifier after 'for'");
    }
    if (!match(TokenType::FROM)) {
        throw ParserError(peek(), "Expected 'from' in for loop");
    }
    ExprPtr start = parseExpr();
    if (!match(TokenType::TO)) {
        throw ParserError(peek(), "Expected 'to' in for loop");
    }
    ExprPtr end = parseExpr();

    ExprPtr step = nullptr;
    if (match(TokenType::STEP)) {
        step = parseExpr();
    }

    if (!match(TokenType::INDENT)) {
        throw ParserError(peek(), "Expected indentation after for loop");
    }
    auto body = parseStmtList();
    if (!match(TokenType::DEDENT) && !check(TokenType::END)) {
        throw ParserError(peek(), "Expected dedent after for block");
    }
    if (!match(TokenType::END)) {
        throw ParserError(peek(), "Expected 'end' to close for loop");
    }
    match(TokenType::NEWLINE);
    return std::make_unique<ForStmt>(iterator, std::move(start), std::move(end), std::move(step), std::move(body));
}

StmtPtr Parser::parseWithLoop() {
    Token iterator = advance();
    if (iterator.type != TokenType::IDENTIFIER) {
        throw ParserError(iterator, "Expected identifier after 'with'");
    }
    if (!match(TokenType::STARTING)) {
        throw ParserError(peek(), "Expected 'starting' in with loop");
    }
    if (!match(TokenType::AT)) {
        throw ParserError(peek(), "Expected 'at' in with loop");
    }
    ExprPtr start = parseExpr();
    if (!match(TokenType::UNTIL)) {
        throw ParserError(peek(), "Expected 'until' in with loop");
    }
    ExprPtr end = parseExpr();

    ExprPtr step = nullptr;
    if (match(TokenType::STEP)) {
        step = parseExpr();
    }

    if (!match(TokenType::INDENT)) {
        throw ParserError(peek(), "Expected indentation after with loop");
    }
    auto body = parseStmtList();
    if (!match(TokenType::DEDENT) && !check(TokenType::END)) {
        throw ParserError(peek(), "Expected dedent after with block");
    }
    if (!match(TokenType::END)) {
        throw ParserError(peek(), "Expected 'end' to close with loop");
    }
    match(TokenType::NEWLINE);
    return std::make_unique<WithStmt>(iterator, std::move(start), std::move(end), std::move(step), std::move(body));
}

std::vector<StmtPtr> Parser::parseStmtList() {
    std::vector<StmtPtr> statements;
    while (!check(TokenType::DEDENT) && !check(TokenType::END) && !check(TokenType::CASE) && 
           !check(TokenType::OTHERWISE) && !check(TokenType::END_OF_FILE)) {
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
    return parseAssignment();
}

ExprPtr Parser::parseAssignment() {
    ExprPtr expr = parseBinaryExpr();
    if (match(TokenType::SET)) {
        if (dynamic_cast<IndexExpr*>(expr.get())) {
            ExprPtr value = parseExpr();
            return std::make_unique<IndexAssignExpr>(std::move(expr), std::move(value));
        }
        if (auto var = dynamic_cast<VariableExpr*>(expr.get())) {
            ExprPtr value = parseExpr();
            return std::make_unique<AssignExpr>(var->name, std::move(value));
        }
        throw ParserError(peek(), "Invalid assignment target");
    }
    return expr;
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
    if (match(TokenType::NUMBER)) {
        return std::make_unique<LiteralExpr>(previous());
    }
    if (match(TokenType::STRING)) {
        return std::make_unique<LiteralExpr>(previous());
    }
    if (match(TokenType::IDENTIFIER) || match(TokenType::UNDERSCORE)) {
        ExprPtr var = std::make_unique<VariableExpr>(previous());
        if (match(TokenType::LEFT_BRACKET)) {
            return parseIndexExpr(std::move(var));
        }
        return var;
    }
    if (match(TokenType::LEFT_BRACKET)) {
        return parseListLiteral();
    }
    if (match(TokenType::LEFT_BRACE)) {
        return parseDictLiteral();
    }
    if (match(TokenType::LEFT_PAREN)) {
        ExprPtr expr = parseExpr();
        if (!match(TokenType::RIGHT_PAREN)) {
            throw ParserError(peek(), "Expected ')' after expression");
        }
        return std::make_unique<ParenExpr>(std::move(expr));
    }
    throw ParserError(peek(), "Expected expression");
}

ExprPtr Parser::parseListLiteral() {
    std::vector<ExprPtr> elements;
    if (!check(TokenType::RIGHT_BRACKET)) {
        do {
            elements.push_back(parseExpr());
        } while (match(TokenType::COMMA));
    }
    if (!match(TokenType::RIGHT_BRACKET)) {
        throw ParserError(peek(), "Expected ']' after list literal");
    }
    return std::make_unique<ListLiteralExpr>(std::move(elements));
}

ExprPtr Parser::parseDictLiteral() {
    std::vector<std::pair<ExprPtr, ExprPtr>> entries;
    if (!check(TokenType::RIGHT_BRACE)) {
        do {
            ExprPtr key = parseExpr();
            if (!match(TokenType::COLON)) {
                throw ParserError(peek(), "Expected ':' in dict literal");
            }
            ExprPtr value = parseExpr();
            entries.emplace_back(std::move(key), std::move(value));
        } while (match(TokenType::COMMA));
    }
    if (!match(TokenType::RIGHT_BRACE)) {
        throw ParserError(peek(), "Expected '}' after dict literal");
    }
    return std::make_unique<DictLiteralExpr>(std::move(entries));
}

ExprPtr Parser::parseIndexExpr(ExprPtr base) {
    ExprPtr index = parseExpr();
    if (!match(TokenType::RIGHT_BRACKET)) {
        throw ParserError(peek(), "Expected ']' after index expression");
    }
    return std::make_unique<IndexExpr>(std::move(base), std::move(index));
}

} // namespace MyCustomLang
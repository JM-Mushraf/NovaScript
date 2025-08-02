#include "Parser.h"
#include <stdexcept>
#include <iostream>

namespace MyCustomLang {

Parser::Parser(std::vector<Token> t) : tokens(std::move(t)), current(0), symbolTable() {}

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
            peek().type == TokenType::SAY ||
            peek().type == TokenType::CASE ||
            peek().type == TokenType::DEFINE ||
            peek().type == TokenType::CALL || 
            peek().type == TokenType::DEDENT ||
            peek().type == TokenType::TRY ||
            peek().type == TokenType::END_OF_FILE) {
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
        if (match(TokenType::DEDENT)) {
            continue;
        }
        statements.push_back(parseStmt());
        while (match(TokenType::NEWLINE)) {}
    }
    return Program(std::move(statements));
}

StmtPtr Parser::parseStmt() {
    try {
        // Handle leading INDENT (if any)
        if (match(TokenType::INDENT)) {
            // Consume it and continue parsing
        }
        if (match(TokenType::LET)) {
            return parseVarDecl();
        }
        if (match(TokenType::SET)) {
            return parseSetStmt();
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
        if (match(TokenType::THROW)) {
        auto expr = parseExpr();
        return std::make_unique<ThrowStmt>(std::move(expr));
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
        if (match(TokenType::TRY)) {
    return parseTryCatchStmt();
}
        if (match(TokenType::CASE)) {
            throw ParserError(previous(), "Unexpected 'case' outside of match statement");
        }
        if (match(TokenType::DEFINE) && match(TokenType::FUNCTION)) {
            return parseFunctionDef();
        }
        if (match(TokenType::CALL)) {
            return parseCallStmt();
        }
        if (match(TokenType::RETURN)) {
            return parseReturnStmt();
        }
        throw ParserError(peek(), "Expected statement (let, set, when, say, match, or repeat)");
    } catch (const ParserError& e) {
        synchronize();
        throw;
    }
}

StmtPtr Parser::parseVarDecl() {
    Token name = advance();
    if (name.type != TokenType::IDENTIFIER) {
        throw ParserError(name, "Expected identifier after 'let'");
    }
    if (symbolTable.symbolExistsInCurrentScope(name.lexeme)) { // Changed from symbolExists
        throw ParserError(name, "Variable '" + name.lexeme + "' already declared in this scope");
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
        } else if (match(TokenType::STRING)) {
            typeHint = previous();
        } else {
            throw ParserError(peek(), "Expected type hint after 'as'");
        }
    } else if (init && dynamic_cast<LiteralExpr*>(init.get())) {
        auto* literal = dynamic_cast<LiteralExpr*>(init.get());
        if (literal->value.type == TokenType::NUMBER) {
            typeHint = Token(TokenType::INTEGER, "int", name.line);
        } else if (literal->value.type == TokenType::STRING) {
            typeHint = Token(TokenType::STRING, "string", name.line);
        }
    }

    symbolTable.addSymbol(name, typeHint, isLong);

    while (match(TokenType::NEWLINE)) {}

    return std::make_unique<VarDeclStmt>(name, std::move(init), typeHint, isLong);
}
// In Parser::parseSetStmt()
StmtPtr Parser::parseSetStmt() {
    Token name = advance();
    if (name.type != TokenType::IDENTIFIER) {
        throw ParserError(name, "Expected identifier after 'set'");
    }
    if (!symbolTable.symbolExists(name.lexeme)) {
        throw ParserError(name, "Variable '" + name.lexeme + "' not declared");
    }
    
    // Handle index assignment (list[index] = value)
    if (match(TokenType::LEFT_BRACKET)) {
        ExprPtr index = parseExpr();
        if (!match(TokenType::RIGHT_BRACKET)) {
            throw ParserError(peek(), "Expected ']' after index");
        }
        if (!match(TokenType::EQUAL)) {
            throw ParserError(peek(), "Expected '=' after index expression");
        }
        ExprPtr value = parseExpr();
        ExprPtr target = std::make_unique<VariableExpr>(name);
        ExprPtr indexExpr = std::make_unique<IndexExpr>(std::move(target), std::move(index));
        return std::make_unique<IndexAssignStmt>(std::move(indexExpr), std::move(value));
    }
    
    // Regular assignment
    if (!match(TokenType::EQUAL)) {
        throw ParserError(peek(), "Expected '=' after identifier in 'set' statement");
    }
    ExprPtr value = parseExpr();
    while (match(TokenType::NEWLINE)) {}
    return std::make_unique<SetStmt>(name, std::move(value));
}

StmtPtr Parser::parseWhenStmt() {
    std::vector<WhenStmt::Branch> branches;
    
    // Parse the first condition in the current (outer) scope
    ExprPtr condition = parseExpr();
    if (!match(TokenType::THEN)) {
        throw ParserError(peek(), "Expected 'then' after condition");
    }
    if (!match(TokenType::INDENT)) {
        throw ParserError(peek(), "Expected indentation after 'then'");
    }
    
    // Enter a new scope for the first branch's body
    symbolTable.enterScope();
    auto body = parseStmtList();
    symbolTable.exitScope();
    branches.emplace_back(std::move(condition), std::move(body));

    // Handle additional branches (otherwise when, otherwise)
    while (check(TokenType::DEDENT) && checkNext(TokenType::OTHERWISE)) {
        match(TokenType::DEDENT);
        match(TokenType::OTHERWISE);
        if (check(TokenType::WHEN)) {
            advance();
            // Parse the condition in the current (outer) scope
            condition = parseBinaryExpr();
            if (!match(TokenType::THEN)) {
                throw ParserError(peek(), "Expected 'then' after condition");
            }
            if (!match(TokenType::INDENT)) {
                throw ParserError(peek(), "Expected indentation after 'then'");
            }
            // Enter a new scope for this branch's body
            symbolTable.enterScope();
            body = parseStmtList();
            symbolTable.exitScope();
            branches.emplace_back(std::move(condition), std::move(body));
        } else {
            if (!match(TokenType::INDENT)) {
                throw ParserError(peek(), "Expected indentation after 'otherwise'");
            }
            // Enter a new scope for the 'otherwise' branch's body
            symbolTable.enterScope();
            body = parseStmtList();
            symbolTable.exitScope();
            branches.emplace_back(nullptr, std::move(body));
        }
    }

    if (!match(TokenType::DEDENT)) {
        throw ParserError(peek(), "Expected dedent before 'end' of 'when' statement");
    }
    if (!match(TokenType::END)) {
        throw ParserError(peek(), "Expected 'end' to close 'when' statement");
    }
    while (match(TokenType::NEWLINE)) {}
    return std::make_unique<WhenStmt>(std::move(branches));
}

StmtPtr Parser::parseSayStmt() {
    ExprPtr expr = parseExpr();
    while (match(TokenType::NEWLINE)) {}
    return std::make_unique<SayStmt>(std::move(expr));
}

StmtPtr Parser::parseMatchStmt() {
    symbolTable.enterScope();
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
        std::vector<StmtPtr> body;
        if (match(TokenType::INDENT)) {
            body = parseStmtList();
            if (!match(TokenType::DEDENT) && !(check(TokenType::CASE) || check(TokenType::END))) {
                throw ParserError(peek(), "Expected dedent after case block");
            }
        } else {
            body.push_back(parseStmt());
        }
        cases.emplace_back(std::move(pattern), std::move(body));
    }

    if (cases.empty()) {
        throw ParserError(peek(), "Expected at least one 'case' clause in 'match' statement");
    }

    if (!match(TokenType::DEDENT) && !check(TokenType::END)) {
        throw ParserError(peek(), "Expected dedent before 'end' of 'match' statement");
    }
    if (!match(TokenType::END)) {
        throw ParserError(peek(), "Expected 'end' to close 'match' statement");
    }
    symbolTable.exitScope();
    while (match(TokenType::NEWLINE)) {}
    return std::make_unique<MatchStmt>(std::move(condition), std::move(cases));
}

StmtPtr Parser::parseWhileLoop() {
    symbolTable.enterScope();
    ExprPtr condition = parseExpr();
    if (!match(TokenType::INDENT)) {
        throw ParserError(peek(), "Expected indentation after while condition");
    }
    auto body = parseStmtList();
    if (!match(TokenType::DEDENT)) {
        throw ParserError(peek(), "Expected dedent after while block");
    }
    if (!match(TokenType::END)) {
        throw ParserError(peek(), "Expected 'end' to close while loop");
    }
    symbolTable.exitScope();
    while (match(TokenType::NEWLINE)) {}
    return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

StmtPtr Parser::parseForLoop() {
    symbolTable.enterScope();
    Token iterator = advance();
    if (iterator.type != TokenType::IDENTIFIER) {
        throw ParserError(iterator, "Expected identifier after 'for'");
    }
    symbolTable.addSymbol(iterator, Token(TokenType::INTEGER, "int", iterator.line), false, {});
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
    symbolTable.exitScope();
    while (match(TokenType::NEWLINE)) {}
    return std::make_unique<ForStmt>(iterator, std::move(start), std::move(end), std::move(step), std::move(body));
}

StmtPtr Parser::parseWithLoop() {
    symbolTable.enterScope();
    Token iterator = advance();
    if (iterator.type != TokenType::IDENTIFIER) {
        throw ParserError(iterator, "Expected identifier after 'with'");
    }
    symbolTable.addSymbol(iterator, Token(TokenType::INTEGER, "int", iterator.line), false, {});
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
    symbolTable.exitScope();
    while (match(TokenType::NEWLINE)) {}
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
    while (match(TokenType::NEWLINE)) {}
    return statements;
}

ExprPtr Parser::parseExpr() {
    return parseBinaryExpr();
}

ExprPtr Parser::parseAssignment() {
    ExprPtr expr = parseBinaryExpr();
    if (match(TokenType::SET)) {
        if (auto* indexExpr = dynamic_cast<IndexExpr*>(expr.get())) {
            ExprPtr value = parseExpr();
            return std::make_unique<IndexAssignExpr>(std::move(expr), std::move(value));
        }
        if (auto* varExpr = dynamic_cast<VariableExpr*>(expr.get())) {
            if (!symbolTable.symbolExists(varExpr->name.lexeme)) {
                throw ParserError(varExpr->name, "Variable '" + varExpr->name.lexeme + "' not declared");
            }
            ExprPtr value = parseExpr();
            return std::make_unique<AssignExpr>(varExpr->name, std::move(value));
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
           check(TokenType::GREATER_EQUAL) || check(TokenType::LESS_EQUAL) ||
           check(TokenType::NOT_EQUAL) || check(TokenType::EQUAL)) {
        Token op = advance();
        if (op.type == TokenType::EQUAL && check(TokenType::EQUAL)) {
            advance();
            op = Token(TokenType::EQUAL_EQUAL, "==", op.line);
        } else if (op.type == TokenType::EQUAL) {
            throw ParserError(op, "Single '=' is not a valid operator. Use '==' for equality.");
        }
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
    if (match(TokenType::MINUS)) {
        if (match(TokenType::NUMBER)) {
            Token number = previous();
            std::string lexeme = "-" + number.lexeme;
            return std::make_unique<LiteralExpr>(Token(TokenType::NUMBER, lexeme, number.line));
        }
        throw ParserError(peek(), "Expected number after unary minus");
    }
    if (match(TokenType::IDENTIFIER) || match(TokenType::UNDERSCORE)) {
        Token name = previous();
        if (check(TokenType::LEFT_PAREN)) {
            advance(); // Consume LEFT_PAREN
            std::vector<ExprPtr> arguments;
            if (!symbolTable.symbolExists(name.lexeme)) {
                throw ParserError(name, "Function '" + name.lexeme + "' not declared");
            }
            Symbol symbol = symbolTable.getSymbol(name.lexeme);
            if (symbol.type != Type::FUNCTION) { // Fixed: Use symbol.type instead of symbol.typeHint.type
                throw ParserError(name, "'" + name.lexeme + "' is not a function");
            }
            if (!check(TokenType::RIGHT_PAREN)) {
                do {
                    arguments.push_back(parseExpr());
                } while (match(TokenType::COMMA));
            }
            if (!match(TokenType::RIGHT_PAREN)) {
                throw ParserError(peek(), "Expected ')' after function arguments");
            }
            return std::make_unique<CallExpr>(name, std::move(arguments));
        }
        if (!symbolTable.symbolExists(name.lexeme)) {
            throw ParserError(name, "Variable or function '" + name.lexeme + "' not declared");
        }
        ExprPtr var = std::make_unique<VariableExpr>(name);
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
    if (match(TokenType::CALL)) {
    Token name = advance();
    if (name.type != TokenType::IDENTIFIER) {
        throw ParserError(name, "Expected function name after 'call'");
    }
    if (!match(TokenType::LEFT_PAREN)) {
        throw ParserError(peek(), "Expected '(' after function name");
    }
    std::vector<ExprPtr> arguments;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            arguments.push_back(parseExpr());
        } while (match(TokenType::COMMA));
    }
    if (!match(TokenType::RIGHT_PAREN)) {
        throw ParserError(peek(), "Expected ')' after arguments");
    }
    return std::make_unique<CallExpr>(name, std::move(arguments));
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

StmtPtr Parser::parseTryCatchStmt() {
    symbolTable.enterScope();
    if (!match(TokenType::INDENT)) {
        throw ParserError(peek(), "Expected indentation after 'try'");
    }
    auto tryBody = parseStmtList();
    if (!match(TokenType::DEDENT)) {
        throw ParserError(peek(), "Expected dedent after try block");
    }
    if (!match(TokenType::CATCH)) {
        throw ParserError(peek(), "Expected 'catch' after try block");
    }
    Token exceptionVar = advance();
    if (exceptionVar.type != TokenType::IDENTIFIER) {
        throw ParserError(exceptionVar, "Expected identifier for exception variable after 'catch'");
    }
    symbolTable.addSymbol(exceptionVar, Token(TokenType::STRING, "string", exceptionVar.line), false);
    if (!match(TokenType::INDENT)) {
        throw ParserError(peek(), "Expected indentation after 'catch'");
    }
    auto catchBody = parseStmtList();
    if (!match(TokenType::DEDENT)) {
        throw ParserError(peek(), "Expected dedent after catch block");
    }
    if (!match(TokenType::END)) {
        throw ParserError(peek(), "Expected 'end' to close try-catch statement");
    }
    symbolTable.exitScope();
    while (match(TokenType::NEWLINE)) {}
    return std::make_unique<TryCatchStmt>(std::move(tryBody), exceptionVar, std::move(catchBody));
}

StmtPtr Parser::parseFunctionDef() {
    Token name = advance();
    if (name.type != TokenType::IDENTIFIER) {
        throw ParserError(name, "Expected function name after 'define function'");
    }
    if (symbolTable.symbolExists(name.lexeme)) {
        throw ParserError(name, "Function '" + name.lexeme + "' already declared in this scope");
    }
    std::vector<Token> parameters;
    if (!match(TokenType::LEFT_PAREN)) {
        throw ParserError(peek(), "Expected '(' after function name");
    }
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            Token param = advance();
            if (param.type != TokenType::IDENTIFIER) {
                throw ParserError(param, "Expected parameter name");
            }
            parameters.push_back(param);
        } while (match(TokenType::COMMA));
    }
    if (!match(TokenType::RIGHT_PAREN)) {
        throw ParserError(peek(), "Expected ')' after parameters");
    }
    symbolTable.addSymbol(name, Type::FUNCTION, false, parameters); // Use Type::FUNCTION
    symbolTable.enterScope();
    for (const auto& param : parameters) {
    symbolTable.addSymbol(param, Type::INTEGER, false, {});
}
    if (!match(TokenType::INDENT)) {
        throw ParserError(peek(), "Expected indentation after function definition");
    }
    auto body = parseStmtList();
    if (!match(TokenType::DEDENT)) {
        throw ParserError(peek(), "Expected dedent after function block");
    }
    if (!match(TokenType::END)) {
        throw ParserError(peek(), "Expected 'end' to close function definition");
    }
    symbolTable.exitScope();
    while (match(TokenType::NEWLINE)) {}
    return std::make_unique<FunctionDefStmt>(name, std::move(parameters), std::move(body));
}
ExprPtr Parser::parseCallExpr() {
    Token name = previous();
    std::vector<ExprPtr> arguments;
    if (!symbolTable.symbolExists(name.lexeme)) {
        throw ParserError(name, "Function '" + name.lexeme + "' not declared");
    }
    Symbol symbol = symbolTable.getSymbol(name.lexeme);
    if (symbol.type != Type::FUNCTION) { // Fixed: Use symbol.type
        throw ParserError(name, "'" + name.lexeme + "' is not a function");
    }
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            arguments.push_back(parseExpr());
        } while (match(TokenType::COMMA));
    }
    if (!match(TokenType::RIGHT_PAREN)) {
        throw ParserError(peek(), "Expected ')' after arguments");
    }
    return std::make_unique<CallExpr>(name, std::move(arguments));
}
StmtPtr Parser::parseCallStmt() {
    Token name = advance();
    if (name.type != TokenType::IDENTIFIER) {
        throw ParserError(name, "Expected function name after 'call'");
    }
    std::vector<ExprPtr> arguments;
    if (!symbolTable.symbolExists(name.lexeme)) {
        throw ParserError(name, "Function '" + name.lexeme + "' not declared");
    }
    Symbol symbol = symbolTable.getSymbol(name.lexeme);
    if (symbol.type != Type::FUNCTION) { // Fixed: Use symbol.type
        throw ParserError(name, "'" + name.lexeme + "' is not a function");
    }
    if (!match(TokenType::LEFT_PAREN)) {
        throw ParserError(peek(), "Expected '(' after function name in 'call'");
    }
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            arguments.push_back(parseExpr());
        } while (match(TokenType::COMMA));
    }
    if (!match(TokenType::RIGHT_PAREN)) {
        throw ParserError(peek(), "Expected ')' after arguments");
    }
    while (match(TokenType::NEWLINE)) {}
    return std::make_unique<CallStmt>(name, std::move(arguments));
}

StmtPtr Parser::parseReturnStmt() {
    Token token = previous();
    ExprPtr value = nullptr;
    if (!check(TokenType::NEWLINE) && !check(TokenType::DEDENT) && !check(TokenType::END)) {
        value = parseExpr();
    }
    while (match(TokenType::NEWLINE)) {}
    return std::make_unique<ReturnStmt>(std::move(value));
}
} // namespace MyCustomLang

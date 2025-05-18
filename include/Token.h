#ifndef TOKEN_H
#define TOKEN_H

#include <string>

namespace MyCustomLang {

enum class TokenType {
    // Keywords
    LET, SET, BE, AS,
    SAY,
    WHEN, THEN, OTHERWISE,
    MATCH, CASE,
    REPEAT, WHILE, FOR, FROM, TO, UNTIL, STEP, STARTING, IN, AT,
    DEFINE, FUNCTION, CALL, RETURN, THROW,
    END, INCREASE, BY, WITH,
    CREATE, MODEL, TRY, CATCH, OPEN, FILE,
    NEWLINE, INDENT, DEDENT,

    // Literals
    IDENTIFIER, NUMBER, STRING,

    // Operators
    PLUS, MINUS, STAR, SLASH, EQUAL, GREATER, LESS,
    UNDERSCORE,

    // Punctuation
    LEFT_PAREN, RIGHT_PAREN,
    LEFT_BRACE, RIGHT_BRACE,
    LEFT_BRACKET, RIGHT_BRACKET,
    SEMICOLON, COMMA,

    // Special
    UNKNOWN, END_OF_FILE
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;

    Token(TokenType t = TokenType::UNKNOWN, std::string l = "", int ln = 0)
        : type(t), lexeme(std::move(l)), line(ln) {}
};

inline std::string tokenTypeToString(TokenType type) {
    switch (type) {
        // Keywords
        case TokenType::LET: return "LET";
        case TokenType::SET: return "SET";
        case TokenType::BE: return "BE";
        case TokenType::AS: return "AS";
        case TokenType::SAY: return "SAY";
        case TokenType::WHEN: return "WHEN";
        case TokenType::THEN: return "THEN";
        case TokenType::OTHERWISE: return "OTHERWISE";
        case TokenType::MATCH: return "MATCH";
        case TokenType::CASE: return "CASE";
        case TokenType::REPEAT: return "REPEAT";
        case TokenType::WHILE: return "WHILE";
        case TokenType::FOR: return "FOR";
        case TokenType::FROM: return "FROM";
        case TokenType::TO: return "TO";
        case TokenType::UNTIL: return "UNTIL";
        case TokenType::STEP: return "STEP";
        case TokenType::STARTING: return "STARTING";
        case TokenType::IN: return "IN";
        case TokenType::AT: return "AT";
        case TokenType::DEFINE: return "DEFINE";
        case TokenType::FUNCTION: return "FUNCTION";
        case TokenType::CALL: return "CALL";
        case TokenType::RETURN: return "RETURN";
        case TokenType::THROW: return "THROW";
        case TokenType::END: return "END";
        case TokenType::INCREASE: return "INCREASE";
        case TokenType::BY: return "BY";
        case TokenType::WITH: return "WITH";
        case TokenType::CREATE: return "CREATE";
        case TokenType::MODEL: return "MODEL";
        case TokenType::TRY: return "TRY";
        case TokenType::CATCH: return "CATCH";
        case TokenType::OPEN: return "OPEN";
        case TokenType::FILE: return "FILE";
        case TokenType::NEWLINE: return "NEWLINE";
        case TokenType::INDENT: return "INDENT";
        case TokenType::DEDENT: return "DEDENT";

        // Literals
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::NUMBER: return "NUMBER";
        case TokenType::STRING: return "STRING";

        // Operators
        case TokenType::PLUS: return "PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::STAR: return "STAR";
        case TokenType::SLASH: return "SLASH";
        case TokenType::EQUAL: return "EQUAL";
        case TokenType::GREATER: return "GREATER";
        case TokenType::LESS: return "LESS";
        case TokenType::UNDERSCORE: return "UNDERSCORE";

        // Punctuation
        case TokenType::LEFT_PAREN: return "LEFT_PAREN";
        case TokenType::RIGHT_PAREN: return "RIGHT_PAREN";
        case TokenType::LEFT_BRACE: return "LEFT_BRACE";
        case TokenType::RIGHT_BRACE: return "RIGHT_BRACE";
        case TokenType::LEFT_BRACKET: return "LEFT_BRACKET";
        case TokenType::RIGHT_BRACKET: return "RIGHT_BRACKET";
        case TokenType::SEMICOLON: return "SEMICOLON";
        case TokenType::COMMA: return "COMMA";

        // Special
        case TokenType::UNKNOWN: return "UNKNOWN";
        case TokenType::END_OF_FILE: return "END_OF_FILE";

        default: return "UNDEFINED";
    }
}

} // namespace MyCustomLang

#endif
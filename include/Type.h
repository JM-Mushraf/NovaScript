#ifndef MYCUSTOMLANG_TYPE_H
#define MYCUSTOMLANG_TYPE_H

namespace MyCustomLang {

enum class Type {
    NONE,
    INTEGER,
    STRING,
    LIST,
    DICT,
    FUNCTION,
    ERROR
};

// Inline function to convert Type enum to string
inline std::string typeToString(Type type) {
    switch (type) {
        case Type::NONE: return "NONE";
        case Type::INTEGER: return "INTEGER";
        case Type::STRING: return "STRING";
        case Type::LIST: return "LIST";
        case Type::DICT: return "DICT";
        case Type::FUNCTION: return "FUNCTION";
        case Type::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

} // namespace MyCustomLang

#endif // MYCUSTOMLANG_TYPE_H
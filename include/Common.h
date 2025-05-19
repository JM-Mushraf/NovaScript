#ifndef COMMON_H
#define COMMON_H

#include <memory>

namespace MyCustomLang {

class Expr;
class Stmt;
using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;

} // namespace MyCustomLang

#endif
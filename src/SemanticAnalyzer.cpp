#include "SemanticAnalyzer.h"
#include "AST.h"

namespace MyCustomLang {

void SemanticAnalyzer::analyze(Program& program) {
    for (auto& stmt : program.statements) {
        analyzeStmt(stmt.get());
    }
}

void SemanticAnalyzer::analyzeStmt(Stmt* stmt) {
    if (auto* varDecl = dynamic_cast<VarDeclStmt*>(stmt)) {
        if (varDecl->init) {
            analyzeExpr(varDecl->init.get());
            Type initType = varDecl->init->inferredType;
            if (varDecl->typeHint.type != TokenType::NONE) {
                varDecl->declaredType = (varDecl->typeHint.type == TokenType::INTEGER) ? Type::INTEGER : Type::STRING;
                checkTypeCompatibility(varDecl->declaredType, initType, varDecl->name);
            } else {
                varDecl->declaredType = initType;
            }
            symbolTable.updateSymbolType(varDecl->name.lexeme, varDecl->declaredType);
        }
    } else if (auto* setStmt = dynamic_cast<SetStmt*>(stmt)) {
        analyzeExpr(setStmt->value.get());
        Type valueType = setStmt->value->inferredType;
        Symbol sym = symbolTable.getSymbol(setStmt->name.lexeme);
        checkTypeCompatibility(sym.type, valueType, setStmt->name);
        symbolTable.updateSymbolType(setStmt->name.lexeme, valueType);
    } else if (auto* whenStmt = dynamic_cast<WhenStmt*>(stmt)) {
        for (auto& branch : whenStmt->branches) {
            if (branch.condition) {
                analyzeExpr(branch.condition.get());
                if (branch.condition->inferredType != Type::INTEGER) {
                    throw SemanticError(branch.condition->getToken(), "Condition must be an integer (boolean-like)");
                }
            }
            for (auto& s : branch.body) {
                analyzeStmt(s.get());
            }
        }
    } else if (auto* whileStmt = dynamic_cast<WhileStmt*>(stmt)) {
        analyzeExpr(whileStmt->condition.get());
        if (whileStmt->condition->inferredType != Type::INTEGER) {
            throw SemanticError(whileStmt->condition->getToken(), "While condition must be an integer (boolean-like)");
        }
        for (auto& s : whileStmt->body) {
            analyzeStmt(s.get());
        }
    } else if (auto* forStmt = dynamic_cast<ForStmt*>(stmt)) {
        analyzeExpr(forStmt->start.get());
        analyzeExpr(forStmt->end.get());
        if (forStmt->start->inferredType != Type::INTEGER || forStmt->end->inferredType != Type::INTEGER) {
            throw SemanticError(forStmt->iterator, "For loop start and end must be integers");
        }
        if (forStmt->step) {
            analyzeExpr(forStmt->step.get());
            if (forStmt->step->inferredType != Type::INTEGER) {
                throw SemanticError(forStmt->iterator, "For loop step must be an integer");
            }
        }
        symbolTable.enterScope();
        symbolTable.addSymbol(forStmt->iterator, Type::INTEGER, false);
        for (auto& s : forStmt->body) {
            analyzeStmt(s.get());
        }
        symbolTable.exitScope();
    } else if (auto* withStmt = dynamic_cast<WithStmt*>(stmt)) {
        analyzeExpr(withStmt->start.get());
        analyzeExpr(withStmt->end.get());
        if (withStmt->start->inferredType != Type::INTEGER || withStmt->end->inferredType != Type::INTEGER) {
            throw SemanticError(withStmt->iterator, "With loop start and end must be integers");
        }
        if (withStmt->step) {
            analyzeExpr(withStmt->step.get());
            if (withStmt->step->inferredType != Type::INTEGER) {
                throw SemanticError(withStmt->iterator, "With loop step must be an integer");
            }
        }
        symbolTable.enterScope();
        symbolTable.addSymbol(withStmt->iterator, Type::INTEGER, false);
        for (auto& s : withStmt->body) {
            analyzeStmt(s.get());
        }
        symbolTable.exitScope();
    } else if (auto* funcDef = dynamic_cast<FunctionDefStmt*>(stmt)) {
    symbolTable.enterScope();
    for (const auto& param : funcDef->parameters) {
        symbolTable.addSymbol(param, Type::INTEGER, false); // Set INTEGER for parameters
    }
    Type inferredReturnType = Type::NONE;
    for (auto& s : funcDef->body) {
        analyzeStmt(s.get());
        if (auto* returnStmt = dynamic_cast<ReturnStmt*>(s.get())) {
            if (returnStmt->value) {
                analyzeExpr(returnStmt->value.get()); // Ensure expression is analyzed
                Type returnType = returnStmt->value->inferredType;
                returnStmt->returnType = returnType;
                if (inferredReturnType == Type::NONE) {
                    inferredReturnType = returnType;
                } else if (returnType != Type::NONE && returnType != inferredReturnType) {
                    throw SemanticError(returnStmt->value->getToken(), "Inconsistent return type in function");
                }
            }
        }
    }
    symbolTable.updateSymbolReturnType(funcDef->name.lexeme, inferredReturnType);
    symbolTable.updateSymbolReturnType(funcDef->name.lexeme, inferredReturnType);
    symbolTable.exitScope();
} else if (auto* callStmt = dynamic_cast<CallStmt*>(stmt)) {
        Symbol sym = symbolTable.getSymbol(callStmt->name.lexeme);
        if (sym.type != Type::FUNCTION) {
            throw SemanticError(callStmt->name, "'" + callStmt->name.lexeme + "' is not a function");
        }
        if (sym.parameters.size() != callStmt->arguments.size()) {
            throw SemanticError(callStmt->name, "Incorrect number of arguments for function '" + callStmt->name.lexeme + "'");
        }
        for (auto& arg : callStmt->arguments) {
            analyzeExpr(arg.get());
        }
    } else if (auto* returnStmt = dynamic_cast<ReturnStmt*>(stmt)) {
        if (returnStmt->value) {
            analyzeExpr(returnStmt->value.get());
            returnStmt->returnType = returnStmt->value->inferredType;
        }
    } else if (auto* throwStmt = dynamic_cast<ThrowStmt*>(stmt)) {
        analyzeExpr(throwStmt->expr.get());
        if (throwStmt->expr->inferredType != Type::STRING) {
            throw SemanticError(throwStmt->expr->getToken(), "Throw expression must be a string");
        }
    } else if (auto* tryCatch = dynamic_cast<TryCatchStmt*>(stmt)) {
        for (auto& s : tryCatch->tryBody) {
            analyzeStmt(s.get());
        }
        symbolTable.enterScope();
        symbolTable.addSymbol(tryCatch->exceptionVar, Type::STRING, false);
        for (auto& s : tryCatch->catchBody) {
            analyzeStmt(s.get());
        }
        symbolTable.exitScope();
    } else if (auto* matchStmt = dynamic_cast<MatchStmt*>(stmt)) {
        analyzeExpr(matchStmt->condition.get());
        for (auto& case_ : matchStmt->cases) {
            analyzeExpr(case_.pattern.get());
            checkTypeCompatibility(matchStmt->condition->inferredType, case_.pattern->inferredType, case_.pattern->getToken());
            for (auto& s : case_.body) {
                analyzeStmt(s.get());
            }
        }
    } else if (auto* indexAssign = dynamic_cast<IndexAssignStmt*>(stmt)) {
        analyzeExpr(indexAssign->target.get());
        analyzeExpr(indexAssign->value.get());
        if (indexAssign->target->inferredType != Type::LIST && indexAssign->target->inferredType != Type::DICT) {
            throw SemanticError(indexAssign->target->getToken(), "Index target must be a list or dictionary");
        }
    }
}

void SemanticAnalyzer::analyzeExpr(Expr* expr) {
    expr->inferredType = inferExprType(expr);
}

Type SemanticAnalyzer::inferExprType(Expr* expr) {
    if (auto* literal = dynamic_cast<LiteralExpr*>(expr)) {
        if (literal->value.type == TokenType::NUMBER) return Type::INTEGER;
        if (literal->value.type == TokenType::STRING) return Type::STRING;
        return Type::ERROR;
    } else if (auto* binary = dynamic_cast<BinaryExpr*>(expr)) {
        Type leftType = inferExprType(binary->left.get());
        Type rightType = inferExprType(binary->right.get());
        switch (binary->op.type) {
            case TokenType::PLUS:
            case TokenType::MINUS:
            case TokenType::STAR:
            case TokenType::SLASH:
                if (leftType == Type::INTEGER && rightType == Type::INTEGER) {
                    return Type::INTEGER;
                }
                if (leftType != Type::INTEGER) {
                    throw SemanticError(binary->left->getToken(), "Left operand must be an integer");
                }
                binary->left->inferredType = Type::INTEGER;
                if (rightType != Type::INTEGER) {
                    throw SemanticError(binary->right->getToken(), "Right operand must be an integer");
                }
                binary->right->inferredType = Type::INTEGER;
                return Type::ERROR;
            case TokenType::GREATER:
            case TokenType::LESS:
            case TokenType::GREATER_EQUAL:
            case TokenType::LESS_EQUAL:
            case TokenType::EQUAL_EQUAL:
            case TokenType::NOT_EQUAL:
                // Disallow comparisons if types are NONE
                if (leftType == Type::NONE || rightType == Type::NONE) {
                    // If the operand is a variable, infer its type as INTEGER and update the symbol table
                    if (leftType == Type::NONE) {
                        if (auto* var = dynamic_cast<VariableExpr*>(binary->left.get())) {
                            symbolTable.updateSymbolType(var->name.lexeme, Type::INTEGER);
                            leftType = Type::INTEGER;
                            binary->left->inferredType = Type::INTEGER;
                        }
                    }
                    if (rightType == Type::NONE) {
                        if (auto* var = dynamic_cast<VariableExpr*>(binary->right.get())) {
                            symbolTable.updateSymbolType(var->name.lexeme, Type::INTEGER);
                            rightType = Type::INTEGER;
                            binary->right->inferredType = Type::INTEGER;
                        }
                    }
                    // If still NONE, throw an error
                    if (leftType == Type::NONE || rightType == Type::NONE) {
                        throw SemanticError(binary->op, "Cannot compare operands with unknown types");
                    }
                }
                if (leftType == rightType) {
                    return Type::INTEGER; // Boolean-like result
                }
                throw SemanticError(binary->op, "Operands must have the same type for comparison");
            default:
                return Type::ERROR;
        }
    } else if (auto* paren = dynamic_cast<ParenExpr*>(expr)) {
        return inferExprType(paren->expr.get());
    } else if (auto* list = dynamic_cast<ListLiteralExpr*>(expr)) {
        Type elementType = Type::NONE;
        for (auto& elem : list->elements) {
            Type type = inferExprType(elem.get());
            if (elementType == Type::NONE) {
                elementType = type;
            } else if (elem->inferredType != elementType && elem->inferredType != Type::NONE) {
                throw SemanticError(elem->getToken(), "All elements in a list must have the same type");
            }
        }
        return Type::LIST;
    } else if (auto* dict = dynamic_cast<DictLiteralExpr*>(expr)) {
        Type keyType = Type::NONE;
        Type valueType = Type::NONE;
        for (auto& entry : dict->entries) {
            Type kType = inferExprType(entry.first.get());
            if (keyType == Type::NONE) {
                keyType = kType;
            } else if (entry.first->inferredType != keyType && entry.first->inferredType != Type::NONE) {
                throw SemanticError(entry.first->getToken(), "All keys in a dictionary must have the same type");
            }
            Type vType = inferExprType(entry.second.get());
            if (valueType == Type::NONE) {
                valueType = vType;
            } else if (entry.second->inferredType != valueType && entry.second->inferredType != Type::NONE) {
                throw SemanticError(entry.second->getToken(), "All values in a dictionary must have the same type");
            }
        }
        return Type::DICT;
    } else if (auto* index = dynamic_cast<IndexExpr*>(expr)) {
        Type baseType = inferExprType(index->base.get());
        Type indexType = inferExprType(index->index.get());
        if (baseType != Type::LIST && baseType != Type::DICT) {
            throw SemanticError(index->base->getToken(), "Index base must be a list or dictionary");
        }
        if (indexType != Type::INTEGER) {
            throw SemanticError(index->index->getToken(), "Index must be an integer");
        }
        if (baseType == Type::LIST) {
            return Type::INTEGER; // Assume list elements are integers for simplicity
        }
        return Type::INTEGER; // Assume dict values are integers for simplicity
    } else if (auto* call = dynamic_cast<CallExpr*>(expr)) {
        Symbol sym = symbolTable.getSymbol(call->name.lexeme);
        if (sym.type != Type::FUNCTION) {
            throw SemanticError(call->name, "'" + call->name.lexeme + "' is not a function");
        }
        if (sym.parameters.size() != call->arguments.size()) {
            throw SemanticError(call->name, "Incorrect number of arguments for function '" + call->name.lexeme + "'");
        }
        for (size_t i = 0; i < call->arguments.size(); ++i) {
            Type argType = inferExprType(call->arguments[i].get());
            // For simplicity, we don't check parameter types here (parameters have Type::NONE)
        }
        return sym.returnType;
    } else if (auto* var = dynamic_cast<VariableExpr*>(expr)) {
        Symbol sym = symbolTable.getSymbol(var->name.lexeme);
        return sym.type;
    } else if (auto* assign = dynamic_cast<AssignExpr*>(expr)) {
        Type valueType = inferExprType(assign->value.get());
        Symbol sym = symbolTable.getSymbol(assign->name.lexeme);
        checkTypeCompatibility(sym.type, assign->value->inferredType, assign->name);
        return valueType;
    } else if (auto* indexAssign = dynamic_cast<IndexAssignExpr*>(expr)) {
        Type targetType = inferExprType(indexAssign->target.get());
        if (indexAssign->target->inferredType != Type::LIST && indexAssign->target->inferredType != Type::DICT) {
            throw SemanticError(indexAssign->target->getToken(), "Index assign target must be a list or dictionary");
        }
        return inferExprType(indexAssign->value.get());
    }
    return Type::ERROR;
}

void SemanticAnalyzer::checkTypeCompatibility(Type expected, Type actual, const Token& token) {
    if (expected == Type::NONE || actual == Type::NONE) return;
    if (expected != actual) {
        throw SemanticError(token, "Type mismatch: expected " + typeToString(expected) + ", got " + typeToString(actual));
    }
}

void SemanticAnalyzer::updateFunctionReturnType(const std::string& funcName, Type returnType) {
    symbolTable.updateSymbolReturnType(funcName, returnType);
}

} // namespace MyCustomLang
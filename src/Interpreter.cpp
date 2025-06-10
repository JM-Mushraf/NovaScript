#include "Interpreter.h"
#include <iostream>

namespace MyCustomLang {

Value Interpreter::evaluateExpr(const Expr* expr) {
    if (auto* lit = dynamic_cast<const LiteralExpr*>(expr)) {
        if (lit->value.type == TokenType::NUMBER) {
            return std::stoll(lit->value.lexeme);
        } else if (lit->value.type == TokenType::STRING) {
            return lit->value.lexeme;
        }
    } else if (auto* var = dynamic_cast<const VariableExpr*>(expr)) {
        return env.get(var->name.lexeme);
    } else if (auto* bin = dynamic_cast<const BinaryExpr*>(expr)) {
        Value left = evaluateExpr(bin->left.get());
        Value right = evaluateExpr(bin->right.get());

        if (std::holds_alternative<int64_t>(left) && std::holds_alternative<int64_t>(right)) {
            int64_t l = std::get<int64_t>(left);
            int64_t r = std::get<int64_t>(right);
            switch (bin->op.type) {
                case TokenType::PLUS: return l + r;
                case TokenType::MINUS: return l - r;
                case TokenType::STAR: return l * r;
                case TokenType::SLASH:
                    if (r == 0) throw std::runtime_error("Division by zero");
                    return l / r;
                case TokenType::GREATER: return l > r ? 1 : 0;
                case TokenType::LESS: return l < r ? 1 : 0;
                case TokenType::GREATER_EQUAL: return l >= r ? 1 : 0;
                case TokenType::LESS_EQUAL: return l <= r ? 1 : 0;
                case TokenType::EQUAL_EQUAL: return l == r ? 1 : 0;
                case TokenType::NOT_EQUAL: return l != r ? 1 : 0;
                default: throw std::runtime_error("Unknown binary operator");
            }
        } else {
            throw std::runtime_error("Type mismatch in binary expression");
        }
    } else if (auto* call = dynamic_cast<const CallExpr*>(expr)) {
        Value funcVal = env.get(call->name.lexeme);
        if (!std::holds_alternative<std::shared_ptr<FunctionDefStmt>>(funcVal)) {
            throw std::runtime_error(call->name.lexeme + " is not a function");
        }
        auto func = std::get<std::shared_ptr<FunctionDefStmt>>(funcVal);

        std::vector<Value> args;
        for (const auto& arg : call->arguments) {
            args.push_back(evaluateExpr(arg.get()));
        }

        if (args.size() != func->parameters.size()) {
            throw std::runtime_error("Function " + call->name.lexeme + " expected " +
                                     std::to_string(func->parameters.size()) + " arguments but got " +
                                     std::to_string(args.size()));
        }

        env.enterScope();
        for (size_t i = 0; i < func->parameters.size(); ++i) {
            env.define(func->parameters[i].lexeme, args[i]);
        }

        try {
            for (const auto& stmt : func->body) {
                executeStmt(stmt.get());
            }
            env.exitScope();
            return Value{};
        } catch (const Value& returnValue) {
            env.exitScope();
            return returnValue;
        }
    } else if (auto* paren = dynamic_cast<const ParenExpr*>(expr)) {
        return evaluateExpr(paren->expr.get());
    }

    throw std::runtime_error("Unknown expression type");
}

void Interpreter::executeStmt(const Stmt* stmt) {
    if (auto* varDecl = dynamic_cast<const VarDeclStmt*>(stmt)) {
        Value value = evaluateExpr(varDecl->init.get());
        env.define(varDecl->name.lexeme, value);
    } else if (auto* setStmt = dynamic_cast<const SetStmt*>(stmt)) {
        Value value = evaluateExpr(setStmt->value.get());
        env.assign(setStmt->name.lexeme, value);
    } else if (auto* sayStmt = dynamic_cast<const SayStmt*>(stmt)) {
        Value value = evaluateExpr(sayStmt->expr.get());
        if (std::holds_alternative<int64_t>(value)) {
            std::cout << std::get<int64_t>(value) << std::endl;
        } else if (std::holds_alternative<std::string>(value)) {
            std::cout << std::get<std::string>(value) << std::endl;
        } else {
            std::cout << "[void]" << std::endl;
        }
    } else if (auto* funcDef = dynamic_cast<const FunctionDefStmt*>(stmt)) {
        env.define(funcDef->name.lexeme, std::make_shared<FunctionDefStmt>(*funcDef));
    } else if (auto* callStmt = dynamic_cast<const CallStmt*>(stmt)) {
        std::vector<ExprPtr> clonedArgs;
        for (const auto& arg : callStmt->arguments) {
            clonedArgs.push_back(arg->clone());
        }
        auto callExpr = std::make_unique<CallExpr>(callStmt->name, std::move(clonedArgs));
        evaluateExpr(callExpr.get());
    } else if (auto* returnStmt = dynamic_cast<const ReturnStmt*>(stmt)) {
        if (returnStmt->value) {
            throw evaluateExpr(returnStmt->value.get());
        }
        throw Value{};
    } else if (auto* whenStmt = dynamic_cast<const WhenStmt*>(stmt)) {
        for (const auto& branch : whenStmt->branches) {
            if (!branch.condition) {
                env.enterScope();
                for (const auto& s : branch.body) {
                    executeStmt(s.get());
                }
                env.exitScope();
                break;
            }
            Value cond = evaluateExpr(branch.condition.get());
            if (!std::holds_alternative<int64_t>(cond)) {
                throw std::runtime_error("Condition must evaluate to an integer");
            }
            if (std::get<int64_t>(cond) != 0) {
                env.enterScope();
                for (const auto& s : branch.body) {
                    executeStmt(s.get());
                }
                env.exitScope();
                break;
            }
        }
    } else if (auto* whileStmt = dynamic_cast<const WhileStmt*>(stmt)) {
        while (true) {
            Value cond = evaluateExpr(whileStmt->condition.get());
            if (!std::holds_alternative<int64_t>(cond)) {
                throw std::runtime_error("Condition must evaluate to an integer");
            }
            if (std::get<int64_t>(cond) == 0) break;
            env.enterScope();
            for (const auto& s : whileStmt->body) {
                executeStmt(s.get());
            }
            env.exitScope();
        }
    } else if (auto* forStmt = dynamic_cast<const ForStmt*>(stmt)) {
        Value startVal = evaluateExpr(forStmt->start.get());
        Value endVal = evaluateExpr(forStmt->end.get());
        Value stepVal = forStmt->step ? evaluateExpr(forStmt->step.get()) : Value(1);

        if (!std::holds_alternative<int64_t>(startVal) || !std::holds_alternative<int64_t>(endVal) ||
            !std::holds_alternative<int64_t>(stepVal)) {
            throw std::runtime_error("For loop bounds and step must be integers");
        }

        int64_t start = std::get<int64_t>(startVal);
        int64_t end = std::get<int64_t>(endVal);
        int64_t step = std::get<int64_t>(stepVal);

        if (step == 0) throw std::runtime_error("Step cannot be zero");

        env.enterScope();
        env.define(forStmt->iterator.lexeme, start);
        if (step > 0) {
            for (int64_t i = start; i <= end; i += step) {
                env.assign(forStmt->iterator.lexeme, i);
                for (const auto& s : forStmt->body) {
                    executeStmt(s.get());
                }
            }
        } else {
            for (int64_t i = start; i >= end; i += step) {
                env.assign(forStmt->iterator.lexeme, i);
                for (const auto& s : forStmt->body) {
                    executeStmt(s.get());
                }
            }
        }
        env.exitScope();
    } else {
        throw std::runtime_error("Unknown statement type");
    }
}

void Interpreter::interpret(const Program& program) {
    for (const auto& stmt : program.statements) {
        executeStmt(stmt.get());
    }
}

} // namespace MyCustomLang
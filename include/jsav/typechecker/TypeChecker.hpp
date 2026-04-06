/*
 * Created by gbian on 2 aprile 2026.
 * Copyright (c) 2026 All rights reserved.
 */
#pragma once

// clang-format off
#include "../headers.hpp"
#include "jsav/ast/Program.hpp"
#include "jsav/ast/TypedProgram.hpp"
#include "jsav/typechecker/Constraint.hpp"
#include "jsav/typechecker/ConstraintSolver.hpp"
#include "jsav/typechecker/SymbolTable.hpp"
// clang-format on

namespace jsv {

/// Type checking result — matches spec.md Output Contract (FR-001)
struct TypeCheckResult {
    TypedProgram program;
    std::vector<CompileError> errors;
};

/**
 * @brief Main type checker class.
 *
 * Transforms a Raw AST into a fully Typed AST using Hindley-Milner
 * style constraint-based type inference.
 *
 * Pipeline:
 *   1. Name resolution (populate symbol table)
 *   2. Constraint generation (traverse AST, emit constraints)
 *   3. Constraint solving (unification)
 *   4. Zonking (apply substitution to typed AST)
 */
class TypeChecker {
public:
    /// Type check a program, producing fully typed AST and error collection
    [[nodiscard]] TypeCheckResult check(const Program& program);

    /// Type a single expression (recursive) — public for unit testing
    [[nodiscard]] TypedExprPtr type_expr(const Expr& expr);

    /// Type a single statement (recursive) — public for unit testing
    [[nodiscard]] TypedStmtPtr type_stmt(const Stmt& stmt);

private:
    /// Phase 1: Name resolution (populate symbol table)
    void resolve_names(const Program& program);
    void resolve_names_stmt(const Stmt& stmt);

    /// Phase 2: Constraint generation (traverse AST, emit constraints)
    void generate_constraints(const Program& program);

    /// Phase 3: Constraint solving (unification)
    [[nodiscard]] SolverResult solve_constraints();

    /// Phase 4: Zonking (apply substitution to typed AST)
    [[nodiscard]] TypedProgram zonk(const Substitution& subst);

    /// Recursive zonking helpers
    [[nodiscard]] TypedStmtPtr zonk_stmt_full(const Substitution& subst, const TypedStmt& stmt);
    [[nodiscard]] TypedExprPtr zonk_expr_full(const Substitution& subst, const TypedExpr& expr);
    [[nodiscard]] std::unique_ptr<TypedBlockStmt> zonk_block_full(const Substitution& subst, const TypedBlockStmt& block);

    SymbolTable symbols_;
    ConstraintSet constraints_;
    std::vector<CompileError> errors_;
    std::vector<std::string> message_storage_;  // Owns dynamic strings for CompileError::message_ (std::string_view)
    std::vector<TypedStmtPtr> typed_stmts_;  // Stored during constraint generation

    /// Tracks the expected return type of the enclosing function (for return statement checking)
    std::optional<TypePtr> current_function_return_type_;
    /// Tracks the name of the enclosing function (for error messages)
    std::optional<std::string> current_function_name_;
    /// Tracks nesting depth inside loops (for break/continue validation)
    int loop_depth_ = 0;
};

}  // namespace jsv

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

    /**
     * @brief Result of type checking a program.
     *
     * Contains the fully typed AST and any errors encountered during type checking.
     * Matches spec.md Output Contract (FR-001).
     *
     * @note If `errors` is non-empty, the `program` may be partially typed or contain
     *       placeholder types where inference failed.
     */
    struct TypeCheckResult {
        TypedProgram program;              ///< The fully typed AST (may be incomplete if errors occurred)
        std::vector<CompileError> errors;  ///< Collection of type errors encountered during checking
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
        /**
         * @brief Type check a complete program.
         *
         * Performs the full type checking pipeline: name resolution, constraint
         * generation, constraint solving (unification), and zonking to produce
         * a fully typed AST.
         *
         * @param program The raw (untyped) program AST to type check.
         * @return TypeCheckResult containing the typed program and any errors.
         *
         * @code
         * TypeChecker checker;
         * auto result = checker.check(parsedProgram);
         * if (result.errors.empty()) {
         *     // Successfully typed
         * }
         * @endcode
         */
        [[nodiscard]] TypeCheckResult check(const Program &program);

        /**
         * @brief Type a single expression (recursive).
         *
         * Exposed publicly for unit testing individual expression typing.
         *
         * @param expr The raw expression to type.
         * @return Typed expression with inferred type annotation.
         */
        [[nodiscard]] TypedExprPtr type_expr(const Expr &expr);

        /**
         * @brief Type a single statement (recursive).
         *
         * Exposed publicly for unit testing individual statement typing.
         *
         * @param stmt The raw statement to type.
         * @return Typed statement with all contained expressions typed.
         */
        [[nodiscard]] TypedStmtPtr type_stmt(const Stmt &stmt);

    private:
        /// Phase 1: Name resolution (populate symbol table)
        void resolve_names(const Program &program);
        void resolve_names_stmt(const Stmt &stmt);

        /// Phase 2: Constraint generation (traverse AST, emit constraints)
        void generate_constraints(const Program &program);

        /// Phase 3: Constraint solving (unification)
        [[nodiscard]] SolverResult solve_constraints() const;

        /// Phase 4: Zonking (apply substitution to typed AST)
        [[nodiscard]] TypedProgram zonk(const Substitution &subst);

        /// Recursive zonking helpers
        [[nodiscard]] TypedStmtPtr zonk_stmt_full(const Substitution &subst, const TypedStmt &stmt);
        [[nodiscard]] TypedExprPtr zonk_expr_full(const Substitution &subst, const TypedExpr &expr);
        [[nodiscard]] std::unique_ptr<TypedBlockStmt> zonk_block_full(const Substitution &subst, const TypedBlockStmt &block);

        SymbolTable symbols_;
        ConstraintSet constraints_;
        std::vector<CompileError> errors_;
        std::deque<std::string> message_storage_;  // Owns dynamic strings for CompileError::message_ (std::string_view); deque prevents
                                                   // view invalidation on reallocation
        std::vector<TypedStmtPtr> typed_stmts_;    // Stored during constraint generation

        /// Tracks nesting depth inside loops (for break/continue validation)
        std::size_t loop_depth_ = 0;
    };

}  // namespace jsv

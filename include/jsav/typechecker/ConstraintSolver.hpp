/*
 * Created by gbian on 2 aprile 2026.
 * Copyright (c) 2026 All rights reserved.
 */
#pragma once

// clang-format off
#include "../headers.hpp"
#include "jsav/error/CompileError.hpp"
#include "jsav/typechecker/Constraint.hpp"
#include "jsav/typechecker/Substitution.hpp"
#include "jsav/typechecker/UnionFind.hpp"
// clang-format on

namespace jsv {

    /**
     * @brief Result of constraint solving.
     *
     * Contains the computed substitution mapping type variables to their
     * resolved types, along with any errors encountered during unification.
     *
     * @code
     * ConstraintSolver solver;
     * SolverResult result = solver.solve(constraints);
     * if (result.errors.empty()) {
     *     // Use result.substitution
     * }
     * @endcode
     */
    struct SolverResult {
        Substitution substitution{};         ///< Variable → Type mappings (empty if solving failed)
        std::vector<CompileError> errors{};  ///< Unification errors (empty on success)
    };

    /**
     * @brief Constraint solver using union-find unification.
     *
     * Solves type constraints by finding a substitution that makes
     * all constraint pairs equal. Reports errors for unresolvable
     * constraints.
     *
     * @code
     * // Typical usage:
     * ConstraintCollector collector;
     * collector.visitAST(ast);
     * ConstraintSolver solver;
     * SolverResult result = solver.solve(collector.constraints());
     * if (!result.errors.empty()) {
     *     for (const auto& error : result.errors) {
     *         reportError(error);
     *     }
     * }
     * @endcode
     */
    class ConstraintSolver {
    public:
        /**
         * @brief Solve all type constraints and produce a unified substitution.
         *
         * Processes each constraint in the set, attempting to unify the types involved.
         * Uses union-find for efficient unification with path compression.
         *
         * @param constraints The set of type constraints to solve.
         * @return SolverResult containing the substitution and any errors encountered.
         *
         * @code
         * ConstraintSet constraints;
         * constraints.add(Constraint{type1, type2, location});
         * ConstraintSolver solver;
         * SolverResult result = solver.solve(constraints);
         * if (result.errors.empty()) {
         *     // Apply result.substitution to resolve types
         * }
         * @endcode
         */
        [[nodiscard]] SolverResult solve(const ConstraintSet &constraints);

        /**
         * @brief Check if a type variable occurs within a type (occurs check).
         *
         * Used to detect infinite/recursive types during unification.
         * If var occurs in type, unifying them would create a cyclic type.
         *
         * @param var The type variable ID to search for.
         * @param type The type to search within.
         * @param subst The current substitution for resolving type variables.
         * @return true if var occurs in type (directly or through substitution), false otherwise.
         *
         * @code
         * if (ConstraintSolver::occursIn(varId, candidateType, currentSubst)) {
         *     // Cannot unify: would create infinite type
         * }
         * @endcode
         */
        [[nodiscard]] static bool occurs_in(TypeVarId var, const TypePtr &type, const Substitution &subst);

        /// Unify two types, producing substitution entries
        [[nodiscard]] std::expected<void, CompileError> unify(const TypePtr &t1, const TypePtr &t2, const Constraint &constraint);
    private:

        UnionFind union_find_;
        Substitution substitution_;
    };

}  // namespace jsv

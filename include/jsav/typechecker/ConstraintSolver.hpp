/*
 * Created by gbian on 2 aprile 2026.
 * Copyright (c) 2026 All rights reserved.
 */
#pragma once

// clang-format off
#include "jsav/error/CompileError.hpp"
#include "jsav/typechecker/Constraint.hpp"
#include "jsav/typechecker/Substitution.hpp"
#include "jsav/typechecker/UnionFind.hpp"
// clang-format on

namespace jsv {

/// Result of constraint solving
struct SolverResult {
    Substitution substitution;            ///< Variable → Type mappings
    std::vector<CompileError> errors;     ///< Unification errors
};

/**
 * @brief Constraint solver using union-find unification.
 *
 * Solves type constraints by finding a substitution that makes
 * all constraint pairs equal. Reports errors for unresolvable
 * constraints.
 */
class ConstraintSolver {
public:
    /// Solve all constraints and produce substitution
    [[nodiscard]] SolverResult solve(const ConstraintSet& constraints);

    /// Check if a type variable occurs in a type (for occurs check)
    [[nodiscard]] static bool occurs_in(TypeVarId var, const TypePtr& type, const Substitution& subst);

private:
    /// Unify two types, producing substitution entries
    [[nodiscard]] std::expected<void, CompileError>
    unify(const TypePtr& t1, const TypePtr& t2, const Constraint& constraint);

    UnionFind union_find_;
    Substitution substitution_;
};

}  // namespace jsv

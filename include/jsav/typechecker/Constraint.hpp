/*
 * Created by gbian on 2 aprile 2026.
 * Copyright (c) 2026 All rights reserved.
 */
#pragma once

// clang-format off
#include "../headers.hpp"
#include "jsav/ast/Type.hpp"
#include "jsav/location/SourceSpan.hpp"
// clang-format on

namespace jsv {

/// Unique identifier for constraints (C1, C2, C3, ...)
using ConstraintId = std::size_t;

/**
 * @brief Type constraint: lhs = rhs
 *
 * Generated during AST traversal. The constraint solver attempts
 * to find a substitution that makes lhs and rhs equal.
 */
struct Constraint {
    ConstraintId id;                      ///< Unique constraint ID
    TypePtr lhs;                          ///< Left-hand side type
    TypePtr rhs;                          ///< Right-hand side type
    SourceSpan origin;                    ///< Source location for error reporting
    std::string reason;                   ///< Generation context (e.g., "binary +")
};

/**
 * @brief Constraint set for accumulating constraints during generation.
 *
 * Constraints are added sequentially during type checking. The set
 * is passed to the solver which produces a substitution.
 */
class ConstraintSet {
public:
    /// Add a new constraint and return its ID
    [[nodiscard]] ConstraintId add(TypePtr lhs, TypePtr rhs, SourceSpan origin, std::string_view reason);

    /// Get all constraints
    [[nodiscard]] const std::vector<Constraint>& constraints() const noexcept;

    /// Get constraint by ID
    [[nodiscard]] const Constraint* get(ConstraintId id) const noexcept;

    /// Number of constraints
    [[nodiscard]] std::size_t size() const noexcept;

private:
    std::vector<Constraint> constraints_;
    ConstraintId next_id_{1};
};

}  // namespace jsv

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
 * @brief Type constraint representing an equality relation: lhs = rhs
 *
 * Generated during AST traversal when type inference encounters an expression
 * requiring type unification. The constraint solver attempts to find a
 * substitution (mapping from type variables to concrete types) that makes
 * lhs and rhs equal.
 *
 * @note Constraints are typically created via ConstraintSet::add() rather
 *       than direct construction.
 *
 * @code
 * // A constraint generated from binary addition: a + b
 * // where a has type T1 and b has type T2
 * Constraint c{
 *     .id = 1,
 *     .lhs = make_type_var("T1"),
 *     .rhs = int_type(),
 *     .origin = expr->span(),
 *     .reason = "binary + requires numeric operands"
 * };
 * @endcode
 *
 * @var id      Unique identifier assigned by ConstraintSet (starts at 1; 0 is reserved/invalid)
 * @var lhs     Left-hand side type expression to be unified
 * @var rhs     Right-hand side type expression to be unified
 * @var origin  Source location where this constraint originated (for error diagnostics)
 * @var reason  Human-readable context explaining why this constraint was generated
 */
struct Constraint {
    ConstraintId id;                      ///< Unique constraint ID (1-based, 0 = invalid)
    TypePtr lhs;                          ///< Left-hand side type to unify
    TypePtr rhs;                          ///< Right-hand side type to unify
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
   /**
     * @brief Add a new type equality constraint to the set.
     *
     * Creates a new constraint representing the equality relation `lhs = rhs`
     * and assigns it a unique identifier. Constraints are stored in insertion
     * order and can later be retrieved via constraints() or get().
     * @param lhs    Left-hand side type expression
     * @param rhs    Right-hand side type expression  
     * @param origin Source location where the constraint originates (for error messages)
     * @param reason Human-readable explanation of why this constraint was generated
     * @throws std::bad_alloc if memory allocation fails
     *
     * @par Example
     * @code
     * ConstraintSet cs;
     * ConstraintId id = cs.add(lhs_type, rhs_type, span, "function argument");
     * assert(id >= 1);  // IDs are 1-based
     * @endcode
     */
    ConstraintId add(TypePtr lhs, TypePtr rhs, SourceSpan origin, std::string_view reason);

    /**
     * @brief Get a read-only reference to all accumulated constraints.
     *
     * Returns the constraints in the order they were added. The returned
     * reference remains valid until the next non-const operation on this set.
     *
     * @return Const reference to the internal vector of constraints.
     *
     * @par Example
     * @code
     * for (const Constraint& c : cs.constraints()) {
     *     process(c);
     * }
     * @endcode
     */
    [[nodiscard]] const std::vector<Constraint>& constraints() const noexcept;

    /**
     * @brief Look up a constraint by its unique identifier.
     *
     * Performs a linear search for the constraint with the given ID.
     *
     * @param id The constraint identifier to search for.
     *
     * @return Pointer to the constraint if found, nullptr otherwise.
     *         The pointer remains valid until the next non-const operation.
     *
     * @note Complexity: O(n) where n is the number of constraints.
     *
     * @par Example
     * @code
     * if (const Constraint* c = cs.get(id)) {
     *     report_error(c->origin, c->reason);
     * }
     * @endcode
     */
    [[nodiscard]] const Constraint* get(ConstraintId id) const noexcept;

    /**
     * @brief Get the number of constraints in the set.
     *
     * @return The total number of constraints added to this set.
     *
     * @par Example
     * @code
     * if (cs.size() > 0) {
     *     solve(cs.constraints());
     * }
     * @endcode
     */
    [[nodiscard]] std::size_t size() const noexcept;

private:
    std::vector<Constraint> constraints_;
    ConstraintId next_id_{1};
};

}  // namespace jsv

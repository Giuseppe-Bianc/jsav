/*
 * Created by gbian on 2 aprile 2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner, *-identifier-length, *-pro-type-static-cast-downcast)
#include "jsav/typechecker/ConstraintSolver.hpp"
#include "jsav/typechecker/ErrorType.hpp"

namespace jsv {

    SolverResult ConstraintSolver::solve(const ConstraintSet &constraints) {
        SolverResult result;
        union_find_ = UnionFind{};
        substitution_ = Substitution{};

        for(const auto &constraint : constraints.constraints()) {
            auto unify_result = unify(constraint.lhs, constraint.rhs, constraint);
            if(!unify_result) { result.errors.push_back(unify_result.error()); }
        }

        result.substitution = std::move(substitution_);
        return result;
    }

    bool ConstraintSolver::occurs_in(TypeVarId var, const TypePtr &type, const Substitution &subst) {
        auto resolved = subst.apply(type);
        if(!resolved) { return false; }

        if(const auto *tv = dynamic_cast<const TypeVariable *>(resolved.get())) { return tv->id() == var; }

        // Recurse into compound types
        switch(resolved->kind()) {
        case TypeKind::Array:
            {
                const auto *arr = static_cast<const ArrayType *>(resolved.get());
                return occurs_in(var, arr->element_type(), subst);
            }
        case TypeKind::Vector:
            {
                const auto *vec = static_cast<const VectorType *>(resolved.get());
                return occurs_in(var, vec->element_type(), subst);
            }
        default:
            return false;
        }
    }

    std::expected<void, CompileError> ConstraintSolver::unify(const TypePtr &t1, const TypePtr &t2, const Constraint &constraint) {
        // Handle ErrorType - silently succeeds with any type
        if(t1 && t1->kind() == TypeKind::Error) { return {}; }
        if(t2 && t2->kind() == TypeKind::Error) { return {}; }

        if(!t1 || !t2) {
            return std::unexpected{CompileError::TypeError(ErrorCode::E2034, "Null type encountered during unification", constraint.origin,
                                                           "This indicates an internal compiler error.")};
        }

        // Type variable unification
        if(const auto *tv1 = dynamic_cast<const TypeVariable *>(t1.get())) {
            if(const auto *tv2 = dynamic_cast<const TypeVariable *>(t2.get())) {
                if(tv1->id() == tv2->id()) { return {}; }  // Same variable

                // Check occurs check
                if(occurs_in(tv1->id(), t2, substitution_)) {
                    return std::unexpected{CompileError::TypeError(ErrorCode::E2035, "Occurs check failed", constraint.origin,
                                                                   "This would create an infinite recursive type.")};
                }

                // Bind tv1 to tv2
                union_find_.make_set(tv1->id());
                union_find_.make_set(tv2->id());
                union_find_.unite(tv1->id(), tv2->id());
                substitution_.bind(tv1->id(), t2);
                // LDEBUG("Unified {} = {}", tv1->to_string(), t2->to_string());
                return {};
            }

            // tv1 = concrete type
            if(occurs_in(tv1->id(), t2, substitution_)) {
                return std::unexpected{CompileError::TypeError(ErrorCode::E2035, "Occurs check failed", constraint.origin,
                                                               "This would create an infinite recursive type.")};
            }

            union_find_.make_set(tv1->id());
            substitution_.bind(tv1->id(), t2);
            // LDEBUG("Bound {} = {}", tv1->to_string(), t2->to_string());
            return {};
        }

        // NOLINTNEXTLINE(*-diagnostic-unused-but-set-variable)
        // cppcheck-suppress unreadVariable
        if(const auto *tv2 = dynamic_cast<const TypeVariable *>(t2.get())) {
            // Concrete type = tv2: swap and unify
            return unify(t2, t1, constraint);
        }

        // Both are concrete types - check structural equality
        if(t1->kind() != t2->kind()) {
            // Special case: numeric type mismatches (i64 vs f64)
            const bool is_numeric_mismatch = t1->is_numeric() && t2->is_numeric();

            std::string hint;
            if(is_numeric_mismatch) {
                hint = FORMAT("Did you mean to cast {} to {}? Example: `static_cast<{}>(value)` or `{}(value)`.", t2->to_string(),
                              t1->to_string(), t1->to_string(), t1->to_string());
            } else {
                hint = FORMAT("Did you mean to cast {} to {}?", t2->to_string(), t1->to_string());
            }

            return std::unexpected{CompileError::TypeError(ErrorCode::E2034, "Type mismatch", constraint.origin, std::move(hint))};
        }

        // Same kind - check compound types recursively
        switch(t1->kind()) {
        case TypeKind::Array:
            {
                const auto *arr1 = static_cast<const ArrayType *>(t1.get());
                const auto *arr2 = static_cast<const ArrayType *>(t2.get());
                return unify(arr1->element_type(), arr2->element_type(), constraint);
            }
        case TypeKind::Vector:
            {
                const auto *vec1 = static_cast<const VectorType *>(t1.get());
                const auto *vec2 = static_cast<const VectorType *>(t2.get());
                return unify(vec1->element_type(), vec2->element_type(), constraint);
            }
        default:
            // Primitive types with matching kinds are unified
            return {};
        }
    }

}  // namespace jsv

// NOLINTEND(*-include-cleaner, *-identifier-length, *-pro-type-static-cast-downcast)
/*
 * Created by gbian on 2 aprile 2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner, *-identifier-length, *-pro-type-static-cast-downcast)
#include "jsav/typechecker/ConstraintSolver.hpp"
#include "jsav/typechecker/ErrorType.hpp"
#include "jsav/typechecker/TypeVisitor.hpp"

namespace jsv {

    // Recurse into compound types via visitor
    struct OccursVisitor : TypeVisitor {
        TypeVarId var;
        const Substitution &subst;
        bool found{false};

        OccursVisitor(TypeVarId v, const Substitution &s) : var{v}, subst{s} {}

        void visit_array(const ArrayType &arr) override { found = ConstraintSolver::occurs_in(var, arr.element_type(), subst); }
        void visit_vector(const VectorType &vec) override { found = ConstraintSolver::occurs_in(var, vec.element_type(), subst); }
    };

    // Same kind - check compound types recursively via visitor
    struct UnifyVisitor : TypeVisitor {
        ConstraintSolver &solver;
        const TypePtr &t2;
        const Constraint &constraint;
        std::optional<std::expected<void, CompileError>> result{std::nullopt};

        UnifyVisitor(ConstraintSolver &s, const TypePtr &rhs, const Constraint &c) : solver{s}, t2{rhs}, constraint{c} {}

        void visit_array(const ArrayType &arr1) override {
            const auto *arr2 = static_cast<const ArrayType *>(t2.get());
            result = solver.unify(arr1.element_type(), arr2->element_type(), constraint);
        }

        void visit_vector(const VectorType &vec1) override {
            const auto *vec2 = static_cast<const VectorType *>(t2.get());
            result = solver.unify(vec1.element_type(), vec2->element_type(), constraint);
        }
    };

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

        const auto *resolvedPtr = resolved.get();
        if(const auto *tv = TypeVariable::classof(resolvedPtr) ? static_cast<const TypeVariable *>(resolvedPtr) : nullptr) {
            return tv->id() == var;
        }

        OccursVisitor visitor{var, subst};
        visit_type(resolved, visitor);
        return visitor.found;
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
        const auto *t1Ptr = t1.get();
        if(const auto *tv1 = TypeVariable::classof(t1Ptr) ? static_cast<const TypeVariable *>(t1Ptr) : nullptr) {
            const auto *t2Ptr = t2.get();
            if(const auto *tv2 = TypeVariable::classof(t2Ptr) ? static_cast<const TypeVariable *>(t2Ptr) : nullptr) {
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

        // NOLINTEND(*-include-cleaner, *-identifier-length, *-pro-type-static-cast-downcast)
        // clang-format off
        // NOLINTBEGIN(*-diagnostic-unused-but-set-variable, *-include-cleaner, *-identifier-length, *-pro-type-static-cast-downcast)
        // clang-format on
        // cppcheck-suppress unreadVariable
        const auto *t2PtrForSwap = t2.get();
        if([[maybe_unused]] const auto *tv2 = TypeVariable::classof(t2PtrForSwap) ? static_cast<const TypeVariable *>(t2PtrForSwap)
                                                                                  : nullptr) {
            // Concrete type = tv2: swap and unify
            return unify(t2, t1, constraint);
        }
        // clang-format off
        // NOLINTEND(*-diagnostic-unused-but-set-variable, *-include-cleaner, *-identifier-length, *-pro-type-static-cast-downcast)
        // clang-format on
        // NOLINTBEGIN(*-include-cleaner, *-identifier-length, *-pro-type-static-cast-downcast)

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

        UnifyVisitor visitor{*this, t2, constraint};
        visit_type(t1, visitor);
        // If no compound type was visited (primitive types), return success
        return visitor.result.value_or(std::expected<void, CompileError>{});
    }

}  // namespace jsv

// NOLINTEND(*-include-cleaner, *-identifier-length, *-pro-type-static-cast-downcast)
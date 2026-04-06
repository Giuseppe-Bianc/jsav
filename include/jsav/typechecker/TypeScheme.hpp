/*
 * Created by gbian on 2 aprile 2026.
 * Copyright (c) 2026 All rights reserved.
 */
#pragma once

// clang-format off
#include "../headers.hpp"
#include "jsav/ast/Type.hpp"
#include "jsav/typechecker/TypeVariable.hpp"
// clang-format on

namespace jsv {

/**
 * @brief Type scheme: ∀(vars). body
 *
 * Represents a polymorphic type with quantified type variables.
 * Example: ∀T. T → T for the identity function.
 *
 * A monomorphic type (no quantified variables) is a degenerate
 * TypeScheme with an empty quantified_vars list.
 */
struct TypeScheme {
    std::vector<TypeVarId> quantified_vars;  ///< Bound type variables
    TypePtr body;                             ///< Type body with references to vars
    bool is_const{false};                     ///< Whether the binding is immutable (const)

    /// Instantiate with fresh type variables
    [[nodiscard]] TypePtr instantiate() const;

    /// Create monomorphic scheme (no quantified variables)
    [[nodiscard]] static TypeScheme mono(TypePtr type);

    /// Create monomorphic scheme with mutability flag
    [[nodiscard]] static TypeScheme mono(TypePtr type, bool const_flag);
};

}  // namespace jsv

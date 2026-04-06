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
     * @brief Substitution: maps type variables to their resolved types.
     *
     * Produced by the constraint solver and applied (zonked) to the
     * typed AST to produce concrete types.
     */
    class Substitution {
    public:
        /// Bind a type variable to a type
        void bind(TypeVarId var, TypePtr type);

        /// Lookup the binding for a type variable
        [[nodiscard]] std::optional<TypePtr> lookup(TypeVarId var) const noexcept;

        /// Apply substitution to a type (resolve all type variables)
        [[nodiscard]] TypePtr apply(const TypePtr &type) const;

        /// Check if a variable is bound
        [[nodiscard]] bool contains(TypeVarId var) const noexcept;

        /// Number of bindings
        [[nodiscard]] std::size_t size() const noexcept;

    private:
        std::unordered_map<TypeVarId, TypePtr> bindings_;
    };

}  // namespace jsv

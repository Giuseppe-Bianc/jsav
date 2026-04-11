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
        TypePtr body;                            ///< Type body with references to vars
        bool is_const{false};                    ///< Whether the binding is immutable (const)

        /// For function bindings: the declared return type (used for return statement validation)
        std::optional<TypePtr> return_type;
        /// For function bindings: the function name (used in error messages)
        std::optional<std::string> function_name;

        /**
         * @brief Check if this scheme represents a function binding.
         *
         * @return true if return_type has a value (indicating function context).
         */
        [[nodiscard]] bool is_function_binding() const noexcept { return return_type.has_value(); }

        /**
         * @brief Instantiate this type scheme with fresh type variables.
         *
         * Creates a new type by substituting all quantified type variables
         * with fresh (unique) type variables. This is used during type inference
         * when a polymorphic binding is referenced.
         *
         * @return A TypePtr representing the instantiated type with fresh variables.
         *
         * @par Example
         * @code
         * // Given a scheme ∀T. T → T (identity function type)
         * TypeScheme identity_scheme = ...;
         * TypePtr instance = identity_scheme.instantiate();
         * // instance is now ?0 → ?0 where ?0 is a fresh type variable
         * @endcode
         */
        [[nodiscard]] TypePtr instantiate() const;
        /**
         * @brief Create a monomorphic type scheme (no quantified variables).
         *
         * A monomorphic scheme wraps a concrete type without any universal
         * quantification. This is used for bindings with known, fixed types.
         *
         * @param type The concrete type to wrap in the scheme.
         * @param const_flag Whether the binding is immutable (const). Defaults to false.
         * @param ret_type Optional return type context (for function bindings only).
         * @param func_name Optional function name (for function bindings only).
         * @return A TypeScheme with empty quantified_vars and the given body type.
         *
         * @par Example
         * @code
         * auto int_scheme = TypeScheme::mono(int_type);           // mutable binding
         * auto const_scheme = TypeScheme::mono(int_type, true);   // const binding
         * auto func_scheme = TypeScheme::mono(func_tvar, false, return_type, "foo");  // function
         * @endcode
         */
        [[nodiscard]] static TypeScheme mono(TypePtr type, bool const_flag = false, std::optional<TypePtr> ret_type = std::nullopt,
                                             std::optional<std::string> func_name = std::nullopt);
    };

}  // namespace jsv

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
        /**
         * @brief Bind a type variable to a resolved type.
         *
         * Records the association between @p var and @p type in the
         * substitution map. If @p var is already bound, the previous
         * binding is overwritten.
         *
         * @param var  The type variable identifier to bind.
         * @param type The type to associate with @p var.
         *
         * @code
         * Substitution sub;
         * sub.bind(tv_id, PrimitiveType::i32());
         * assert(sub.contains(tv_id));
         * @endcode
         */
        void bind(TypeVarId var, TypePtr type);

        /**
         * @brief Look up the type bound to a type variable.
         *
         * @param var The type variable identifier to look up.
         * @return The associated TypePtr if @p var is bound, or std::nullopt otherwise.
         *
         * @code
         * Substitution sub;
         * sub.bind(tv_id, PrimitiveType::bool_());
         * auto result = sub.lookup(tv_id);
         * assert(result.has_value() && *result == PrimitiveType::bool_());
         * @endcode
         */
        [[nodiscard]] std::optional<TypePtr> lookup(TypeVarId var) const noexcept;

        /**
         * @brief Apply this substitution to a type, resolving all nested type variables.
         *
         * Recursively replaces every type variable occurring in @p type with
         * its bound type (if any). Returns the original type unchanged when
         * no variables in it are bound.
         *
         * @param type The type to which the substitution is applied.
         * @return A new TypePtr with all resolvable type variables replaced,
         *         or the original @p type if no substitutions were applicable.
         *
         * @code
         * Substitution sub;
         * sub.bind(inner_tv, PrimitiveType::f64());
         * auto resolved = sub.apply(some_vector_type);  // Vec<?T1> → Vec<f64>
         * @endcode
         */
        [[nodiscard]] TypePtr apply(const TypePtr &type) const;

        /**
         * @brief Check whether a type variable has a binding in this substitution.
         *
         * @param var The type variable identifier to check.
         * @return true if @p var is bound, false otherwise.
         *
         * @code
         * Substitution sub;
         * assert(!sub.contains(tv_id));
         * sub.bind(tv_id, PrimitiveType::string());
         * assert(sub.contains(tv_id));
         * @endcode
         */
        [[nodiscard]] bool contains(TypeVarId var) const noexcept;

        /**
         * @brief Return the number of type variable bindings stored.
         *
         * @return The count of bindings in this substitution.
         *
         * @code
         * Substitution sub;
         * assert(sub.size() == 0);
         * sub.bind(tv1, PrimitiveType::i32());
         * sub.bind(tv2, PrimitiveType::u8());
         * assert(sub.size() == 2);
         * @endcode
         */
        [[nodiscard]] std::size_t size() const noexcept;

    private:
        std::unordered_map<TypeVarId, TypePtr> bindings_;
    };

}  // namespace jsv

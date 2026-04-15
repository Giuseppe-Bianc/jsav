/*
 * Created by gbian on 2 aprile 2026.
 * Copyright (c) 2026 All rights reserved.
 */
#pragma once

// clang-format off
#include "../headers.hpp"
#include "jsav/typechecker/TypeScheme.hpp"
// clang-format on

namespace jsv {

    /**
     * @brief Symbol table with lexical scoping.
     *
     * Manages identifier-to-TypeScheme mappings with nested scope
     * support. Implements shadowing: inner-scope bindings hide
     * outer-scope bindings with the same name.
     */
    class SymbolTable {
    public:
        /**
         * @brief Enter a new (inner) scope.
         *
         * Creates an empty scope frame and pushes it onto the scope stack.
         * All subsequent `define` calls will bind to this new scope until
         * `popScope` is called.
         */
        void push_scope();

        /**
         * @brief Exit the current (innermost) scope.
         *
         * Removes the innermost scope frame and all its bindings.
         * @pre The scope stack must not be empty (depth() > 0).
         */
        void pop_scope();

        /**
         * @brief Define a symbol in the current (innermost) scope.
         *
         * Binds @p name to @p scheme in the current scope. If @p name is
         * already defined in the current scope, the binding is overwritten.
         *
         * @param name   The identifier to bind.
         * @param scheme The type scheme to associate with the identifier.
         * @pre At least one scope must exist (depth() > 0).
         */
        void define(std::string_view name, TypeScheme scheme);

        /**
         * @brief Lookup a symbol across all scopes.
         *
         * Searches for @p name starting from the innermost scope and
         * proceeding outward. Returns the first binding found.
         *
         * @param name The identifier to look up.
         * @return The associated TypeScheme if found; std::nullopt otherwise.
         */
        [[nodiscard]] std::optional<TypeScheme> lookup(std::string_view name) const;

        /**
         * @brief Check if a symbol is defined in the current scope only.
         *
         * Does not search outer scopes. Useful for detecting redefinitions
         * or shadowing within the same scope level.
         *
         * @param name The identifier to check.
         * @return true if @p name is bound in the innermost scope; false otherwise.
         */
        [[nodiscard]] bool defined_in_current_scope(std::string_view name) const;

        /**
         * @brief Get the current scope depth.
         *
         * @return The number of active scope frames (0 if no scopes have been pushed).
         */
        [[nodiscard]] std::size_t depth() const noexcept;

        /**
         * @brief Set the return type context for the enclosing function.
         *
         * Updates the most recently defined function binding (in the current scope or
         * the scope where the function was declared) with the given return type and name.
         * This is used to track the expected return type for return statement validation.
         *
         * @param ret_type The declared return type of the enclosing function.
         * @param func_name The name of the enclosing function (for error messages).
         */
        void set_function_return_context(TypePtr ret_type, std::string func_name);

        /**
         * @brief Get the return type context of the enclosing function.
         *
         * Searches for the nearest function binding (one that has a return_type set)
         * across all scopes, starting from innermost.
         *
         * @return A pair of {return_type, function_name} if inside a function, std::nullopt otherwise.
         */
        [[nodiscard]] std::optional<std::pair<TypePtr, std::string_view>> get_function_return_context() const;

    private:
        using FunctionReturnContext = std::pair<TypePtr, std::string>;

        struct StringHash {
            using is_transparent = void;  // enables heterogeneous lookup
            [[nodiscard]] std::size_t operator()(std::string_view sv) const noexcept { return std::hash<std::string_view>{}(sv); }
        };
        std::vector<std::unordered_map<std::string_view, TypeScheme, StringHash, std::equal_to<>>> scopes_;
        std::vector<std::optional<FunctionReturnContext>> function_return_contexts_;
    };

}  // namespace jsv

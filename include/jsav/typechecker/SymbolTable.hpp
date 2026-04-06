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
    /// Enter a new scope
    void push_scope();

    /// Exit the current scope
    void pop_scope();

    /// Define a symbol in the current scope
    void define(std::string_view name, TypeScheme scheme);

    /// Lookup a symbol (searches from innermost to outermost scope)
    [[nodiscard]] std::optional<TypeScheme> lookup(std::string_view name) const;

    /// Check if a symbol exists in current scope only
    [[nodiscard]] bool defined_in_current_scope(std::string_view name) const;

    /// Current scope depth
    [[nodiscard]] std::size_t depth() const noexcept;

private:
    std::vector<std::unordered_map<std::string, TypeScheme>> scopes_;
};

}  // namespace jsv

/*
 * Created by gbian on 2 aprile 2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner)
#include "jsav/typechecker/SymbolTable.hpp"

namespace jsv {

    void SymbolTable::push_scope() { scopes_.emplace_back(); }

    void SymbolTable::pop_scope() {
        if(!scopes_.empty()) { scopes_.pop_back(); }
    }

    void SymbolTable::define(std::string_view name, TypeScheme scheme) {
        if(scopes_.empty()) { scopes_.emplace_back(); }
        scopes_.back().insert_or_assign(name, std::move(scheme));
    }

    std::optional<TypeScheme> SymbolTable::lookup(std::string_view name) const {
        // Search from innermost to outermost scope
        for(const auto &scope : std::ranges::reverse_view(scopes_)) {
            auto found = scope.find(name);
            if(found != scope.end()) { return found->second; }
        }
        return std::nullopt;
    }

    bool SymbolTable::defined_in_current_scope(std::string_view name) const {
        if(scopes_.empty()) { return false; }
        return scopes_.back().contains(name);
    }

    std::size_t SymbolTable::depth() const noexcept { return scopes_.size(); }

    void SymbolTable::set_function_return_context(TypePtr ret_type, std::string func_name) {
        // Search from innermost to outermost scope to find the function binding
        for(auto &scope : std::ranges::reverse_view(scopes_)) {
            auto it = scope.find(func_name);
            if(it != scope.end() && it->second.is_function_binding()) {
                it->second.return_type = std::move(ret_type);
                it->second.function_name = std::move(func_name);
                return;
            }
        }
        // If not found by name, look for any function binding in current scope
        if(!scopes_.empty()) {
            for(auto &[name, scheme] : scopes_.back()) {
                if(scheme.is_function_binding()) {
                    scheme.return_type = std::move(ret_type);
                    scheme.function_name = std::move(func_name);
                    return;
                }
            }
        }
    }

    std::optional<std::pair<TypePtr, std::string_view>> SymbolTable::get_function_return_context() const {
        // Search from innermost to outermost scope for the nearest function binding
        for(const auto &scope : std::ranges::reverse_view(scopes_)) {
            for(const auto &[name, scheme] : scope) {
                if(scheme.is_function_binding() && scheme.return_type) {
                    return std::make_pair(*scheme.return_type, std::string_view{name});
                }
            }
        }
        return std::nullopt;
    }

}  // namespace jsv

// NOLINTEND(*-include-cleaner)
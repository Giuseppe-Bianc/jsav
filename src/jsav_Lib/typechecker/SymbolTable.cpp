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
        // Push a synthetic function context marker into the current innermost scope.
        // This ensures get_function_return_context finds the NEAREST enclosing function,
        // not an arbitrary one from an outer scope.
        if(scopes_.empty()) { scopes_.emplace_back(); }
        auto &current_scope = scopes_.back();
        TypeScheme ctx_marker;
        ctx_marker.return_type = std::move(ret_type);
        ctx_marker.function_name = std::move(func_name);
        current_scope.insert_or_assign("__function_context__", std::move(ctx_marker));
    }

    std::optional<std::pair<TypePtr, std::string_view>> SymbolTable::get_function_return_context() const {
        // Search from innermost to outermost scope for the nearest function context marker
        for(const auto &scope : std::ranges::reverse_view(scopes_)) {
            auto it = scope.find("__function_context__");
            if(it != scope.end() && it->second.is_function_binding() && it->second.return_type) {
                return std::make_pair(*it->second.return_type, std::string_view{*it->second.function_name});
            }
        }
        return std::nullopt;
    }

}  // namespace jsv

// NOLINTEND(*-include-cleaner)
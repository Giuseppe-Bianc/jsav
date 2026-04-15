/*
 * Created by gbian on 2 aprile 2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner, *-identifier-length)
#include "jsav/typechecker/SymbolTable.hpp"

namespace jsv {

    void SymbolTable::push_scope() {
        scopes_.emplace_back();
        function_return_contexts_.emplace_back(std::nullopt);
    }

    void SymbolTable::pop_scope() {
        if(!scopes_.empty()) {
            scopes_.pop_back();
            function_return_contexts_.pop_back();
        }
    }

    void SymbolTable::define(std::string_view name, TypeScheme scheme) {
        if(scopes_.empty()) { push_scope(); }
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
        if(scopes_.empty()) { push_scope(); }
        function_return_contexts_.back() = std::make_pair(std::move(ret_type), std::move(func_name));
    }

    std::optional<std::pair<TypePtr, std::string_view>> SymbolTable::get_function_return_context() const {
        // Search from innermost to outermost scope for the nearest function context.
        for(const auto &ctx_opt : std::ranges::reverse_view(function_return_contexts_)) {
            if(ctx_opt.has_value()) {
                const auto &[ret, fname] = *ctx_opt;
                return std::make_pair(ret, std::string_view{fname});
            }
        }
        return std::nullopt;
    }

}  // namespace jsv

// NOLINTEND(*-include-cleaner, *-identifier-length)
/*
 * Created by gbian on 2 aprile 2026.
 * Copyright (c) 2026 All rights reserved.
 */
#include "jsav/typechecker/SymbolTable.hpp"

namespace jsv {

    void SymbolTable::push_scope() { scopes_.emplace_back(); }

    void SymbolTable::pop_scope() {
        if(!scopes_.empty()) { scopes_.pop_back(); }
    }

    void SymbolTable::define(std::string_view name, TypeScheme scheme) {
        if(scopes_.empty()) { scopes_.emplace_back(); }
        scopes_.back().insert_or_assign(std::string{name}, std::move(scheme));
    }

    std::optional<TypeScheme> SymbolTable::lookup(std::string_view name) const {
        // Search from innermost to outermost scope
        for(auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
            auto found = it->find(std::string{name});
            if(found != it->end()) { return found->second; }
        }
        return std::nullopt;
    }

    bool SymbolTable::defined_in_current_scope(std::string_view name) const {
        if(scopes_.empty()) { return false; }
        return scopes_.back().contains(std::string{name});
    }

    std::size_t SymbolTable::depth() const noexcept { return scopes_.size(); }

}  // namespace jsv

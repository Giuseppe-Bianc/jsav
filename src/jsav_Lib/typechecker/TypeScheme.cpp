/*
 * Created by gbian on 2 aprile 2026.
 * Copyright (c) 2026 All rights reserved.
 */
#include "jsav/typechecker/TypeScheme.hpp"
#include "jsav/typechecker/TypeVariable.hpp"

namespace jsv {

    TypeScheme TypeScheme::mono(TypePtr type, bool const_flag) {
        return TypeScheme{.quantified_vars = {}, .body = std::move(type), .is_const = const_flag};
    }

    TypePtr TypeScheme::instantiate() const {
        if(quantified_vars.empty()) { return body; }

        // Generate fresh type variables for each quantified variable
        std::unordered_map<TypeVarId, TypePtr> fresh_vars;
        for(auto qvar : quantified_vars) { fresh_vars[qvar] = fresh_type_variable(); }

        // Substitute quantified variables with fresh ones
        // For now, return body as-is since the substitution logic for
        // TypeVariable IDs within compound types requires full AST traversal.
        // The body contains references to TypeVariable instances whose IDs
        // match quantified_vars. We need to replace those.

        // Simple case: if body is a TypeVariable that's quantified, replace it
        if(const auto *tv = dynamic_cast<const TypeVariable *>(body.get())) {
            auto it = fresh_vars.find(tv->id());
            if(it != fresh_vars.end()) { return it->second; }
            return body;
        }

        // For compound types, we'd need to traverse and replace.
        // This is a simplified implementation - full version would use a visitor.
        return body;
    }

}  // namespace jsv

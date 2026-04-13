/*
 * Created by gbian on 2 aprile 2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner, *-identifier-length, *-pro-type-static-cast-downcast)
#include "jsav/typechecker/TypeScheme.hpp"
#include "jsav/typechecker/TypeVariable.hpp"

namespace jsv {

    TypeScheme TypeScheme::mono(TypePtr type, bool const_flag, std::optional<TypePtr> ret_type, std::optional<std::string> func_name) {
        return TypeScheme{.quantified_vars = {},
                          .body = std::move(type),
                          .is_const = const_flag,
                          .return_type = std::move(ret_type),
                          .function_name = std::move(func_name)};
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
        const auto *bodyPtr = body.get();
        if(const auto *tv = TypeVariable::classof(bodyPtr) ? static_cast<const TypeVariable *>(bodyPtr) : nullptr) {
            auto it = fresh_vars.find(tv->id());
            if(it != fresh_vars.end()) { return it->second; }
            return body;
        }

        // For compound types, we'd need to traverse and replace.
        // This is a simplified implementation - full version would use a visitor.
        return body;
    }

}  // namespace jsv

// NOLINTEND(*-include-cleaner, *-identifier-length, *-pro-type-static-cast-downcast)
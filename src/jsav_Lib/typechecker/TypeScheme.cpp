/*
 * Created by gbian on 2 aprile 2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner, *-identifier-length, *-pro-type-static-cast-downcast)
#include "jsav/typechecker/TypeScheme.hpp"
#include "jsav/ast/Type.hpp"
#include "jsav/typechecker/TypeVariable.hpp"

namespace jsv {

    TypeScheme TypeScheme::mono(TypePtr type, bool const_flag, std::optional<TypePtr> ret_type, std::optional<std::string> func_name) {
        return TypeScheme{.quantified_vars = {},
                          .body = std::move(type),
                          .is_const = const_flag,
                          .return_type = std::move(ret_type),
                          .function_name = std::move(func_name)};
    }

    namespace {
        /// Recursive visitor that substitutes quantified type variables with fresh ones
        [[nodiscard]] TypePtr substitute_quantified(const TypePtr &type, const std::unordered_map<TypeVarId, TypePtr> &fresh_vars) {
            const auto *base = type.get();

            // TypeVariable: replace if quantified
            if(TypeVariable::classof(base)) {
                const auto *tv = static_cast<const TypeVariable *>(base);
                auto it = fresh_vars.find(tv->id());
                if(it != fresh_vars.end()) { return it->second; }
                return type;
            }

            // ArrayType: recurse into element type
            if(ArrayType::classof(base)) {
                const auto *arr = static_cast<const ArrayType *>(base);
                auto new_element = substitute_quantified(arr->element_type(), fresh_vars);
                if(new_element == arr->element_type()) { return type; }  // No change
                return std::make_shared<ArrayType>(new_element, arr->size_expr());
            }

            // VectorType: recurse into element type
            if(VectorType::classof(base)) {
                const auto *vec = static_cast<const VectorType *>(base);
                auto new_element = substitute_quantified(vec->element_type(), fresh_vars);
                if(new_element == vec->element_type()) { return type; }  // No change
                return std::make_shared<VectorType>(new_element);
            }

            // CustomType, PrimitiveType, Error: no type variables to substitute
            return type;
        }
    }  // namespace

    TypePtr TypeScheme::instantiate() const {
        if(quantified_vars.empty()) { return body; }

        // Generate fresh type variables for each quantified variable
        std::unordered_map<TypeVarId, TypePtr> fresh_vars;
        fresh_vars.reserve(quantified_vars.size());
        for(auto qvar : quantified_vars) { fresh_vars[qvar] = fresh_type_variable(); }

        // Traverse the entire type body, substituting all quantified variables
        return substitute_quantified(body, fresh_vars);
    }

}  // namespace jsv

// NOLINTEND(*-include-cleaner, *-identifier-length, *-pro-type-static-cast-downcast)
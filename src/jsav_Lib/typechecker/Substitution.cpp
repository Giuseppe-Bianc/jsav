/*
 * Created by gbian on 2 aprile 2026.
 * Copyright (c) 2026 All rights reserved.
 */
#include "jsav/typechecker/Substitution.hpp"
#include "jsav/typechecker/ErrorType.hpp"

namespace jsv {

    void Substitution::bind(TypeVarId var, TypePtr type) { bindings_[var] = std::move(type); }

    std::optional<TypePtr> Substitution::lookup(TypeVarId var) const noexcept {
        auto it = bindings_.find(var);
        if(it == bindings_.end()) { return std::nullopt; }
        return it->second;
    }

    bool Substitution::contains(TypeVarId var) const noexcept { return bindings_.contains(var); }

    std::size_t Substitution::size() const noexcept { return bindings_.size(); }

    TypePtr Substitution::apply(const TypePtr &type) const {
        if(!type) { return nullptr; }

        // Handle type variables
        if(const auto *tv = dynamic_cast<const TypeVariable *>(type.get())) {
            auto it = bindings_.find(tv->id());
            if(it != bindings_.end()) {
                // Recursively apply to handle transitive bindings
                return apply(it->second);
            }
            return type;
        }

        // Handle compound types - recursively apply
        switch(type->kind()) {
        case TypeKind::Array:
            {
                const auto *arr = static_cast<const ArrayType *>(type.get());
                auto elem = apply(arr->element_type());
                if(elem == arr->element_type()) { return type; }
                return std::make_shared<ArrayType>(elem, arr->size_expr());
            }
        case TypeKind::Vector:
            {
                const auto *vec = static_cast<const VectorType *>(type.get());
                auto elem = apply(vec->element_type());
                if(elem == vec->element_type()) { return type; }
                return std::make_shared<VectorType>(elem);
            }
        case TypeKind::Custom:
            {
                // Custom types don't contain type variables
                return type;
            }
        default:
            // Primitive types, TypeVar (unbound), Error - return as-is
            return type;
        }
    }

}  // namespace jsv

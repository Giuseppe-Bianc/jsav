/*
 * Created by gbian on 2 aprile 2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner, *-identifier-length, *-pro-type-static-cast-downcast)
#include "jsav/typechecker/Substitution.hpp"
#include "jsav/typechecker/ErrorType.hpp"

namespace jsv {

    void Substitution::bind(TypeVarId var, TypePtr type) {
        apply_cache_.clear();
        bindings_[var] = std::move(type);
    }

    std::optional<TypePtr> Substitution::lookup(TypeVarId var) const noexcept {
        auto it = bindings_.find(var);
        if(it == bindings_.end()) { return std::nullopt; }
        return it->second;
    }

    bool Substitution::contains(TypeVarId var) const noexcept { return bindings_.contains(var); }

    std::size_t Substitution::size() const noexcept { return bindings_.size(); }

    TypePtr Substitution::apply(const TypePtr &type) const { return applyImpl(type); }

    TypePtr Substitution::applyImpl(const TypePtr &type) const {
        if(!type) [[unlikely]] { return nullptr; }

        if(const auto it = apply_cache_.find(type.get()); it != apply_cache_.end()) { return it->second; }

        TypePtr result;

        if(const auto *tv = dynamic_cast<const TypeVariable *>(type.get())) {
            const auto it = bindings_.find(tv->id());
            result = (it != bindings_.end()) ? applyImpl(it->second) : type;
        } else {
            switch(type->kind()) {
            case TypeKind::Array:
                {
                    const auto *arr = static_cast<const ArrayType *>(type.get());
                    auto elem = applyImpl(arr->element_type());
                    result = (elem == arr->element_type()) ? type : std::make_shared<ArrayType>(std::move(elem), arr->size_expr());
                    break;
                }
            case TypeKind::Vector:
                {
                    const auto *vec = static_cast<const VectorType *>(type.get());
                    auto elem = applyImpl(vec->element_type());
                    result = (elem == vec->element_type()) ? type : std::make_shared<VectorType>(std::move(elem));
                    break;
                }
            case TypeKind::Custom:
                result = type;
                break;
            default:
                result = type;
                break;
            }
        }

        apply_cache_.emplace(type.get(), result);
        return result;
    }

}  // namespace jsv

// NOLINTEND(*-include-cleaner, *-identifier-length, *-pro-type-static-cast-downcast)
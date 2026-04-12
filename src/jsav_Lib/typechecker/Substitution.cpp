/*
 * Created by gbian on 2 aprile 2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner, *-identifier-length, *-pro-type-static-cast-downcast)
#include "jsav/typechecker/Substitution.hpp"
#include "jsav/typechecker/ErrorType.hpp"
#include "jsav/typechecker/TypeVisitor.hpp"

namespace jsv {

    // Use visitor to dispatch on compound types
    struct ApplyVisitor : TypeVisitor {
        const Substitution &self;
        TypePtr out{nullptr};

        explicit ApplyVisitor(const Substitution &s) : self{s} {}

        void visit_array(const ArrayType &arr) override {
            auto elem = self.applyImpl(arr.element_type());
            out = (elem == arr.element_type()) ? nullptr : std::make_shared<ArrayType>(std::move(elem), arr.size_expr());
        }

        void visit_vector(const VectorType &vec) override {
            auto elem = self.applyImpl(vec.element_type());
            out = (elem == vec.element_type()) ? nullptr : std::make_shared<VectorType>(std::move(elem));
        }
    };

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

        const auto *typePtr = type.get();
        if(const auto *tv = TypeVariable::classof(typePtr) ? static_cast<const TypeVariable *>(typePtr) : nullptr) {
            const auto it = bindings_.find(tv->id());
            result = (it != bindings_.end()) ? applyImpl(it->second) : type;
        } else {
            ApplyVisitor visitor{*this};
            visit_type(type, visitor);

            if(visitor.out) {
                result = std::move(visitor.out);
            } else {
                result = type;
            }
        }

        apply_cache_.emplace(type.get(), result);
        return result;
    }

}  // namespace jsv

// NOLINTEND(*-include-cleaner, *-identifier-length, *-pro-type-static-cast-downcast)
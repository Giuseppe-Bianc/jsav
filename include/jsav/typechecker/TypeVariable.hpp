/*
 * Created by gbian on 2 aprile 2026.
 * Copyright (c) 2026 All rights reserved.
 */
#pragma once

// clang-format off
#include "../headers.hpp"
#include "jsav/ast/Type.hpp"
// clang-format on

namespace jsv {

/// Unique identifier for type variables
using TypeVarId = std::size_t;

/**
 * @brief Type variable representation (?T1, ?T2, ...) for constraint-based type inference.
 *
 * Type variables represent unknown types that will be resolved during
 * constraint solving. They are generated for expressions without explicit
 * type annotations.
 *
 * @invariant id_ > 0 (zero is reserved for uninitialized)
 */
class TypeVariable final : public TypeBase {
public:
    explicit constexpr TypeVariable(TypeVarId id) : TypeBase{TypeKind::TypeVar}, id_{id} {}

    [[nodiscard]] constexpr TypeVarId id() const noexcept { return id_; }

    /// Format as string (e.g., "?T42")
    [[nodiscard]] std::string to_string() const override;

    [[nodiscard]] static constexpr bool classof(const TypeBase* t) noexcept {
        return t && t->kind() == TypeKind::TypeVar;
    }

    /// Equality comparison: two type variables are equal if their IDs match
    [[nodiscard]] bool operator==(const TypeBase& other) const noexcept override {
        if(other.kind() != kind()) { return false; }
        const auto* other_tv = static_cast<const TypeVariable*>(&other);
        return id_ == other_tv->id_;
    }

private:
    TypeVarId id_;
};

/// Generate a fresh type variable with unique ID (thread-local counter)
[[nodiscard]] TypePtr fresh_type_variable() noexcept;

}  // namespace jsv

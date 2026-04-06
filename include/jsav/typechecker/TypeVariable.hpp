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
    /**
     * @brief Retrieves the unique identifier of this type variable.
     * @return The non-zero TypeVarId assigned during construction.
     *
     * @code
     * TypeVariable tv{42};
     * assert(tv.id() == 42);
     * @endcode
     */
    explicit constexpr TypeVariable(TypeVarId id) : TypeBase{TypeKind::TypeVar}, id_{id} {}

    /**
     * @brief Formats the type variable as a human-readable string.
     * @return A string in the format "?T{id}", e.g., "?T42" for id=42.
     *
     * @code
     * TypeVariable tv{123};
     * assert(tv.to_string() == "?T123");
     * @endcode
     */
    [[nodiscard]] constexpr TypeVarId id() const noexcept { return id_; }

    /// Format as string (e.g., "?T42")
    [[nodiscard]] std::string to_string() const override;

    /**
     * @brief LLVM-style RTTI helper for safe downcasting.
     * @param t Pointer to a TypeBase (may be nullptr).
     * @return true if @p t is non-null and represents a TypeVariable.
     *
     * @code
     * const TypeBase* base = get_some_type();
     * if (TypeVariable::classof(base)) {
     *     const auto* tv = static_cast<const TypeVariable*>(base);
     *     // use tv->id()
     * }
     * @endcode
     */
    [[nodiscard]] static constexpr bool classof(const TypeBase* t) noexcept {
        return t && t->kind() == TypeKind::TypeVar;
    }

    /**
     * @brief Compares two TypeBase instances for equality.
     * @param other The TypeBase reference to compare against.
     * @return true if @p other is a TypeVariable with the same ID.
     *
     * Two TypeVariables are considered equal if and only if their IDs match.
     * Comparing a TypeVariable to any other TypeBase-derived type returns false.
     */
    [[nodiscard]] bool operator==(const TypeBase& other) const noexcept override {
        if(other.kind() != kind()) { return false; }
        const auto* other_tv = static_cast<const TypeVariable*>(&other);
        return id_ == other_tv->id_;
    }

private:
    TypeVarId id_;
};

/**
 * @brief Generates a fresh type variable with a globally unique ID.
 *
 * Uses a thread-local counter to ensure uniqueness across concurrent
 * type inference operations. Each call returns a new TypeVariable with
 * an ID guaranteed to be unique within the current thread.
 *
 * @return A TypePtr (smart pointer) to a newly allocated TypeVariable.
 * @throws std::bad_alloc if memory allocation fails.
 *
 * @code
 * auto tv1 = fresh_type_variable();  // e.g., ?T1
 * auto tv2 = fresh_type_variable();  // e.g., ?T2
 * assert(tv1->to_string() != tv2->to_string());
 * @endcode
 *
 * @note Thread-safety: Safe to call concurrently from multiple threads.
 *       Each thread maintains its own counter namespace.
 */
[[nodiscard]] TypePtr fresh_type_variable() noexcept;

}  // namespace jsv

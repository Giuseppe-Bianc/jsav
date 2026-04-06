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

    /**
     * @brief Error type placeholder for error recovery during type checking.
     *
     * ErrorType is inserted when a type error is detected. It silently
     * unifies with any type, preventing cascading error reports from
     * a single root cause.
     *
     * @note This is a singleton type - use error_type() to obtain instances.
     */
    class ErrorType final : public TypeBase {
    public:
        ErrorType() : TypeBase{TypeKind::Error} {}

        /**
         * @brief Returns a string representation of the error type.
         * @return A string identifying this as an error type (e.g., "<error>").
         */
        [[nodiscard]] std::string to_string() const override;

        /**
         * @brief LLVM-style RTTI support for type identification.
         * @param t Pointer to the type to check.
         * @return true if @p t is non-null and represents an ErrorType.
         */
        [[nodiscard]] static constexpr bool classof(const TypeBase *t) noexcept { return t && t->kind() == TypeKind::Error; }

        /**
         * @brief Equality comparison for error types.
         * @param other The type to compare against.
         * @return true if @p other is also an ErrorType.
         * @note ErrorType compares equal to any other ErrorType instance,
         *       enabling error propagation without cascading diagnostics.
         */
        [[nodiscard]] bool operator==(const TypeBase &other) const noexcept override { return other.kind() == kind(); }
    };

    /// Singleton error type instance
    [[nodiscard]] TypePtr error_type() noexcept;

}  // namespace jsv

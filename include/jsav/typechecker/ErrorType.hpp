/*
 * Created by gbian on 2 aprile 2026.
 * Copyright (c) 2026 All rights reserved.
 */
#pragma once

// clang-format off
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

    [[nodiscard]] std::string to_string() const override;

    [[nodiscard]] static constexpr bool classof(const TypeBase* t) noexcept {
        return t && t->kind() == TypeKind::Error;
    }

    /// Equality comparison: ErrorType equals any ErrorType
    [[nodiscard]] bool operator==(const TypeBase& other) const noexcept override {
        return other.kind() == kind();
    }
};

/// Singleton error type instance
[[nodiscard]] TypePtr error_type() noexcept;

}  // namespace jsv

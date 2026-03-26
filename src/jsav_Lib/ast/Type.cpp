/*
 * Created by gbian on 24 marzo 2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner, *-identifier-length)

#include "jsav/ast/Type.hpp"
#include "jsav/ast/Expressions.hpp"

namespace jsv {

    // Note: type_kind_name constexpr definition moved to header for cross-TU visibility

    // ============================================================
    // PrimitiveType::to_string
    // ============================================================
    [[nodiscard]] std::string PrimitiveType::to_string() const { return std::string{type_kind_name(kind())}; }

    // ============================================================
    // CustomType::to_string
    // ============================================================
    [[nodiscard]] std::string CustomType::to_string() const { return std::string{*name_}; }

    // ============================================================
    // ArrayType::to_string
    // ============================================================
    [[nodiscard]] std::string ArrayType::to_string() const {
        std::string result = "[";
        result += element_type_->to_string();
        result += "; ";
        if(size_expr_) {
            // Try to extract the size value from IntegerLiteral using kind() check
            if(size_expr_->kind() == NodeKind::IntegerLiteral) {
                const auto *int_lit = dynamic_cast<const IntegerLiteral *>(size_expr_.get());
                result += FORMAT("{}", int_lit->value());
            } else {
                result += "<expr>";
            }
        } else {
            result += "unknown";
        }
        result += "]";
        return result;
    }

    // ============================================================
    // VectorType::to_string
    // ============================================================
    [[nodiscard]] std::string VectorType::to_string() const { return FORMAT("Vec<{}>", element_type_); }

}  // namespace jsv

// NOLINTEND(*-include-cleaner, *-identifier-length)

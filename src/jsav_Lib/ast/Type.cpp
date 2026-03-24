/*
 * Created by gbian on 24 marzo 2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner, *-identifier-length)

#include "jsav/ast/Type.hpp"

namespace jsv {

    // ============================================================
    // type_kind_name - string representation of TypeKind
    // ============================================================
    [[nodiscard]] std::string_view type_kind_name(TypeKind kind) noexcept {
        switch(kind) {
        case TypeKind::I8:
            return "i8";
        case TypeKind::I16:
            return "i16";
        case TypeKind::I32:
            return "i32";
        case TypeKind::I64:
            return "i64";
        case TypeKind::U8:
            return "u8";
        case TypeKind::U16:
            return "u16";
        case TypeKind::U32:
            return "u32";
        case TypeKind::U64:
            return "u64";
        case TypeKind::F32:
            return "f32";
        case TypeKind::F64:
            return "f64";
        case TypeKind::Char:
            return "char";
        case TypeKind::String:
            return "string";
        case TypeKind::Bool:
            return "bool";
        case TypeKind::Custom:
            return "custom";
        case TypeKind::Array:
            return "array";
        case TypeKind::Vector:
            return "vector";
        case TypeKind::Void:
            return "void";
        case TypeKind::NullPtr:
            return "nullptr";
        default:
            return "unknown";
        }
    }

    // ============================================================
    // PrimitiveType::to_string
    // ============================================================
    [[nodiscard]] std::string PrimitiveType::to_string() const {
        return std::string{type_kind_name(kind())};
    }

    // ============================================================
    // CustomType::to_string
    // ============================================================
    [[nodiscard]] std::string CustomType::to_string() const {
        return std::string{*name_};
    }

    // ============================================================
    // ArrayType::to_string
    // ============================================================
    [[nodiscard]] std::string ArrayType::to_string() const {
        std::string result = "[";
        result += element_type_->to_string();
        result += "; ";
        if(size_expr_) {
            // For now, we just indicate that there's a size expression
            // In a full implementation, you'd evaluate or print the expression
            result += "<expr>";
        } else {
            result += "unknown";
        }
        result += "]";
        return result;
    }

    // ============================================================
    // VectorType::to_string
    // ============================================================
    [[nodiscard]] std::string VectorType::to_string() const {
        return FORMAT("Vec<{}>", element_type_);
    }

}  // namespace jsv

// NOLINTEND(*-include-cleaner, *-identifier-length)

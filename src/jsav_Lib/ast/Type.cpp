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
        std::string result;
        auto out = std::back_inserter(result);
        FORMAT_TO(out, "[{}; ", element_type_->to_string());
        if(size_expr_) {
            // Try to extract the size value from IntegerLiteral using kind() check
            if(size_expr_->kind() == NodeKind::IntegerLiteral) {
                const auto *int_lit = dynamic_cast<const IntegerLiteral *>(size_expr_.get());
                FORMAT_TO(out, "{}", int_lit->value());
            } else {
                FORMAT_TO(out, "<expr>");
            }
        } else {
            FORMAT_TO(out, "unknown");
        }
        FORMAT_TO(out, "]");
        return result;
    }

    // ============================================================
    // VectorType::to_string
    // ============================================================
    [[nodiscard]] std::string VectorType::to_string() const { return FORMAT("Vec<{}>", element_type_->to_string()); }

    [[nodiscard]] bool ArrayType::sizes_equal(const Expr &a, const Expr &b) noexcept {
        if(const auto *ia = node_dyn_cast<const IntegerLiteral>(&a)) {
            if(const auto *ib = node_dyn_cast<const IntegerLiteral>(&b)) { return ia->value() == ib->value(); }
        }
        return &a == &b;
    }

    // ============================================================
    // numeric_promotion - Implements C/C++ usual arithmetic conversions
    // ============================================================
    [[nodiscard]] TypePtr numeric_promotion(const TypePtr &t1, const TypePtr &t2) noexcept {
        if(!t1 || !t2) { return nullptr; }
        if(!t1->is_numeric() || !t2->is_numeric()) { return nullptr; }

        // Same type - no promotion needed
        if(*t1 == *t2) { return t1; }

        // Helper lambda to get numeric rank (higher = "wider" type)
        const auto get_rank = [](const TypePtr &t) -> int {
            switch(t->kind()) {
            case TypeKind::I8:
            case TypeKind::U8:
                return 1;
            case TypeKind::I16:
            case TypeKind::U16:
                return 2;
            case TypeKind::I32:
            case TypeKind::U32:
                return 3;
            case TypeKind::I64:
            case TypeKind::U64:
                return 4;
            case TypeKind::F32:
                return 5;
            case TypeKind::F64:
                return 6;
            default:
                return 0;
            }
        };

        const auto rank1 = get_rank(t1);
        const auto rank2 = get_rank(t2);

        // Floating-point promotion: any float + higher float → higher float
        if(t1->is_floating_point() && t2->is_floating_point()) { return rank1 >= rank2 ? t1 : t2; }

        // Integer + floating-point → floating-point
        if(t1->is_floating_point()) { return t1; }
        if(t2->is_floating_point()) { return t2; }

        // Both are integers - promote to wider type
        if(rank1 > rank2) { return t1; }
        if(rank2 > rank1) { return t2; }

        // Same rank but different types (signed vs unsigned of same size)
        // Promote to unsigned to preserve bit pattern
        if(t1->kind() == TypeKind::I8 || t1->kind() == TypeKind::I16 || t1->kind() == TypeKind::I32 || t1->kind() == TypeKind::I64) {
            // t1 is signed, t2 is unsigned - promote to unsigned
            switch(t2->kind()) {
            case TypeKind::U8:
                return PrimitiveType::u8();
            case TypeKind::U16:
                return PrimitiveType::u16();
            case TypeKind::U32:
                return PrimitiveType::u32();
            case TypeKind::U64:
                return PrimitiveType::u64();
            default:
                break;
            }
        }

        // t1 is unsigned, t2 is signed - same rank, promote to unsigned
        switch(t1->kind()) {
        case TypeKind::U8:
            return PrimitiveType::u8();
        case TypeKind::U16:
            return PrimitiveType::u16();
        case TypeKind::U32:
            return PrimitiveType::u32();
        case TypeKind::U64:
            return PrimitiveType::u64();
        default:
            break;
        }

        // Fallback - should not reach here for valid numeric types
        return rank1 >= rank2 ? t1 : t2;
    }

}  // namespace jsv

// NOLINTEND(*-include-cleaner, *-identifier-length)

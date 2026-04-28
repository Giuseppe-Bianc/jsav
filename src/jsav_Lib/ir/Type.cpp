// clang-format off
// NOLINTBEGIN(*-include-cleaner, *-avoid-magic-numbers, *-magic-numbers, *-owning-memory)
// clang-format on

#include "jsav/ir/Type.hpp"
#include "jsavCore/util/Sha256.hpp"

namespace jsv {

    // ─────────────────────────────────────────────────────────────────────────────
    // PrimitiveType Implementation
    // ─────────────────────────────────────────────────────────────────────────────

    std::string PrimitiveType::canonical_name() const noexcept {
        switch(kind_) {
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
            case TypeKind::Bool:
                return "bool";
            default:
                return "unknown";
        }
    }

    std::shared_ptr<const PrimitiveType> PrimitiveType::i8() noexcept {
        static const auto instance = std::make_shared<const PrimitiveType>(TypeKind::I8);
        return instance;
    }

    std::shared_ptr<const PrimitiveType> PrimitiveType::i16() noexcept {
        static const auto instance = std::make_shared<const PrimitiveType>(TypeKind::I16);
        return instance;
    }

    std::shared_ptr<const PrimitiveType> PrimitiveType::i32() noexcept {
        static const auto instance = std::make_shared<const PrimitiveType>(TypeKind::I32);
        return instance;
    }

    std::shared_ptr<const PrimitiveType> PrimitiveType::i64() noexcept {
        static const auto instance = std::make_shared<const PrimitiveType>(TypeKind::I64);
        return instance;
    }

    std::shared_ptr<const PrimitiveType> PrimitiveType::u8() noexcept {
        static const auto instance = std::make_shared<const PrimitiveType>(TypeKind::U8);
        return instance;
    }

    std::shared_ptr<const PrimitiveType> PrimitiveType::u16() noexcept {
        static const auto instance = std::make_shared<const PrimitiveType>(TypeKind::U16);
        return instance;
    }

    std::shared_ptr<const PrimitiveType> PrimitiveType::u32() noexcept {
        static const auto instance = std::make_shared<const PrimitiveType>(TypeKind::U32);
        return instance;
    }

    std::shared_ptr<const PrimitiveType> PrimitiveType::u64() noexcept {
        static const auto instance = std::make_shared<const PrimitiveType>(TypeKind::U64);
        return instance;
    }

    std::shared_ptr<const PrimitiveType> PrimitiveType::f32() noexcept {
        static const auto instance = std::make_shared<const PrimitiveType>(TypeKind::F32);
        return instance;
    }

    std::shared_ptr<const PrimitiveType> PrimitiveType::f64() noexcept {
        static const auto instance = std::make_shared<const PrimitiveType>(TypeKind::F64);
        return instance;
    }

    std::shared_ptr<const PrimitiveType> PrimitiveType::bool_() noexcept {
        static const auto instance = std::make_shared<const PrimitiveType>(TypeKind::Bool);
        return instance;
    }

    // ─────────────────────────────────────────────────────────────────────────────
    // CompositeType Implementation
    // ─────────────────────────────────────────────────────────────────────────────

    CompositeType::CompositeType(std::string composite_name) noexcept : name_(std::move(composite_name)) {}

    std::string CompositeType::canonical_name() const noexcept { return name_; }

    // ─────────────────────────────────────────────────────────────────────────────
    // UserDefinedType Implementation
    // ─────────────────────────────────────────────────────────────────────────────

    UserDefinedType::UserDefinedType(std::string type_name, TypeLayout layout) noexcept
        : type_identity_(std::move(type_name)), layout_(layout), equivalence_rule_("nominal") {
        // Compute version_id from SHA-256 hash of canonical layout binary
        const auto canonical_binary = layout_.compute_canonical_binary();
        version_id_ = SHA256::hash(canonical_binary);
    }

    std::string UserDefinedType::canonical_name() const noexcept { return type_identity_; }

    // ─────────────────────────────────────────────────────────────────────────────
    // TypeLayout Implementation
    // ─────────────────────────────────────────────────────────────────────────────

    std::string TypeLayout::compute_canonical_binary() const noexcept {
        // Serialize layout fields and offsets in a deterministic order
        // for SHA-256 hashing
        std::string result = fmt::format("layout:{}", field_offsets.size());
        for(size_t i = 0; i < fields.size(); ++i) {
            const auto &[name, type] = fields[i];
            result += fmt::format(":{}@{}", name, field_offsets[i]);
        }
        return result;
    }

    // ─────────────────────────────────────────────────────────────────────────────
    // TypeIdentity Implementation
    // ─────────────────────────────────────────────────────────────────────────────

    std::string TypeIdentity::fully_qualified() const noexcept {
        if(scope_path.empty()) {
            return fmt::format("{}::{}", module_name, type_name);
        }
        return fmt::format("{}::{}::{}", module_name, scope_path, type_name);
    }

}  // namespace jsv

// clang-format off
// NOLINTEND(*-include-cleaner, *-avoid-magic-numbers, *-magic-numbers, *-owning-memory)
// clang-format on

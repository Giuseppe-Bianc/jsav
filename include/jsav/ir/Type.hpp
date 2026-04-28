#pragma once

#include "../error/CompileError.hpp"
#include "GlobalEntityId.hpp"
#include "IrCommon.hpp"

#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace jsv {

    enum class TypeKind : std::uint8_t {
        I8,
        I16,
        I32,
        I64,
        U8,
        U16,
        U32,
        U64,
        F32,
        F64,
        Char,
        String,
        Bool,
        Composite,
        UserDefined,
        TypeVar,
        Error,
    };

    // Forward declarations
    class SHA256;
    struct TypeIdentity;
    class TypeBase;

    // ─────────────────────────────────────────────────────────────────────────────
    // Type layout and identity structures
    // ─────────────────────────────────────────────────────────────────────────────

    /// Type layout specification: immutable description of type structure
    struct TypeLayout {
        std::string canonical_binary_form;  // Canonical binary representation for SHA-256 hashing
        std::vector<std::pair<std::string, std::shared_ptr<const TypeBase>>> fields;  // Field name -> type mapping
        std::vector<uint32_t> field_offsets;  // Byte offsets for each field

        /// Compute canonical binary serialization for SHA-256 hashing
        [[nodiscard]] std::string compute_canonical_binary() const noexcept;
    };

    /// Type identity: fully qualified identifier for user-defined types
    struct TypeIdentity {
        std::string module_name;
        std::string type_name;
        std::string scope_path;  // For nested types

        /// Get fully qualified name (module::scope::type)
        [[nodiscard]] std::string fully_qualified() const noexcept;
    };

    /// @brief Base type interface (US1, MVP)
    class TypeBase {
    public:
        virtual ~TypeBase() = default;

        [[nodiscard]] virtual TypeKind kind() const noexcept = 0;
        [[nodiscard]] virtual std::string canonical_name() const noexcept = 0;
    };

    /// Primitive type entity (i8, i32, f64, etc.)
    class PrimitiveType : public TypeBase {
    public:
        [[nodiscard]] TypeKind kind() const noexcept override { return kind_; }
        [[nodiscard]] std::string canonical_name() const noexcept override;

        // Factory methods for standard primitives
        [[nodiscard]] static std::shared_ptr<const PrimitiveType> i8() noexcept;
        [[nodiscard]] static std::shared_ptr<const PrimitiveType> i16() noexcept;
        [[nodiscard]] static std::shared_ptr<const PrimitiveType> i32() noexcept;
        [[nodiscard]] static std::shared_ptr<const PrimitiveType> i64() noexcept;
        [[nodiscard]] static std::shared_ptr<const PrimitiveType> u8() noexcept;
        [[nodiscard]] static std::shared_ptr<const PrimitiveType> u16() noexcept;
        [[nodiscard]] static std::shared_ptr<const PrimitiveType> u32() noexcept;
        [[nodiscard]] static std::shared_ptr<const PrimitiveType> u64() noexcept;
        [[nodiscard]] static std::shared_ptr<const PrimitiveType> f32() noexcept;
        [[nodiscard]] static std::shared_ptr<const PrimitiveType> f64() noexcept;
        [[nodiscard]] static std::shared_ptr<const PrimitiveType> bool_() noexcept;

    private:
        explicit PrimitiveType(TypeKind k) noexcept : kind_(k) {}
        TypeKind kind_;
    };

    /// Composite type entity (struct, array, etc.)
    class CompositeType : public TypeBase {
    public:
        [[nodiscard]] TypeKind kind() const noexcept override { return TypeKind::Composite; }
        [[nodiscard]] std::string canonical_name() const noexcept override;

        // ── Construction ──────────────────────────────────────────────────────────
        explicit CompositeType(std::string composite_name) noexcept;

        // ── Accessors ─────────────────────────────────────────────────────────────
        [[nodiscard]] const std::string &name() const noexcept { return name_; }
        [[nodiscard]] const std::vector<std::shared_ptr<const TypeBase>> &member_types() const noexcept {
            return member_types_;
        }

    private:
        std::string name_;
        std::vector<std::shared_ptr<const TypeBase>> member_types_;
    };

    /// @brief User-defined type with nominal versioning and SHA-256 hashing (US1, MVP, T031)
    /// @invariant Type equivalence is NOMINAL (identity + version), not structural
    /// @invariant PROHIBITED: Changing the type of an existing Value after definition
    /// @invariant PROHIBITED: Modifying fields of an existing TypeLayout
    /// @invariant PROHIBITED: Silently reinterpreting values between type versions
    class UserDefinedType : public TypeBase {
    public:
        [[nodiscard]] TypeKind kind() const noexcept override { return TypeKind::UserDefined; }
        [[nodiscard]] std::string canonical_name() const noexcept override;

        // ── Construction with SHA-256 deterministic versioning ───────────────────
        /// Create a user-defined type with deterministic version ID from canonical binary serialization
        /// The version_id is derived from SHA-256(canonical_layout_binary), ensuring:
        /// - Deterministic across reruns (same layout -> same version)
        /// - Immutable after construction
        /// - Suitable for type equivalence checking
        explicit UserDefinedType(std::string type_name, TypeLayout layout) noexcept;

        // ── Accessors ─────────────────────────────────────────────────────────────

        /// Get type identity (fully qualified name)
        [[nodiscard]] std::string_view type_identity() const noexcept { return type_identity_; }

        /// Get deterministic version ID (SHA-256 of canonical layout)
        [[nodiscard]] std::string_view version_id() const noexcept { return version_id_; }

        /// Get type layout (immutable)
        [[nodiscard]] const TypeLayout &layout() const noexcept { return layout_; }

        /// Get equivalence rule (always "nominal" for this implementation)
        [[nodiscard]] std::string_view equivalence_rule() const noexcept { return equivalence_rule_; }

        // ── Nominal type equivalence ──────────────────────────────────────────────
        /// Two user-defined types are equivalent iff:
        /// - They have the same type_identity
        /// - They have the same version_id (which implies same layout)
        [[nodiscard]] bool is_equivalent_to(const UserDefinedType &other) const noexcept {
            return type_identity_ == other.type_identity_ && version_id_ == other.version_id_;
        }

    private:
        std::string type_identity_;
        std::string version_id_;  // Deterministic SHA-256 hash of canonical layout
        TypeLayout layout_;
        std::string equivalence_rule_;  // Always "nominal"
    };

    /// Type reference wrapper (used in Value/Function signatures)
    struct TypeRef {
        std::shared_ptr<const TypeBase> type_ptr;

        [[nodiscard]] bool operator==(const TypeRef &other) const noexcept { return type_ptr == other.type_ptr; }
    };

}  // namespace jsv

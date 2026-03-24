/*
 * Created by gbian on 24 marzo 2026.
 * Copyright (c) 2026 All rights reserved.
 */
#pragma once

// clang-format off
#include "Node.hpp"
// clang-format on

namespace jsv {

    /**
     * @brief Type kind discriminator for the Type system.
     *
     * Identifies which concrete type variant is active.
     * Mirrors the Rust Type enum structure.
     */
    enum class TypeKind : std::uint8_t {
        // Signed integer types
        I8,
        I16,
        I32,
        I64,

        // Unsigned integer types
        U8,
        U16,
        U32,
        U64,

        // Floating-point types
        F32,
        F64,

        // Character and string types
        Char,
        String,

        // Boolean type
        Bool,

        // Custom/user-defined types
        Custom,

        // Compound types
        Array,   // [T; N] - fixed-size array with element type and size expression
        Vector,  // Vec<T> - dynamic array with element type

        // Special types
        Void,    // No value (function return type)
        NullPtr, // Null pointer type
    };

    /**
     * @brief Get the string representation of a TypeKind.
     * @param kind The type kind to convert to string.
     * @return String view of the type kind name.
     */
    [[nodiscard]] std::string_view type_kind_name(TypeKind kind) noexcept;

    // ============================================================
    // Base TypeBase class - abstract base for all type variants
    // ============================================================
    class TypeBase {
    public:
        /**
         * @brief Virtual destructor for proper cleanup.
         */
        virtual ~TypeBase() = default;

        /**
         * @brief Get the type kind discriminator.
         * @return The TypeKind indicating which variant is active.
         */
        [[nodiscard]] constexpr TypeKind kind() const noexcept { return kind_; }

        /**
         * @brief Check if this is a primitive type (not compound or custom).
         * @return true if this is a built-in primitive type.
         */
        [[nodiscard]] constexpr bool is_primitive() const noexcept {
            return kind_ >= TypeKind::I8 && kind_ <= TypeKind::NullPtr &&
                   kind_ != TypeKind::Custom && kind_ != TypeKind::Array && kind_ != TypeKind::Vector;
        }

        /**
         * @brief Check if this is an integer type (signed or unsigned).
         * @return true if this is any integer type.
         */
        [[nodiscard]] constexpr bool is_integer() const noexcept {
            return (kind_ >= TypeKind::I8 && kind_ <= TypeKind::I64) ||
                   (kind_ >= TypeKind::U8 && kind_ <= TypeKind::U64);
        }

        /**
         * @brief Check if this is a signed integer type.
         * @return true if this is I8, I16, I32, or I64.
         */
        [[nodiscard]] constexpr bool is_signed_integer() const noexcept {
            return kind_ >= TypeKind::I8 && kind_ <= TypeKind::I64;
        }

        /**
         * @brief Check if this is an unsigned integer type.
         * @return true if this is U8, U16, U32, or U64.
         */
        [[nodiscard]] constexpr bool is_unsigned_integer() const noexcept {
            return kind_ >= TypeKind::U8 && kind_ <= TypeKind::U64;
        }

        /**
         * @brief Check if this is a floating-point type.
         * @return true if this is F32 or F64.
         */
        [[nodiscard]] constexpr bool is_floating_point() const noexcept {
            return kind_ == TypeKind::F32 || kind_ == TypeKind::F64;
        }

        /**
         * @brief Check if this is a numeric type (integer or floating-point).
         * @return true if this supports numeric operations.
         */
        [[nodiscard]] constexpr bool is_numeric() const noexcept {
            return is_integer() || is_floating_point();
        }

        /**
         * @brief Get string representation of the type.
         * @return String containing the type name.
         */
        [[nodiscard]] virtual std::string to_string() const = 0;

        /**
         * @brief Equality comparison operator.
         * @param other The type to compare with.
         * @return true if types are structurally equal.
         */
        [[nodiscard]] virtual bool operator==(const TypeBase &other) const noexcept = 0;

        /**
         * @brief Inequality comparison operator.
         * @param other The type to compare with.
         * @return true if types are not equal.
         */
        [[nodiscard]] constexpr bool operator!=(const TypeBase &other) const noexcept {
            return !(*this == other);
        }

    protected:
        /**
         * @brief Protected constructor - only derived classes can instantiate.
         * @param kind The type kind for this instance.
         */
        explicit constexpr TypeBase(TypeKind kind) noexcept : kind_{kind} {}

        // Delete copy/move to enforce shared_ptr usage
        TypeBase(const TypeBase &) = delete;
        TypeBase &operator=(const TypeBase &) = delete;
        TypeBase(TypeBase &&) = delete;
        TypeBase &operator=(TypeBase &&) = delete;

    private:
        TypeKind kind_;
    };

    // ============================================================
    // Primitive Type - for simple types without additional data
    // ============================================================
    class PrimitiveType final : public TypeBase {
    public:
        /**
         * @brief Construct a primitive type.
         * @param kind The primitive type kind.
         * @pre kind must be a primitive type (not Custom, Array, or Vector).
         */
        explicit constexpr PrimitiveType(TypeKind kind) : TypeBase{kind} {
            assert(kind != TypeKind::Custom && "Use CustomType for custom types");
            assert(kind != TypeKind::Array && "Use ArrayType for array types");
            assert(kind != TypeKind::Vector && "Use VectorType for vector types");
        }

        /**
         * @brief Create I8 type.
         * @return Shared pointer to I8 type.
         */
        [[nodiscard]] static std::shared_ptr<const PrimitiveType> i8() {
            static const auto instance = std::shared_ptr<const PrimitiveType>(new PrimitiveType(TypeKind::I8));
            return instance;
        }

        /**
         * @brief Create I16 type.
         * @return Shared pointer to I16 type.
         */
        [[nodiscard]] static std::shared_ptr<const PrimitiveType> i16() {
            static const auto instance = std::shared_ptr<const PrimitiveType>(new PrimitiveType(TypeKind::I16));
            return instance;
        }

        /**
         * @brief Create I32 type.
         * @return Shared pointer to I32 type.
         */
        [[nodiscard]] static std::shared_ptr<const PrimitiveType> i32() {
            static const auto instance = std::shared_ptr<const PrimitiveType>(new PrimitiveType(TypeKind::I32));
            return instance;
        }

        /**
         * @brief Create I64 type.
         * @return Shared pointer to I64 type.
         */
        [[nodiscard]] static std::shared_ptr<const PrimitiveType> i64() {
            static const auto instance = std::shared_ptr<const PrimitiveType>(new PrimitiveType(TypeKind::I64));
            return instance;
        }

        /**
         * @brief Create U8 type.
         * @return Shared pointer to U8 type.
         */
        [[nodiscard]] static std::shared_ptr<const PrimitiveType> u8() {
            static const auto instance = std::shared_ptr<const PrimitiveType>(new PrimitiveType(TypeKind::U8));
            return instance;
        }

        /**
         * @brief Create U16 type.
         * @return Shared pointer to U16 type.
         */
        [[nodiscard]] static std::shared_ptr<const PrimitiveType> u16() {
            static const auto instance = std::shared_ptr<const PrimitiveType>(new PrimitiveType(TypeKind::U16));
            return instance;
        }

        /**
         * @brief Create U32 type.
         * @return Shared pointer to U32 type.
         */
        [[nodiscard]] static std::shared_ptr<const PrimitiveType> u32() {
            static const auto instance = std::shared_ptr<const PrimitiveType>(new PrimitiveType(TypeKind::U32));
            return instance;
        }

        /**
         * @brief Create U64 type.
         * @return Shared pointer to U64 type.
         */
        [[nodiscard]] static std::shared_ptr<const PrimitiveType> u64() {
            static const auto instance = std::shared_ptr<const PrimitiveType>(new PrimitiveType(TypeKind::U64));
            return instance;
        }

        /**
         * @brief Create F32 type.
         * @return Shared pointer to F32 type.
         */
        [[nodiscard]] static std::shared_ptr<const PrimitiveType> f32() {
            static const auto instance = std::shared_ptr<const PrimitiveType>(new PrimitiveType(TypeKind::F32));
            return instance;
        }

        /**
         * @brief Create F64 type.
         * @return Shared pointer to F64 type.
         */
        [[nodiscard]] static std::shared_ptr<const PrimitiveType> f64() {
            static const auto instance = std::shared_ptr<const PrimitiveType>(new PrimitiveType(TypeKind::F64));
            return instance;
        }

        /**
         * @brief Create Char type.
         * @return Shared pointer to Char type.
         */
        [[nodiscard]] static std::shared_ptr<const PrimitiveType> char_() {
            static const auto instance = std::shared_ptr<const PrimitiveType>(new PrimitiveType(TypeKind::Char));
            return instance;
        }

        /**
         * @brief Create String type.
         * @return Shared pointer to String type.
         */
        [[nodiscard]] static std::shared_ptr<const PrimitiveType> string() {
            static const auto instance = std::shared_ptr<const PrimitiveType>(new PrimitiveType(TypeKind::String));
            return instance;
        }

        /**
         * @brief Create Bool type.
         * @return Shared pointer to Bool type.
         */
        [[nodiscard]] static std::shared_ptr<const PrimitiveType> bool_() {
            static const auto instance = std::shared_ptr<const PrimitiveType>(new PrimitiveType(TypeKind::Bool));
            return instance;
        }

        /**
         * @brief Create Void type.
         * @return Shared pointer to Void type.
         */
        [[nodiscard]] static std::shared_ptr<const PrimitiveType> void_() {
            static const auto instance = std::shared_ptr<const PrimitiveType>(new PrimitiveType(TypeKind::Void));
            return instance;
        }

        /**
         * @brief Create NullPtr type.
         * @return Shared pointer to NullPtr type.
         */
        [[nodiscard]] static std::shared_ptr<const PrimitiveType> nullptr_() {
            static const auto instance = std::shared_ptr<const PrimitiveType>(new PrimitiveType(TypeKind::NullPtr));
            return instance;
        }

        /**
         * @brief String representation.
         * @return Type name as string.
         */
        [[nodiscard]] std::string to_string() const override;

        /**
         * @brief Equality comparison.
         * @param other The type to compare with.
         * @return true if both types have the same kind.
         */
        [[nodiscard]] bool operator==(const TypeBase &other) const noexcept override {
            if(other.kind() != kind()) { return false; }
            return true;
        }

        /**
         * @brief Type check for PrimitiveType.
         * @param n Type to check.
         * @return true if n is a PrimitiveType.
         */
        [[nodiscard]] static constexpr bool classof(const TypeBase *n) {
            return n->kind() == TypeKind::I8 || n->kind() == TypeKind::I16 || n->kind() == TypeKind::I32 ||
                   n->kind() == TypeKind::I64 || n->kind() == TypeKind::U8 || n->kind() == TypeKind::U16 ||
                   n->kind() == TypeKind::U32 || n->kind() == TypeKind::U64 || n->kind() == TypeKind::F32 ||
                   n->kind() == TypeKind::F64 || n->kind() == TypeKind::Char || n->kind() == TypeKind::String ||
                   n->kind() == TypeKind::Bool || n->kind() == TypeKind::Void || n->kind() == TypeKind::NullPtr;
        }
    };

    // ============================================================
    // CustomType - user-defined types
    // ============================================================
    class CustomType final : public TypeBase {
    public:
        /**
         * @brief Construct a custom type.
         * @param name Custom type name.
         */
        explicit CustomType(std::string_view name)
            : TypeBase{TypeKind::Custom}, name_{std::make_shared<const std::string>(name)} {}

        /**
         * @brief Get the custom type name.
         * @return String view of the type name.
         */
        [[nodiscard]] std::string_view name() const noexcept { return *name_; }

        /**
         * @brief String representation.
         * @return Type name as string.
         */
        [[nodiscard]] std::string to_string() const override;

        /**
         * @brief Equality comparison.
         * @param other The type to compare with.
         * @return true if both types have the same name.
         */
        [[nodiscard]] bool operator==(const TypeBase &other) const noexcept override {
            if(other.kind() != kind()) { return false; }
            const auto *other_custom = dynamic_cast<const CustomType *>(&other);
            assert(other_custom && "Type kind check passed but cast failed");
            return *name_ == *other_custom->name_;
        }

        /**
         * @brief Type check for CustomType.
         * @param n Type to check.
         * @return true if n is a CustomType.
         */
        [[nodiscard]] static constexpr bool classof(const TypeBase *n) {
            return n->kind() == TypeKind::Custom;
        }

    private:
        std::shared_ptr<const std::string> name_;
    };

    // ============================================================
    // ArrayType - fixed-size array [T; N]
    // ============================================================
    class ArrayType final : public TypeBase {
    public:
        /**
         * @brief Construct an array type.
         * @param element_type Type of array elements.
         * @param size_expr Compile-time constant expression for array size.
         */
        ArrayType(std::shared_ptr<const TypeBase> element_type, std::shared_ptr<const Expr> size_expr)
            : TypeBase{TypeKind::Array}, element_type_{std::move(element_type)}, size_expr_{std::move(size_expr)} {
            assert(element_type_ && "Element type cannot be null");
            assert(size_expr_ && "Size expression cannot be null");
        }

        /**
         * @brief Get the element type.
         * @return Shared pointer to the element type.
         */
        [[nodiscard]] const std::shared_ptr<const TypeBase> &element_type() const noexcept { return element_type_; }

        /**
         * @brief Get the size expression.
         * @return Shared pointer to the size expression.
         */
        [[nodiscard]] const std::shared_ptr<const Expr> &size_expr() const noexcept { return size_expr_; }

        /**
         * @brief String representation.
         * @return Type name as string (e.g., "[i32; 10]").
         */
        [[nodiscard]] std::string to_string() const override;

        /**
         * @brief Equality comparison.
         * @param other The type to compare with.
         * @return true if both element types and size expressions are equal.
         */
        [[nodiscard]] bool operator==(const TypeBase &other) const noexcept override {
            if(other.kind() != kind()) { return false; }
            const auto *other_array = dynamic_cast<const ArrayType *>(&other);
            assert(other_array && "Type kind check passed but cast failed");
            // Compare element types and use pointer equality for expressions
            return *element_type_ == *other_array->element_type_ &&
                   (size_expr_ == other_array->size_expr_);
        }

        /**
         * @brief Type check for ArrayType.
         * @param n Type to check.
         * @return true if n is an ArrayType.
         */
        [[nodiscard]] static constexpr bool classof(const TypeBase *n) {
            return n->kind() == TypeKind::Array;
        }

    private:
        std::shared_ptr<const TypeBase> element_type_;
        std::shared_ptr<const Expr> size_expr_;
    };

    // ============================================================
    // VectorType - dynamic array Vec<T>
    // ============================================================
    class VectorType final : public TypeBase {
    public:
        /**
         * @brief Construct a vector type.
         * @param element_type Type of vector elements.
         */
        explicit VectorType(std::shared_ptr<const TypeBase> element_type)
            : TypeBase{TypeKind::Vector}, element_type_{std::move(element_type)} {
            assert(element_type_ && "Element type cannot be null");
        }

        /**
         * @brief Get the element type.
         * @return Shared pointer to the element type.
         */
        [[nodiscard]] const std::shared_ptr<const TypeBase> &element_type() const noexcept { return element_type_; }

        /**
         * @brief String representation.
         * @return Type name as string (e.g., "Vec<i32>").
         */
        [[nodiscard]] std::string to_string() const override;

        /**
         * @brief Equality comparison.
         * @param other The type to compare with.
         * @return true if both element types are equal.
         */
        [[nodiscard]] bool operator==(const TypeBase &other) const noexcept override {
            if(other.kind() != kind()) { return false; }
            const auto *other_vector = dynamic_cast<const VectorType *>(&other);
            assert(other_vector && "Type kind check passed but cast failed");
            return *element_type_ == *other_vector->element_type_;
        }

        /**
         * @brief Type check for VectorType.
         * @param n Type to check.
         * @return true if n is a VectorType.
         */
        [[nodiscard]] static constexpr bool classof(const TypeBase *n) {
            return n->kind() == TypeKind::Vector;
        }

    private:
        std::shared_ptr<const TypeBase> element_type_;
    };

    // ============================================================
    // Type alias for shared pointer
    // ============================================================
    using TypePtr = std::shared_ptr<const TypeBase>;

}  // namespace jsv

// -------------------------------------------------------------------------
// std::formatter  (C++23 <format>)
// -------------------------------------------------------------------------
namespace std {
    template <>
    struct formatter<jsv::TypePtr> : formatter<std::string_view> {
        template <typename FormatContext>
        auto format(const jsv::TypePtr &type, FormatContext &ctx) const {
            if(type) {
                return formatter<std::string_view>::format(type->to_string(), ctx);
            }
            return formatter<std::string_view>::format("none", ctx);
        }
    };
}  // namespace std

// -------------------------------------------------------------------------
// fmt::formatter  (fmtlib)
// -------------------------------------------------------------------------
template <>
struct fmt::formatter<jsv::TypePtr> : fmt::formatter<std::string_view> {
    template <typename FormatContext>
    auto format(const jsv::TypePtr &type, FormatContext &ctx) const {
        if(type) {
            return fmt::formatter<std::string_view>::format(type->to_string(), ctx);
        }
        return fmt::formatter<std::string_view>::format("none", ctx);
    }
};

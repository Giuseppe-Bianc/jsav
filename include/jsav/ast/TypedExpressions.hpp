/*
 * Created by gbian on 1 aprile 2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

// clang-format off
#include "jsav/ast/TypedNode.hpp"
// clang-format on

namespace jsv {

    // ============================================================
    // Typed Literal Nodes
    // ============================================================

    /**
     * @brief Typed integer literal.
     *
     * Represents an integer literal with its resolved type.
     * Example: `42` with type i32, or `42u64` with type u64.
     */
    class TypedIntegerLiteral final : public TypedExpr {
    public:
        TypedIntegerLiteral(std::int64_t value, TypePtr node_type, SourceSpan loc = {},
                            std::optional<std::string> type_suffix = std::nullopt)
          : TypedExpr{NodeKind::IntegerLiteral, std::move(node_type), loc}, value_{value}, type_suffix_{std::move(type_suffix)} {}

        [[nodiscard]] std::int64_t value() const noexcept { return value_; }
        [[nodiscard]] const std::optional<std::string> &type_suffix() const noexcept { return type_suffix_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::IntegerLiteral; }

    private:
        std::int64_t value_;
        std::optional<std::string> type_suffix_;
    };

    /**
     * @brief Typed float literal.
     *
     * Represents a floating-point literal with its resolved type.
     * Example: `3.14` with type f64, or `3.14f32` with type f32.
     */
    class TypedFloatLiteral final : public TypedExpr {
    public:
        TypedFloatLiteral(double value, TypePtr node_type, SourceSpan loc = {})
          : TypedExpr{NodeKind::FloatLiteral, std::move(node_type), loc}, value_{value} {}

        [[nodiscard]] double value() const noexcept { return value_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::FloatLiteral; }

    private:
        double value_;
    };

    /**
     * @brief Typed string literal.
     *
     * Represents a string literal with type `string`.
     */
    class TypedStringLiteral final : public TypedExpr {
    public:
        TypedStringLiteral(std::string value, TypePtr node_type, SourceSpan loc = {})
          : TypedExpr{NodeKind::StringLiteral, std::move(node_type), loc}, value_{std::move(value)} {}

        [[nodiscard]] const std::string &value() const noexcept { return value_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::StringLiteral; }

    private:
        std::string value_;
    };

    /**
     * @brief Typed character literal.
     *
     * Represents a character literal with type `char`.
     */
    class TypedCharLiteral final : public TypedExpr {
    public:
        TypedCharLiteral(char value, TypePtr node_type, SourceSpan loc = {})
          : TypedExpr{NodeKind::CharLiteral, std::move(node_type), loc}, value_{value} {}

        [[nodiscard]] char value() const noexcept { return value_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::CharLiteral; }

    private:
        char value_;
    };

    /**
     * @brief Typed boolean literal.
     *
     * Represents a boolean literal with type `bool`.
     */
    class TypedBoolLiteral final : public TypedExpr {
    public:
        TypedBoolLiteral(bool value, TypePtr node_type, SourceSpan loc = {})
          : TypedExpr{NodeKind::BoolLiteral, std::move(node_type), loc}, value_{value} {}

        [[nodiscard]] bool value() const noexcept { return value_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::BoolLiteral; }

    private:
        bool value_;
    };

    /**
     * @brief Typed null literal.
     *
     * Represents a null literal with type `nullptr`.
     */
    class TypedNullLiteral final : public TypedExpr {
    public:
        explicit TypedNullLiteral(TypePtr node_type, SourceSpan loc = {}) : TypedExpr{NodeKind::NullLiteral, std::move(node_type), loc} {}

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::NullLiteral; }
    };

    // ============================================================
    // Typed Identifier and References
    // ============================================================

    /**
     * @brief Typed identifier.
     *
     * Represents an identifier reference with its resolved type.
     * The type is determined by the declaration being referenced.
     */
    class TypedIdentifier final : public TypedExpr {
    public:
        TypedIdentifier(std::string name, TypePtr node_type, SourceSpan loc = {})
          : TypedExpr{NodeKind::Identifier, std::move(node_type), loc}, name_{std::move(name)} {}

        [[nodiscard]] const std::string &name() const noexcept { return name_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::Identifier; }

    private:
        std::string name_;
    };

    // ============================================================
    // Typed Expression Nodes
    // ============================================================

    /**
     * @brief Typed unary expression.
     *
     * Represents a unary operation with its resolved result type.
     * Example: `-x` (negation), `!flag` (logical not).
     */
    class TypedUnaryExpr final : public TypedExpr {
    public:
        TypedUnaryExpr(UnaryOp op, TypedExprPtr operand, TypePtr node_type, SourceSpan loc = {})
          : TypedExpr{NodeKind::UnaryExpr, std::move(node_type), loc}, op_{op}, operand_{std::move(operand)} {}

        [[nodiscard]] UnaryOp op() const noexcept { return op_; }
        [[nodiscard]] const TypedExpr &operand() const noexcept { return *operand_; }
        [[nodiscard]] TypedExpr &operand() noexcept { return *operand_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::UnaryExpr; }

    private:
        UnaryOp op_;
        TypedExprPtr operand_;
    };

    /**
     * @brief Typed binary expression.
     *
     * Represents a binary operation with its resolved result type.
     * Example: `a + b` (addition), `x == y` (comparison).
     */
    class TypedBinaryExpr final : public TypedExpr {
    public:
        TypedBinaryExpr(BinaryOp op, TypedExprPtr lhs, TypedExprPtr rhs, TypePtr node_type, SourceSpan loc = {})
          : TypedExpr{NodeKind::BinaryExpr, std::move(node_type), loc}, op_{op}, lhs_{std::move(lhs)}, rhs_{std::move(rhs)} {}

        [[nodiscard]] BinaryOp op() const noexcept { return op_; }
        [[nodiscard]] const TypedExpr &lhs() const noexcept { return *lhs_; }
        [[nodiscard]] const TypedExpr &rhs() const noexcept { return *rhs_; }
        [[nodiscard]] TypedExpr &lhs() noexcept { return *lhs_; }
        [[nodiscard]] TypedExpr &rhs() noexcept { return *rhs_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::BinaryExpr; }

    private:
        BinaryOp op_;
        TypedExprPtr lhs_;
        TypedExprPtr rhs_;
    };

    /**
     * @brief Typed ternary (conditional) expression.
     *
     * Represents a conditional expression with its resolved result type.
     * Example: `cond ? then_expr : else_expr`
     */
    class TypedTernaryExpr final : public TypedExpr {
    public:
        TypedTernaryExpr(TypedExprPtr condition, TypedExprPtr then_expr, TypedExprPtr else_expr, TypePtr node_type, SourceSpan loc = {})
          : TypedExpr{NodeKind::TernaryExpr, std::move(node_type), loc}, condition_{std::move(condition)}, then_expr_{std::move(then_expr)},
            else_expr_{std::move(else_expr)} {}

        [[nodiscard]] const TypedExpr &condition() const noexcept { return *condition_; }
        [[nodiscard]] const TypedExpr &then_expr() const noexcept { return *then_expr_; }
        [[nodiscard]] const TypedExpr &else_expr() const noexcept { return *else_expr_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::TernaryExpr; }

    private:
        TypedExprPtr condition_;
        TypedExprPtr then_expr_;
        TypedExprPtr else_expr_;
    };

    /**
     * @brief Typed call expression.
     *
     * Represents a function call with its resolved return type.
     * Example: `foo(a, b, c)` where the type is foo's return type.
     */
    class TypedCallExpr final : public TypedExpr {
    public:
        TypedCallExpr(TypedExprPtr callee, std::vector<TypedExprPtr> args, TypePtr node_type, SourceSpan loc = {})
          : TypedExpr{NodeKind::CallExpr, std::move(node_type), loc}, callee_{std::move(callee)}, args_{std::move(args)} {}

        [[nodiscard]] const TypedExpr &callee() const noexcept { return *callee_; }
        [[nodiscard]] const std::vector<TypedExprPtr> &args() const noexcept { return args_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::CallExpr; }

    private:
        TypedExprPtr callee_;
        std::vector<TypedExprPtr> args_;
    };

    /**
     * @brief Typed index expression.
     *
     * Represents an array/vector indexing operation with its resolved element type.
     * Example: `arr[idx]` where the type is the element type of arr.
     */
    class TypedIndexExpr final : public TypedExpr {
    public:
        TypedIndexExpr(TypedExprPtr object, TypedExprPtr index, TypePtr node_type, SourceSpan loc = {})
          : TypedExpr{NodeKind::IndexExpr, std::move(node_type), loc}, object_{std::move(object)}, index_{std::move(index)} {}

        [[nodiscard]] const TypedExpr &object() const noexcept { return *object_; }
        [[nodiscard]] const TypedExpr &index() const noexcept { return *index_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::IndexExpr; }

    private:
        TypedExprPtr object_;
        TypedExprPtr index_;
    };

    /**
     * @brief Typed member access expression.
     *
     * Represents a struct/class member access with its resolved member type.
     * Example: `obj.field` where the type is the field's declared type.
     */
    class TypedMemberExpr final : public TypedExpr {
    public:
        TypedMemberExpr(TypedExprPtr object, std::string member, TypePtr node_type, SourceSpan loc = {})
          : TypedExpr{NodeKind::MemberExpr, std::move(node_type), loc}, object_{std::move(object)}, member_{std::move(member)} {}

        [[nodiscard]] const TypedExpr &object() const noexcept { return *object_; }
        [[nodiscard]] const std::string &member() const noexcept { return member_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::MemberExpr; }

    private:
        TypedExprPtr object_;
        std::string member_;
    };

    /**
     * @brief Typed assignment expression.
     *
     * Represents an assignment operation. The type is the type of the target.
     * Example: `x = expr` where the type is the type of x.
     */
    class TypedAssignExpr final : public TypedExpr {
    public:
        TypedAssignExpr(TypedExprPtr target, TypedExprPtr value, TypePtr node_type, SourceSpan loc = {})
          : TypedExpr{NodeKind::AssignExpr, std::move(node_type), loc}, target_{std::move(target)}, value_{std::move(value)} {}

        [[nodiscard]] const TypedExpr &target() const noexcept { return *target_; }
        [[nodiscard]] const TypedExpr &value() const noexcept { return *value_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::AssignExpr; }

    private:
        TypedExprPtr target_;
        TypedExprPtr value_;
    };

    /**
     * @brief Typed cast expression.
     *
     * Represents an explicit type cast with the target type.
     * Example: `(int)expr` or `cast<i32>(expr)`.
     */
    class TypedCastExpr final : public TypedExpr {
    public:
        TypedCastExpr(std::string target_type, TypedExprPtr operand, TypePtr node_type, SourceSpan loc = {})
          : TypedExpr{NodeKind::CastExpr, std::move(node_type), loc}, target_type_{std::move(target_type)}, operand_{std::move(operand)} {}

        [[nodiscard]] const std::string &target_type() const noexcept { return target_type_; }
        [[nodiscard]] const TypedExpr &operand() const noexcept { return *operand_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::CastExpr; }

    private:
        std::string target_type_;
        TypedExprPtr operand_;
    };

    /**
     * @brief Typed array literal.
     *
     * Represents an array literal with its resolved array type.
     * Example: `[1, 2, 3]` with type `[i32; 3]`.
     */
    class TypedArrayLiteral final : public TypedExpr {
    public:
        TypedArrayLiteral(std::vector<TypedExprPtr> elements, TypePtr node_type, SourceSpan loc = {})
          : TypedExpr{NodeKind::ArrayLiteral, std::move(node_type), loc}, elements_{std::move(elements)} {}

        [[nodiscard]] const std::vector<TypedExprPtr> &elements() const noexcept { return elements_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::ArrayLiteral; }

    private:
        std::vector<TypedExprPtr> elements_;
    };

    /**
     * @brief Typed grouping expression.
     *
     * Represents a parenthesized expression. The type is the same as the inner expression.
     * Example: `(expr)` preserves the type of expr.
     */
    class TypedGroupingExpr final : public TypedExpr {
    public:
        TypedGroupingExpr(TypedExprPtr expression, TypePtr node_type, SourceSpan loc = {})
          : TypedExpr{NodeKind::GroupingExpr, std::move(node_type), loc}, expression_{std::move(expression)} {}

        [[nodiscard]] const TypedExpr &expression() const noexcept { return *expression_; }
        [[nodiscard]] TypedExpr &expression() noexcept { return *expression_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::GroupingExpr; }

    private:
        TypedExprPtr expression_;
    };

}  // namespace jsv

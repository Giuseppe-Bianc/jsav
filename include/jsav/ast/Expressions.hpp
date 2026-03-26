/*
 * Created by gbian on 17/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */
#pragma once

// clang-format off
#include "Node.hpp"
// clang-format on

namespace jsv {
    // ============================================================
    // Integer Literal:  42
    // ============================================================
    class IntegerLiteral final : public Expr {
    public:
        explicit IntegerLiteral(std::int64_t value, SourceSpan loc = {}, std::optional<std::string> type_suffix = std::nullopt)
          : Expr(NodeKind::IntegerLiteral, loc), value_{value}, type_suffix_{std::move(type_suffix)} {}

        [[nodiscard]] std::int64_t value() const noexcept { return value_; }
        [[nodiscard]] const std::optional<std::string> &type_suffix() const noexcept { return type_suffix_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::IntegerLiteral; }

    private:
        std::int64_t value_;
        std::optional<std::string> type_suffix_;
    };
    // ============================================================
    // Float Literal:  3.14
    // ============================================================
    class FloatLiteral final : public Expr {
    public:
        explicit FloatLiteral(double value, SourceSpan loc = {}) : Expr(NodeKind::FloatLiteral, loc), value_{value} {}

        [[nodiscard]] double value() const noexcept { return value_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::FloatLiteral; }

    private:
        double value_;
    };

    // ============================================================
    // String Literal:  "hello"
    // ============================================================
    class StringLiteral final : public Expr {
    public:
        explicit StringLiteral(std::string value, SourceSpan loc = {}) : Expr(NodeKind::StringLiteral, loc), value_{std::move(value)} {}

        [[nodiscard]] const std::string &value() const noexcept { return value_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::StringLiteral; }

    private:
        std::string value_;
    };

    // ============================================================
    // String Literal:  "hello"
    // ============================================================
    class CharLiteral final : public Expr {
    public:
        explicit CharLiteral(char value, SourceSpan loc = {}) : Expr(NodeKind::CharLiteral, loc), value_{value} {}

        [[nodiscard]] char value() const noexcept { return value_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::CharLiteral; }

    private:
        char value_;
    };

    // ============================================================
    // Bool Literal:  true / false
    // ============================================================
    class BoolLiteral final : public Expr {
    public:
        explicit BoolLiteral(bool value, SourceSpan loc = {}) : Expr(NodeKind::BoolLiteral, loc), value_{value} {}

        [[nodiscard]] bool value() const noexcept { return value_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::BoolLiteral; }

    private:
        bool value_;
    };

    // ============================================================
    // Null Literal: null
    // ============================================================
    class NullLiteral final : public Expr {
    public:
        explicit NullLiteral(SourceSpan loc = {}) : Expr(NodeKind::NullLiteral, loc) {}

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::NullLiteral; }
    };

    // ============================================================
    // Identifier:  x, foo, myVar
    // ============================================================
    class Identifier final : public Expr {
    public:
        explicit Identifier(std::string name, SourceSpan loc = {}) : Expr(NodeKind::Identifier, loc), name_{std::move(name)} {}

        [[nodiscard]] const std::string &name() const noexcept { return name_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::Identifier; }

    private:
        std::string name_;
    };

    // ============================================================
    // Unary Expression:  -x, !flag, ++i
    // ============================================================
    class UnaryExpr final : public Expr {
    public:
        UnaryExpr(UnaryOp op, ExprPtr operand, SourceSpan loc = {})
          : Expr(NodeKind::UnaryExpr, loc), op_{op}, operand_{std::move(operand)} {}

        [[nodiscard]] UnaryOp op() const noexcept { return op_; }
        [[nodiscard]] const Expr &operand() const noexcept { return *operand_; }
        [[nodiscard]] Expr &operand() noexcept { return *operand_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::UnaryExpr; }

    private:
        UnaryOp op_;
        ExprPtr operand_;
    };

    // ============================================================
    // Binary Expression:  a + b, x == y
    // ============================================================
    class BinaryExpr final : public Expr {
    public:
        BinaryExpr(BinaryOp op, ExprPtr lhs, ExprPtr rhs, SourceSpan loc = {})
          : Expr(NodeKind::BinaryExpr, loc), op_{op}, lhs_{std::move(lhs)}, rhs_{std::move(rhs)} {}

        [[nodiscard]] BinaryOp op() const noexcept { return op_; }
        [[nodiscard]] const Expr &lhs() const noexcept { return *lhs_; }
        [[nodiscard]] const Expr &rhs() const noexcept { return *rhs_; }
        [[nodiscard]] Expr &lhs() noexcept { return *lhs_; }
        [[nodiscard]] Expr &rhs() noexcept { return *rhs_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::BinaryExpr; }

    private:
        BinaryOp op_;
        ExprPtr lhs_;
        ExprPtr rhs_;
    };

    // ============================================================
    // Ternary Expression:  cond ? then : else
    // ============================================================
    class TernaryExpr final : public Expr {
    public:
        TernaryExpr(ExprPtr condition, ExprPtr then_expr, ExprPtr else_expr, SourceSpan loc = {})
          : Expr(NodeKind::TernaryExpr, loc), condition_{std::move(condition)}, then_expr_{std::move(then_expr)},
            else_expr_{std::move(else_expr)} {}

        [[nodiscard]] const Expr &condition() const noexcept { return *condition_; }
        [[nodiscard]] const Expr &then_expr() const noexcept { return *then_expr_; }
        [[nodiscard]] const Expr &else_expr() const noexcept { return *else_expr_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::TernaryExpr; }

    private:
        ExprPtr condition_;
        ExprPtr then_expr_;
        ExprPtr else_expr_;
    };

    // ============================================================
    // Call Expression:  foo(a, b, c)
    // ============================================================
    class CallExpr final : public Expr {
    public:
        CallExpr(ExprPtr callee, std::vector<ExprPtr> args, SourceSpan loc = {})
          : Expr(NodeKind::CallExpr, loc), callee_{std::move(callee)}, args_{std::move(args)} {}

        [[nodiscard]] const Expr &callee() const noexcept { return *callee_; }
        [[nodiscard]] const std::vector<ExprPtr> &args() const noexcept { return args_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::CallExpr; }

    private:
        ExprPtr callee_;
        std::vector<ExprPtr> args_;
    };

    // ============================================================
    // Index Expression:  arr[idx]
    // ============================================================
    class IndexExpr final : public Expr {
    public:
        IndexExpr(ExprPtr object, ExprPtr index, SourceSpan loc = {})
          : Expr(NodeKind::IndexExpr, loc), object_{std::move(object)}, index_{std::move(index)} {}

        [[nodiscard]] const Expr &object() const noexcept { return *object_; }
        [[nodiscard]] const Expr &index() const noexcept { return *index_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::IndexExpr; }

    private:
        ExprPtr object_;
        ExprPtr index_;
    };

    // ============================================================
    // Member Expression:  obj.field
    // ============================================================
    class MemberExpr final : public Expr {
    public:
        MemberExpr(ExprPtr object, std::string member, SourceSpan loc = {})
          : Expr(NodeKind::MemberExpr, loc), object_{std::move(object)}, member_{std::move(member)} {}

        [[nodiscard]] const Expr &object() const noexcept { return *object_; }
        [[nodiscard]] const std::string &member() const noexcept { return member_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::MemberExpr; }

    private:
        ExprPtr object_;
        std::string member_;
    };

    // ============================================================
    // Assign Expression:  x = expr
    // ============================================================
    class AssignExpr final : public Expr {
    public:
        AssignExpr(ExprPtr target, ExprPtr value, SourceSpan loc = {})
          : Expr(NodeKind::AssignExpr, loc), target_{std::move(target)}, value_{std::move(value)} {}

        [[nodiscard]] const Expr &target() const noexcept { return *target_; }
        [[nodiscard]] const Expr &value() const noexcept { return *value_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::AssignExpr; }

    private:
        ExprPtr target_;
        ExprPtr value_;
    };

    // ============================================================
    // Cast Expression:  (int)expr  oppure  cast<int>(expr)
    // ============================================================
    class CastExpr final : public Expr {
    public:
        CastExpr(std::string target_type, ExprPtr operand, SourceSpan loc = {})
          : Expr(NodeKind::CastExpr, loc), target_type_{std::move(target_type)}, operand_{std::move(operand)} {}

        [[nodiscard]] const std::string &target_type() const noexcept { return target_type_; }
        [[nodiscard]] const Expr &operand() const noexcept { return *operand_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::CastExpr; }

    private:
        std::string target_type_;
        ExprPtr operand_;
    };

    // ============================================================
    // Array Literal:  [1, 2, 3]
    // ============================================================
    class ArrayLiteral final : public Expr {
    public:
        explicit ArrayLiteral(std::vector<ExprPtr> elements, SourceSpan loc = {})
          : Expr(NodeKind::ArrayLiteral, loc), elements_{std::move(elements)} {}

        [[nodiscard]] const std::vector<ExprPtr> &elements() const noexcept { return elements_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::ArrayLiteral; }

    private:
        std::vector<ExprPtr> elements_;
    };

    // ============================================================
    // Grouping Expression:  (expr) - parenthesized expression
    // ============================================================
    class GroupingExpr final : public Expr {
    public:
        explicit GroupingExpr(ExprPtr expression, SourceSpan loc = {})
          : Expr(NodeKind::GroupingExpr, loc), expression_{std::move(expression)} {}

        [[nodiscard]] const Expr &expression() const noexcept { return *expression_; }
        [[nodiscard]] Expr &expression() noexcept { return *expression_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::GroupingExpr; }

    private:
        ExprPtr expression_;
    };

}  // namespace jsv

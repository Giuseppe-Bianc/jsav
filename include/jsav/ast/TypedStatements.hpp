/*
 * Created by gbian on 1 aprile 2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

// clang-format off
#include "jsav/ast/TypedNode.hpp"
#include "jsav/ast/TypedExpressions.hpp"
// clang-format on

namespace jsv {

    // ============================================================
    // Typed Statement Nodes
    // ============================================================

    /**
     * @brief Typed expression statement.
     *
     * Represents an expression used as a statement. Type is typically void.
     * Example: `foo();` or `x + 1;`
     */
    class TypedExprStmt final : public TypedStmt {
    public:
        TypedExprStmt(TypedExprPtr expression, TypePtr node_type, SourceSpan loc = {})
          : TypedStmt{NodeKind::ExprStmt, std::move(node_type), loc}, expression_{std::move(expression)} {}

        [[nodiscard]] const TypedExpr &expression() const noexcept { return *expression_; }
        [[nodiscard]] TypedExpr &expression() noexcept { return *expression_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::ExprStmt; }

    private:
        TypedExprPtr expression_;
    };

    /**
     * @brief Typed variable declaration.
     *
     * Represents a variable declaration with its resolved type.
     * Example: `var x: i32 = 42;` or `let y = 3.14;` (type inferred).
     */
    class TypedVarDecl final : public TypedStmt {
    public:
        TypedVarDecl(std::string name, TypePtr resolved_type, TypedExprPtr initializer, bool is_const = false, SourceSpan loc = {})
          : TypedStmt{NodeKind::VarDecl, std::move(resolved_type), loc}, name_{std::move(name)}, initializer_{std::move(initializer)},
            is_const_{is_const} {}

        [[nodiscard]] const std::string &name() const noexcept { return name_; }
        [[nodiscard]] const TypedExpr &initializer() const noexcept { return *initializer_; }
        [[nodiscard]] bool has_initializer() const noexcept { return initializer_ != nullptr; }
        [[nodiscard]] bool is_const() const noexcept { return is_const_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::VarDecl; }

    private:
        std::string name_;
        TypedExprPtr initializer_;
        bool is_const_;
    };

    /**
     * @brief Typed block statement.
     *
     * Represents a block of statements with optional scope type information.
     * Example: `{ stmt1; stmt2; ... }`
     */
    class TypedBlockStmt final : public TypedStmt {
    public:
        TypedBlockStmt(std::vector<TypedStmtPtr> statements, TypePtr node_type, SourceSpan loc = {})
          : TypedStmt{NodeKind::BlockStmt, std::move(node_type), loc}, statements_{std::move(statements)} {}

        [[nodiscard]] const std::vector<TypedStmtPtr> &statements() const noexcept { return statements_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::BlockStmt; }

    private:
        std::vector<TypedStmtPtr> statements_;
    };

    /**
     * @brief Typed function declaration.
     *
     * Represents a function with typed parameters and return type.
     * Example: `fn foo(a: i32, b: i32) -> i32 { ... }`
     */
    struct TypedFuncParam {
        std::string name;         ///< Parameter name
        TypePtr type_annotation;  ///< Resolved parameter type
        SourceSpan loc;           ///< Source location
    };

    class TypedFuncDecl final : public TypedStmt {
    public:
        TypedFuncDecl(std::string name, std::vector<TypedFuncParam> params, std::optional<TypePtr> return_type,
                      std::unique_ptr<TypedBlockStmt> body, TypePtr node_type, SourceSpan loc = {})
          : TypedStmt{NodeKind::FuncDecl, std::move(node_type), loc}, name_{std::move(name)}, params_{std::move(params)},
            return_type_{std::move(return_type)}, body_{std::move(body)} {}

        [[nodiscard]] const std::string &name() const noexcept { return name_; }
        [[nodiscard]] const std::vector<TypedFuncParam> &params() const noexcept { return params_; }
        [[nodiscard]] const std::optional<TypePtr> &return_type() const noexcept { return return_type_; }
        [[nodiscard]] const TypedBlockStmt &body() const noexcept { return *body_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::FuncDecl; }

    private:
        std::string name_;
        std::vector<TypedFuncParam> params_;
        std::optional<TypePtr> return_type_;
        std::unique_ptr<TypedBlockStmt> body_;
    };

    /**
     * @brief Typed return statement.
     *
     * Represents a return statement with the type of the returned value.
     * Example: `return expr;` where the type matches the function's return type.
     */
    class TypedReturnStmt final : public TypedStmt {
    public:
        TypedReturnStmt(TypedExprPtr value, TypePtr node_type, SourceSpan loc = {})
          : TypedStmt{NodeKind::ReturnStmt, std::move(node_type), loc}, value_{std::move(value)} {}

        [[nodiscard]] bool has_value() const noexcept { return value_ != nullptr; }
        [[nodiscard]] const TypedExpr &value() const noexcept { return *value_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::ReturnStmt; }

    private:
        TypedExprPtr value_;
    };

    /**
     * @brief Typed if statement.
     *
     * Represents a conditional statement. Type is typically void.
     * Example: `if (cond) { ... } else { ... }`
     */
    class TypedIfStmt final : public TypedStmt {
    public:
        TypedIfStmt(TypedExprPtr condition, TypedStmtPtr then_branch, TypedStmtPtr else_branch, TypePtr node_type, SourceSpan loc = {})
          : TypedStmt{NodeKind::IfStmt, std::move(node_type), loc}, condition_{std::move(condition)}, then_branch_{std::move(then_branch)},
            else_branch_{std::move(else_branch)} {}

        [[nodiscard]] const TypedExpr &condition() const noexcept { return *condition_; }
        [[nodiscard]] const TypedStmt &then_branch() const noexcept { return *then_branch_; }
        [[nodiscard]] bool has_else() const noexcept { return else_branch_ != nullptr; }
        [[nodiscard]] const TypedStmt &else_branch() const noexcept { return *else_branch_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::IfStmt; }

    private:
        TypedExprPtr condition_;
        TypedStmtPtr then_branch_;
        TypedStmtPtr else_branch_;
    };

    /**
     * @brief Typed while statement.
     *
     * Represents a while loop. Type is typically void.
     * Example: `while (cond) { ... }`
     */
    class TypedWhileStmt final : public TypedStmt {
    public:
        TypedWhileStmt(TypedExprPtr condition, TypedStmtPtr body, TypePtr node_type, SourceSpan loc = {})
          : TypedStmt{NodeKind::WhileStmt, std::move(node_type), loc}, condition_{std::move(condition)}, body_{std::move(body)} {}

        [[nodiscard]] const TypedExpr &condition() const noexcept { return *condition_; }
        [[nodiscard]] const TypedStmt &body() const noexcept { return *body_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::WhileStmt; }

    private:
        TypedExprPtr condition_;
        TypedStmtPtr body_;
    };

    /**
     * @brief Typed for statement.
     *
     * Represents a C-style for loop. Type is typically void.
     * Example: `for (init; cond; incr) { ... }`
     */
    class TypedForStmt final : public TypedStmt {
    public:
        TypedForStmt(TypedStmtPtr init, TypedExprPtr condition, TypedExprPtr increment, TypedStmtPtr body, TypePtr node_type,
                     SourceSpan loc = {})
          : TypedStmt{NodeKind::ForStmt, std::move(node_type), loc}, init_{std::move(init)}, condition_{std::move(condition)},
            increment_{std::move(increment)}, body_{std::move(body)} {}

        [[nodiscard]] bool has_init() const noexcept { return init_ != nullptr; }
        [[nodiscard]] bool has_condition() const noexcept { return condition_ != nullptr; }
        [[nodiscard]] bool has_increment() const noexcept { return increment_ != nullptr; }

        [[nodiscard]] const TypedStmt &init() const noexcept { return *init_; }
        [[nodiscard]] const TypedExpr &condition() const noexcept { return *condition_; }
        [[nodiscard]] const TypedExpr &increment() const noexcept { return *increment_; }
        [[nodiscard]] const TypedStmt &body() const noexcept { return *body_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::ForStmt; }

    private:
        TypedStmtPtr init_;
        TypedExprPtr condition_;
        TypedExprPtr increment_;
        TypedStmtPtr body_;
    };

    /**
     * @brief Typed break statement.
     *
     * Represents a loop break statement. Type is void.
     */
    class TypedBreakStmt final : public TypedStmt {
    public:
        TypedBreakStmt(TypePtr node_type, SourceSpan loc = {}) : TypedStmt{NodeKind::BreakStmt, std::move(node_type), loc} {}

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::BreakStmt; }
    };

    /**
     * @brief Typed continue statement.
     *
     * Represents a loop continue statement. Type is void.
     */
    class TypedContinueStmt final : public TypedStmt {
    public:
        TypedContinueStmt(TypePtr node_type, SourceSpan loc = {}) : TypedStmt{NodeKind::ContinueStmt, std::move(node_type), loc} {}

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::ContinueStmt; }
    };

    /**
     * @brief Typed main statement.
     *
     * Represents the program entry point. Type is typically void.
     */
    class TypedMainStmt final : public TypedStmt {
    public:
        TypedMainStmt(TypedStmtPtr body, TypePtr node_type, SourceSpan loc = {})
          : TypedStmt{NodeKind::MainStmt, std::move(node_type), loc}, body_{std::move(body)} {}

        [[nodiscard]] bool has_body() const noexcept { return body_ != nullptr; }
        [[nodiscard]] const TypedStmt &body() const noexcept { return *body_; }
        [[nodiscard]] TypedStmt &body() noexcept { return *body_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::MainStmt; }

    private:
        TypedStmtPtr body_;
    };

}  // namespace jsv

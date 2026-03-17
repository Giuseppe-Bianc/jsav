/*
 * Created by gbian on 17/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

// clang-format off
#include "jsav/ast/Node.hpp"
#include "jsav/ast/Expressions.hpp"
// clang-format on

namespace jsv {

    // ============================================================
    // Expression Statement:  expr;
    // ============================================================
    class ExprStmt final : public Stmt {
    public:
        explicit ExprStmt(ExprPtr expression, SourceSpan loc = {}) : Stmt(NodeKind::ExprStmt, loc), expression_{std::move(expression)} {}

        [[nodiscard]] const Expr &expression() const noexcept { return *expression_; }
        [[nodiscard]] Expr &expression() noexcept { return *expression_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::ExprStmt; }

    private:
        ExprPtr expression_;
    };

    // ============================================================
    // Variable Declaration:  var x: int = 42;
    // ============================================================
    class VarDecl final : public Stmt {
    public:
        VarDecl(std::string name, std::optional<std::string> type_annotation, ExprPtr initializer, bool is_const = false,
                SourceSpan loc = {})
          : Stmt(NodeKind::VarDecl, loc), name_{std::move(name)}, type_annotation_{std::move(type_annotation)},
            initializer_{std::move(initializer)}, is_const_{is_const} {}

        [[nodiscard]] const std::string &name() const noexcept { return name_; }
        [[nodiscard]] const std::optional<std::string> &type_annotation() const noexcept { return type_annotation_; }
        [[nodiscard]] bool has_initializer() const noexcept { return initializer_ != nullptr; }
        [[nodiscard]] const Expr &initializer() const noexcept { return *initializer_; }
        [[nodiscard]] const ExprPtr &initializer_ptr() const noexcept { return initializer_; }
        [[nodiscard]] bool is_const() const noexcept { return is_const_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::VarDecl; }

    private:
        std::string name_;
        std::optional<std::string> type_annotation_;
        ExprPtr initializer_;
        bool is_const_;
    };

    // ============================================================
    // Block Statement:  { stmt1; stmt2; ... }
    // ============================================================
    class BlockStmt final : public Stmt {
    public:
        explicit BlockStmt(std::vector<StmtPtr> statements, SourceSpan loc = {})
          : Stmt(NodeKind::BlockStmt, loc), statements_{std::move(statements)} {}

        [[nodiscard]] const std::vector<StmtPtr> &statements() const noexcept { return statements_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::BlockStmt; }

    private:
        std::vector<StmtPtr> statements_;
    };

    // ============================================================
    // Function Declaration:  fn foo(a: int, b: int) -> int { ... }
    // ============================================================
    struct FuncParam {
        std::string name;
        std::string type;
    };

    class FuncDecl final : public Stmt {
    public:
        FuncDecl(std::string name, std::vector<FuncParam> params, std::optional<std::string> return_type, std::unique_ptr<BlockStmt> body,
                 SourceSpan loc = {})
          : Stmt(NodeKind::FuncDecl, loc), name_{std::move(name)}, params_{std::move(params)}, return_type_{std::move(return_type)},
            body_{std::move(body)} {}

        [[nodiscard]] const std::string &name() const noexcept { return name_; }
        [[nodiscard]] const std::vector<FuncParam> &params() const noexcept { return params_; }
        [[nodiscard]] const std::optional<std::string> &return_type() const noexcept { return return_type_; }
        [[nodiscard]] const BlockStmt &body() const noexcept { return *body_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::FuncDecl; }

    private:
        std::string name_;
        std::vector<FuncParam> params_;
        std::optional<std::string> return_type_;
        std::unique_ptr<BlockStmt> body_;
    };

    // ============================================================
    // Return Statement:  return expr;
    // ============================================================
    class ReturnStmt final : public Stmt {
    public:
        explicit ReturnStmt(ExprPtr value = nullptr, SourceSpan loc = {}) : Stmt(NodeKind::ReturnStmt, loc), value_{std::move(value)} {}

        [[nodiscard]] bool has_value() const noexcept { return value_ != nullptr; }
        [[nodiscard]] const Expr &value() const noexcept { return *value_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::ReturnStmt; }

    private:
        ExprPtr value_;
    };

    // ============================================================
    // If Statement:  if (cond) then_body else else_body
    // ============================================================
    class IfStmt final : public Stmt {
    public:
        IfStmt(ExprPtr condition, StmtPtr then_branch, StmtPtr else_branch = nullptr, SourceSpan loc = {})
          : Stmt(NodeKind::IfStmt, loc), condition_{std::move(condition)}, then_branch_{std::move(then_branch)},
            else_branch_{std::move(else_branch)} {}

        [[nodiscard]] const Expr &condition() const noexcept { return *condition_; }
        [[nodiscard]] const Stmt &then_branch() const noexcept { return *then_branch_; }
        [[nodiscard]] bool has_else() const noexcept { return else_branch_ != nullptr; }
        [[nodiscard]] const Stmt &else_branch() const noexcept { return *else_branch_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::IfStmt; }

    private:
        ExprPtr condition_;
        StmtPtr then_branch_;
        StmtPtr else_branch_;
    };

    // ============================================================
    // While Statement:  while (cond) body
    // ============================================================
    class WhileStmt final : public Stmt {
    public:
        WhileStmt(ExprPtr condition, StmtPtr body, SourceSpan loc = {})
          : Stmt(NodeKind::WhileStmt, loc), condition_{std::move(condition)}, body_{std::move(body)} {}

        [[nodiscard]] const Expr &condition() const noexcept { return *condition_; }
        [[nodiscard]] const Stmt &body() const noexcept { return *body_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::WhileStmt; }

    private:
        ExprPtr condition_;
        StmtPtr body_;
    };

    // ============================================================
    // For Statement:  for (init; cond; incr) body
    // ============================================================
    class ForStmt final : public Stmt {
    public:
        ForStmt(StmtPtr init, ExprPtr condition, ExprPtr increment, StmtPtr body, SourceSpan loc = {})
          : Stmt(NodeKind::ForStmt, loc), init_{std::move(init)}, condition_{std::move(condition)}, increment_{std::move(increment)},
            body_{std::move(body)} {}

        [[nodiscard]] bool has_init() const noexcept { return init_ != nullptr; }
        [[nodiscard]] bool has_condition() const noexcept { return condition_ != nullptr; }
        [[nodiscard]] bool has_increment() const noexcept { return increment_ != nullptr; }

        [[nodiscard]] const Stmt &init() const noexcept { return *init_; }
        [[nodiscard]] const Expr &condition() const noexcept { return *condition_; }
        [[nodiscard]] const Expr &increment() const noexcept { return *increment_; }
        [[nodiscard]] const Stmt &body() const noexcept { return *body_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::ForStmt; }

    private:
        StmtPtr init_;
        ExprPtr condition_;
        ExprPtr increment_;
        StmtPtr body_;
    };

    // ============================================================
    // Break / Continue
    // ============================================================
    class BreakStmt final : public Stmt {
    public:
        explicit BreakStmt(SourceSpan loc = {}) : Stmt(NodeKind::BreakStmt, loc) {}

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::BreakStmt; }
    };

    class ContinueStmt final : public Stmt {
    public:
        explicit ContinueStmt(SourceSpan loc = {}) : Stmt(NodeKind::ContinueStmt, loc) {}

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::ContinueStmt; }
    };

    // ============================================================
    // Print Statement:  print expr;  (built-in per demo)
    // ============================================================
    class PrintStmt final : public Stmt {
    public:
        explicit PrintStmt(ExprPtr expression, SourceSpan loc = {}) : Stmt(NodeKind::PrintStmt, loc), expression_{std::move(expression)} {}

        [[nodiscard]] const Expr &expression() const noexcept { return *expression_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::PrintStmt; }

    private:
        ExprPtr expression_;
    };

}  // namespace jsv

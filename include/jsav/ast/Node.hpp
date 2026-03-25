/*
 * Created by gbian on 17/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once
// clang-format off
#include "../headers.hpp"
#include "NodeKind.hpp"
#include "../location/SourceSpan.hpp"
// clang-format on

namespace jsv {

    // Forward declarations
    class Expr;
    class Stmt;

    class Node {
    public:
        constexpr explicit Node(NodeKind kind, const SourceSpan &loc = {}) : kind_{kind}, loc_{loc} {}
        virtual ~Node() = default;

        Node(const Node &) = delete;
        Node &operator=(const Node &) = delete;
        Node(Node &&) noexcept = default;             // PERF: noexcept enables vector move optimization
        Node &operator=(Node &&) noexcept = default;  // PERF: noexcept enables vector move optimization

        [[nodiscard]] constexpr NodeKind kind() const noexcept { return kind_; }
        [[nodiscard]] constexpr SourceSpan location() const noexcept { return loc_; }
        void set_location(const SourceSpan &loc) noexcept { loc_ = loc; }

        [[nodiscard]] std::string_view kind_name() const noexcept;

        [[nodiscard]] static constexpr bool classof(const Node * /*n*/) noexcept { return true; }

    private:
        NodeKind kind_;
        SourceSpan loc_;
    };

    // ============================================================
    // Unique pointer alias
    // ============================================================
    using NodePtr = std::unique_ptr<Node>;
    using ExprPtr = std::unique_ptr<Expr>;
    using StmtPtr = std::unique_ptr<Stmt>;

    // ============================================================
    // Classi intermedie: Expr e Stmt
    // Non definiscono classof: la logica di categoria è
    // gestita dagli overload di node_isa_check qui sotto.
    // ============================================================
    class Expr : public Node {
    public:
        using Node::Node;
    };

    class Stmt : public Node {
    public:
        using Node::Node;
    };

    // ============================================================
    // node_isa_check — overload liberi per il dispatch di classof.
    //
    // Overload concreti (Expr, Stmt): implementano il controllo
    // di categoria direttamente, senza definire classof nelle
    // classi intermedie.
    //
    // Overload generico: delega a T::classof per le classi
    // concrete (IntegerLiteral, BinaryExpr, ecc.).
    // ============================================================
    [[nodiscard]] constexpr bool node_isa_check(const Node *node, std::type_identity<Expr>) noexcept {
        switch(node->kind()) {
        case NodeKind::IntegerLiteral:
        case NodeKind::FloatLiteral:
        case NodeKind::StringLiteral:
        case NodeKind::BoolLiteral:
        case NodeKind::NullLiteral:
        case NodeKind::Identifier:
        case NodeKind::UnaryExpr:
        case NodeKind::BinaryExpr:
        case NodeKind::TernaryExpr:
        case NodeKind::CallExpr:
        case NodeKind::IndexExpr:
        case NodeKind::MemberExpr:
        case NodeKind::AssignExpr:
        case NodeKind::CastExpr:
        case NodeKind::ArrayLiteral:
        case NodeKind::GroupingExpr:
            return true;
        default:
            return false;
        }
    }

    [[nodiscard]] constexpr bool node_isa_check(const Node *node, std::type_identity<Stmt>) noexcept {
        switch(node->kind()) {
        case NodeKind::ExprStmt:
        case NodeKind::VarDecl:
        case NodeKind::FuncDecl:
        case NodeKind::ReturnStmt:
        case NodeKind::IfStmt:
        case NodeKind::WhileStmt:
        case NodeKind::ForStmt:
        case NodeKind::BlockStmt:
        case NodeKind::BreakStmt:
        case NodeKind::ContinueStmt:
        case NodeKind::MainStmt:
            return true;
        default:
            return false;
        }
    }

    template <typename T> [[nodiscard]] constexpr bool node_isa_check(const Node *node, std::type_identity<T>) noexcept {
        return T::classof(node);
    }

    // ============================================================
    // Safe casting utilities (LLVM-style)
    // ============================================================
    template <typename To, typename From>
        requires std::is_base_of_v<Node, To> && std::is_base_of_v<Node, From>
    [[nodiscard]] inline To *node_cast(From *node) {
        assert(node && node_isa_check(node, std::type_identity<To>{}) && "Invalid node_cast");
        return static_cast<To *>(node);
    }

    template <typename To, typename From>
        requires std::is_base_of_v<Node, To> && std::is_base_of_v<Node, From>
    [[nodiscard]] inline const To *node_cast(const From *node) {
        assert(node && node_isa_check(node, std::type_identity<To>{}) && "Invalid node_cast");
        return static_cast<const To *>(node);
    }

    template <typename To, typename From>
        requires std::is_base_of_v<Node, To> && std::is_base_of_v<Node, From>
    [[nodiscard]] inline To *node_dyn_cast(From *node) {
        if(node && node_isa_check(node, std::type_identity<To>{})) return static_cast<To *>(node);
        return nullptr;
    }

    template <typename To, typename From>
        requires std::is_base_of_v<Node, To> && std::is_base_of_v<Node, From>
    [[nodiscard]] inline const To *node_dyn_cast(const From *node) {
        if(node && node_isa_check(node, std::type_identity<To>{})) return static_cast<const To *>(node);
        return nullptr;
    }

    template <typename To, typename From>
        requires std::is_base_of_v<Node, To> && std::is_base_of_v<Node, From>
    [[nodiscard]] inline bool node_isa(const From *node) {
        return node && node_isa_check(node, std::type_identity<To>{});
    }

}  // namespace jsv
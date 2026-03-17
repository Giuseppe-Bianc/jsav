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
        constexpr explicit Node(NodeKind kind, SourceSpan loc = {}) : kind_{kind}, loc_{loc} {}
        // Non virtual destructor: i nodi vengono distrutti tramite
        // unique_ptr<DerivedConcreto>. Il distruttore è protetto
        // nelle classi intermedie per evitare delete attraverso base.
        // Per semplicità lo rendiamo virtual solo qui.
        virtual ~Node() = default;

        Node(const Node &) = delete;
        Node &operator=(const Node &) = delete;
        Node(Node &&) = default;
        Node &operator=(Node &&) = default;

        [[nodiscard]] constexpr NodeKind kind() const noexcept { return kind_; }
        [[nodiscard]] constexpr SourceSpan location() const noexcept { return loc_; }
        void set_location(SourceSpan loc) noexcept { loc_ = loc; }

        [[nodiscard]] constexpr std::string_view kind_name() const noexcept { return node_kind_name(kind_); }

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
    // Servono come "categorie" per il tag dispatching
    // ============================================================
    class Expr : public Node {
    public:
        using Node::Node;

        [[nodiscard]] static constexpr bool classof(const Node *node) {
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
                return true;
            default:
                return false;
            }
        }
    };

    class Stmt : public Node {
    public:
        using Node::Node;

        [[nodiscard]] static constexpr bool classof(const Node *node) {
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
            case NodeKind::PrintStmt:
                return true;
            default:
                return false;
            }
        }
    };

    // ============================================================
    // Safe casting utilities (LLVM-style)
    // ============================================================
    template <typename To, typename From>
        requires std::is_base_of_v<Node, To> && std::is_base_of_v<Node, From>
    [[nodiscard]] inline To *node_cast(From *node) {
        assert(node && To::classof(node) && "Invalid node_cast");
        return static_cast<To *>(node);
    }

    template <typename To, typename From>
        requires std::is_base_of_v<Node, To> && std::is_base_of_v<Node, From>
    [[nodiscard]] inline const To *node_cast(const From *node) {
        assert(node && To::classof(node) && "Invalid node_cast");
        return static_cast<const To *>(node);
    }

    template <typename To, typename From>
        requires std::is_base_of_v<Node, To> && std::is_base_of_v<Node, From>
    [[nodiscard]] inline To *node_dyn_cast(From *node) {
        if(node && To::classof(node)) return static_cast<To *>(node);
        return nullptr;
    }

    template <typename To, typename From>
        requires std::is_base_of_v<Node, To> && std::is_base_of_v<Node, From>
    [[nodiscard]] inline const To *node_dyn_cast(const From *node) {
        if(node && To::classof(node)) return static_cast<const To *>(node);
        return nullptr;
    }

    template <typename To, typename From>
        requires std::is_base_of_v<Node, To> && std::is_base_of_v<Node, From>
    [[nodiscard]] inline bool node_isa(const From *node) {
        return node && To::classof(node);
    }
}  // namespace jsv

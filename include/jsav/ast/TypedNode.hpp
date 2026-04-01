/*
 * Created by gbian on 1 aprile 2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

// clang-format off
#include "jsav/ast/Node.hpp"
#include "jsav/ast/Type.hpp"
// clang-format on

namespace jsv {

    // ============================================================
    // Typed AST Base Classes
    // ============================================================

    /**
     * @brief Base class for all typed AST nodes.
     *
     * Extends the untyped Node with type information inferred during
     * type checking. Every typed node carries its resolved type.
     */
    class TypedNode : public Node {
    public:
        /**
         * @brief Construct a typed node.
         * @param kind The node kind discriminator.
         * @param node_type The resolved type for this node.
         * @param loc Source location span.
         */
        TypedNode(NodeKind kind, TypePtr node_type, const SourceSpan &loc = {})
          : Node{kind, loc}, node_type_{std::move(node_type)} {}

        /**
         * @brief Get the resolved type of this node.
         * @return Shared pointer to the type, or nullptr if not yet typed.
         */
        [[nodiscard]] const TypePtr &node_type() const noexcept { return node_type_; }

        /**
         * @brief Check if this node has been typed.
         * @return true if node_type() returns a valid type.
         */
        [[nodiscard]] bool is_typed() const noexcept { return node_type_ != nullptr; }

        /**
         * @brief Set the type of this node (used during type checking).
         * @param type The resolved type to assign.
         * @pre Type should not already be set (one-time assignment).
         */
        void set_type(TypePtr type) noexcept { node_type_ = std::move(type); }

        /**
         * @brief Virtual destructor for proper cleanup.
         */
        ~TypedNode() override = default;

    protected:
        TypePtr node_type_;  ///< Resolved type information
    };

    /**
     * @brief Base class for typed expressions.
     *
     * All typed expression nodes inherit from this class.
     * Provides type information for expression evaluation.
     */
    class TypedExpr : public TypedNode {
    public:
        TypedExpr(NodeKind kind, TypePtr node_type, const SourceSpan &loc = {})
          : TypedNode{kind, std::move(node_type), loc} {}

        /**
         * @brief Type check for TypedExpr.
         * @param n Node to check.
         * @return true if n is a TypedExpr (any typed expression kind).
         */
        [[nodiscard]] static constexpr bool classof(const Node *n) {
            if(!n) return false;
            switch(n->kind()) {
            case NodeKind::IntegerLiteral:
            case NodeKind::FloatLiteral:
            case NodeKind::StringLiteral:
            case NodeKind::CharLiteral:
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
    };

    /**
     * @brief Base class for typed statements.
     *
     * All typed statement nodes inherit from this class.
     * Statements typically don't carry types (void), but the
     * infrastructure is available for future extensions.
     */
    class TypedStmt : public TypedNode {
    public:
        TypedStmt(NodeKind kind, TypePtr node_type, const SourceSpan &loc = {})
          : TypedNode{kind, std::move(node_type), loc} {}

        /**
         * @brief Type check for TypedStmt.
         * @param n Node to check.
         * @return true if n is a TypedStmt (any typed statement kind).
         */
        [[nodiscard]] static constexpr bool classof(const Node *n) {
            if(!n) return false;
            switch(n->kind()) {
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
    };

    // ============================================================
    // Smart Pointer Aliases for Typed AST
    // ============================================================
    using TypedNodePtr = std::unique_ptr<TypedNode>;
    using TypedExprPtr = std::unique_ptr<TypedExpr>;
    using TypedStmtPtr = std::unique_ptr<TypedStmt>;

}  // namespace jsv

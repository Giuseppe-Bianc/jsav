/*
 * Created by gbian on 17/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

// clang-format off
#include "jsav/ast/Node.hpp"
#include "jsav/ast/Expressions.hpp"
#include "jsav/ast/Statements.hpp"
#include "jsav/ast/Program.hpp"
// clang-format on

namespace jsv {
    // ============================================================
    // Visitor pattern basato su Enum Tag dispatch.
    // Il dispatch avviene tramite uno switch sul NodeKind tag,
    // poi static_cast al tipo concreto — NESSUN virtual dispatch.
    //
    // R = tipo di ritorno (default void)
    // ============================================================
    template <typename Derived, typename R = void> class Visitor {
    public:
        [[nodiscard]] R visit(const Node &node) { return dispatch(node); }

        [[nodiscard]] R visit(const Node *node) {
            if(!node) throw std::runtime_error("Visiting null node");
            return dispatch(*node);
        }

    protected:
        // No default visit_* methods.
        // Derived MUST implement every visit_* it needs.
        // Missing ones → compile-time error (better than runtime throw).
        ~Visitor() = default;

    private:
        Derived &self() { return static_cast<Derived &>(*this); }

        R dispatch(const Node &node) {
            switch(node.kind()) {
            // --- Expressions ---
            case NodeKind::IntegerLiteral:
                return self().visit_IntegerLiteral(static_cast<const IntegerLiteral &>(node));
            case NodeKind::FloatLiteral:
                return self().visit_FloatLiteral(static_cast<const FloatLiteral &>(node));
            case NodeKind::StringLiteral:
                return self().visit_StringLiteral(static_cast<const StringLiteral &>(node));
            case NodeKind::BoolLiteral:
                return self().visit_BoolLiteral(static_cast<const BoolLiteral &>(node));
            case NodeKind::NullLiteral:
                return self().visit_NullLiteral(static_cast<const NullLiteral &>(node));
            case NodeKind::Identifier:
                return self().visit_Identifier(static_cast<const Identifier &>(node));
            case NodeKind::UnaryExpr:
                return self().visit_UnaryExpr(static_cast<const UnaryExpr &>(node));
            case NodeKind::BinaryExpr:
                return self().visit_BinaryExpr(static_cast<const BinaryExpr &>(node));
            case NodeKind::TernaryExpr:
                return self().visit_TernaryExpr(static_cast<const TernaryExpr &>(node));
            case NodeKind::CallExpr:
                return self().visit_CallExpr(static_cast<const CallExpr &>(node));
            case NodeKind::IndexExpr:
                return self().visit_IndexExpr(static_cast<const IndexExpr &>(node));
            case NodeKind::MemberExpr:
                return self().visit_MemberExpr(static_cast<const MemberExpr &>(node));
            case NodeKind::AssignExpr:
                return self().visit_AssignExpr(static_cast<const AssignExpr &>(node));
            case NodeKind::CastExpr:
                return self().visit_CastExpr(static_cast<const CastExpr &>(node));
            case NodeKind::ArrayLiteral:
                return self().visit_ArrayLiteral(static_cast<const ArrayLiteral &>(node));
            case NodeKind::GroupingExpr:
                return self().visit_GroupingExpr(static_cast<const GroupingExpr &>(node));

            // --- Statements ---
            case NodeKind::ExprStmt:
                return self().visit_ExprStmt(static_cast<const ExprStmt &>(node));
            case NodeKind::VarDecl:
                return self().visit_VarDecl(static_cast<const VarDecl &>(node));
            case NodeKind::FuncDecl:
                return self().visit_FuncDecl(static_cast<const FuncDecl &>(node));
            case NodeKind::ReturnStmt:
                return self().visit_ReturnStmt(static_cast<const ReturnStmt &>(node));
            case NodeKind::IfStmt:
                return self().visit_IfStmt(static_cast<const IfStmt &>(node));
            case NodeKind::WhileStmt:
                return self().visit_WhileStmt(static_cast<const WhileStmt &>(node));
            case NodeKind::ForStmt:
                return self().visit_ForStmt(static_cast<const ForStmt &>(node));
            case NodeKind::BlockStmt:
                return self().visit_BlockStmt(static_cast<const BlockStmt &>(node));
            case NodeKind::BreakStmt:
                return self().visit_BreakStmt(static_cast<const BreakStmt &>(node));
            case NodeKind::ContinueStmt:
                return self().visit_ContinueStmt(static_cast<const ContinueStmt &>(node));
            case NodeKind::PrintStmt:
                return self().visit_PrintStmt(static_cast<const PrintStmt &>(node));

            // --- Program ---
            case NodeKind::Program:
                return self().visit_Program(static_cast<const Program &>(node));
            }

            throw std::runtime_error(FORMAT("Unknown NodeKind: {}", C_I(node.kind())));
        }
    };

}  // namespace jsv

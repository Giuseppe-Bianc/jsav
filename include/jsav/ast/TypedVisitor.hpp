/*
 * Created by gbian on 1 aprile 2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

// clang-format off
#include "jsav/ast/TypedNode.hpp"
#include "jsav/ast/TypedExpressions.hpp"
#include "jsav/ast/TypedStatements.hpp"
#include "jsav/ast/TypedProgram.hpp"
// clang-format on

namespace jsv {

    // ============================================================
    // Typed Visitor Pattern
    // ============================================================

    /**
     * @brief Visitor pattern for typed AST traversal.
     *
     * Provides type-safe visitation of typed AST nodes. The dispatch
     * occurs via enum tag switch (same as untyped Visitor), but all
     * visit_* methods receive typed node references.
     *
     * @tparam Derived The CRTP derived visitor class.
     * @tparam R Return type for visit methods (default: void).
     */
    template <typename Derived, typename R = void> class TypedVisitor {
    public:
        /**
         * @brief Visit a typed node.
         * @param node The typed node to visit.
         * @return Result of the visit operation.
         * @throws std::runtime_error if node is null.
         */
        [[nodiscard]] R visit(const TypedNode &node) { return dispatch(node); }

        /**
         * @brief Visit a typed node pointer.
         * @param node The typed node to visit.
         * @return Result of the visit operation.
         * @throws std::runtime_error if node is null.
         */
        [[nodiscard]] R visit(const TypedNode *node) {
            if(!node) throw std::runtime_error("Visiting null typed node");
            return dispatch(*node);
        }

    protected:
        ~TypedVisitor() = default;

    private:
        Derived &self() { return static_cast<Derived &>(*this); }

        R dispatch(const TypedNode &node) {
            switch(node.kind()) {
            // --- Typed Expressions ---
            case NodeKind::IntegerLiteral:
                [[likely]] return self().visit_IntegerLiteral(static_cast<const TypedIntegerLiteral &>(node));
            case NodeKind::FloatLiteral:
                return self().visit_FloatLiteral(static_cast<const TypedFloatLiteral &>(node));
            case NodeKind::StringLiteral:
                [[likely]] return self().visit_StringLiteral(static_cast<const TypedStringLiteral &>(node));
            case NodeKind::CharLiteral:
                return self().visit_CharLiteral(static_cast<const TypedCharLiteral &>(node));
            case NodeKind::BoolLiteral:
                return self().visit_BoolLiteral(static_cast<const TypedBoolLiteral &>(node));
            case NodeKind::NullLiteral:
                [[unlikely]] return self().visit_NullLiteral(static_cast<const TypedNullLiteral &>(node));
            case NodeKind::Identifier:
                [[likely]] return self().visit_Identifier(static_cast<const TypedIdentifier &>(node));
            case NodeKind::UnaryExpr:
                return self().visit_UnaryExpr(static_cast<const TypedUnaryExpr &>(node));
            case NodeKind::BinaryExpr:
                [[likely]] return self().visit_BinaryExpr(static_cast<const TypedBinaryExpr &>(node));
            case NodeKind::TernaryExpr:
                return self().visit_TernaryExpr(static_cast<const TypedTernaryExpr &>(node));
            case NodeKind::CallExpr:
                [[likely]] return self().visit_CallExpr(static_cast<const TypedCallExpr &>(node));
            case NodeKind::IndexExpr:
                return self().visit_IndexExpr(static_cast<const TypedIndexExpr &>(node));
            case NodeKind::MemberExpr:
                [[likely]] return self().visit_MemberExpr(static_cast<const TypedMemberExpr &>(node));
            case NodeKind::AssignExpr:
                [[likely]] return self().visit_AssignExpr(static_cast<const TypedAssignExpr &>(node));
            case NodeKind::CastExpr:
                return self().visit_CastExpr(static_cast<const TypedCastExpr &>(node));
            case NodeKind::ArrayLiteral:
                return self().visit_ArrayLiteral(static_cast<const TypedArrayLiteral &>(node));
            case NodeKind::GroupingExpr:
                return self().visit_GroupingExpr(static_cast<const TypedGroupingExpr &>(node));

            // --- Typed Statements ---
            case NodeKind::ExprStmt:
                [[likely]] return self().visit_ExprStmt(static_cast<const TypedExprStmt &>(node));
            case NodeKind::VarDecl:
                [[likely]] return self().visit_VarDecl(static_cast<const TypedVarDecl &>(node));
            case NodeKind::FuncDecl:
                [[likely]] return self().visit_FuncDecl(static_cast<const TypedFuncDecl &>(node));
            case NodeKind::ReturnStmt:
                [[likely]] return self().visit_ReturnStmt(static_cast<const TypedReturnStmt &>(node));
            case NodeKind::IfStmt:
                [[likely]] return self().visit_IfStmt(static_cast<const TypedIfStmt &>(node));
            case NodeKind::WhileStmt:
                [[likely]] return self().visit_WhileStmt(static_cast<const TypedWhileStmt &>(node));
            case NodeKind::ForStmt:
                [[likely]] return self().visit_ForStmt(static_cast<const TypedForStmt &>(node));
            case NodeKind::BlockStmt:
                [[likely]] return self().visit_BlockStmt(static_cast<const TypedBlockStmt &>(node));
            case NodeKind::BreakStmt:
                [[unlikely]] return self().visit_BreakStmt(static_cast<const TypedBreakStmt &>(node));
            case NodeKind::ContinueStmt:
                [[unlikely]] return self().visit_ContinueStmt(static_cast<const TypedContinueStmt &>(node));
            case NodeKind::MainStmt:
                [[unlikely]] return self().visit_MainStmt(static_cast<const TypedMainStmt &>(node));

            // --- Typed Program ---
            case NodeKind::Program:
                [[likely]] return self().visit_Program(static_cast<const TypedProgram &>(node));
            }

            throw std::runtime_error(FORMAT("Unknown Typed NodeKind: {}", C_I(node.kind())));
        }
    };

}  // namespace jsv

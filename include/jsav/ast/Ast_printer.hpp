/*
 * Created by gbian on 17/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once
#include "jsav/ast/Visitor.hpp"

namespace jsv {

    // ============================================================
    // AST Pretty Printer — usa il Visitor CRTP
    // ============================================================
    class AstPrinter : public Visitor<AstPrinter> {
        friend class Visitor<AstPrinter>;  // accesso a visit_*

    public:
        void print(const Node &node);

    private:
        int indent_ = 0;

        void print_indent();
        void println(std::string_view msg);

        struct IndentGuard {
            AstPrinter &p;
            IndentGuard(AstPrinter &p) : p(p) { ++p.indent_; }
            ~IndentGuard() { --p.indent_; }
        };

        // ========== Expressions ==========

        void visit_IntegerLiteral(const IntegerLiteral &node);
        void visit_FloatLiteral(const FloatLiteral &node);
        void visit_StringLiteral(const StringLiteral &node);
        void visit_BoolLiteral(const BoolLiteral &node);
        void visit_NullLiteral(const NullLiteral &node);
        void visit_Identifier(const Identifier &node);
        void visit_UnaryExpr(const UnaryExpr &node);
        void visit_BinaryExpr(const BinaryExpr &node);
        void visit_TernaryExpr(const TernaryExpr &node);
        void visit_CallExpr(const CallExpr &node);
        void visit_IndexExpr(const IndexExpr &node);
        void visit_MemberExpr(const MemberExpr &node);
        void visit_AssignExpr(const AssignExpr &node);
        void visit_CastExpr(const CastExpr &node);
        void visit_ArrayLiteral(const ArrayLiteral &node);

        // ========== Statements ==========

        void visit_ExprStmt(const ExprStmt &node);
        void visit_VarDecl(const VarDecl &node);
        void visit_FuncDecl(const FuncDecl &node);
        void visit_ReturnStmt(const ReturnStmt &node);
        void visit_IfStmt(const IfStmt &node);
        void visit_WhileStmt(const WhileStmt &node);
        void visit_ForStmt(const ForStmt &node);
        void visit_BlockStmt(const BlockStmt &node);
        void visit_BreakStmt(const BreakStmt &node);
        void visit_ContinueStmt(const ContinueStmt &node);
        void visit_PrintStmt(const PrintStmt &node);

        void visit_Program(const Program &node);
    };

    // ============================================================
    // S-Expression Printer — restituisce una stringa
    // ============================================================
    class SExprPrinter : public Visitor<SExprPrinter, std::string> {
        friend class Visitor<SExprPrinter, std::string>;

    public:
        std::string to_string(const Node &node);

    private:
        // Expressions
        std::string visit_IntegerLiteral(const IntegerLiteral &n);
        std::string visit_FloatLiteral(const FloatLiteral &n);
        std::string visit_StringLiteral(const StringLiteral &n);
        std::string visit_BoolLiteral(const BoolLiteral &n);
        std::string visit_NullLiteral(const NullLiteral &n);
        std::string visit_Identifier(const Identifier &n);
        std::string visit_UnaryExpr(const UnaryExpr &n);
        std::string visit_BinaryExpr(const BinaryExpr &n);
        std::string visit_TernaryExpr(const TernaryExpr &n);
        std::string visit_CallExpr(const CallExpr &n);
        std::string visit_IndexExpr(const IndexExpr &n);
        std::string visit_MemberExpr(const MemberExpr &n);
        std::string visit_AssignExpr(const AssignExpr &n);
        std::string visit_CastExpr(const CastExpr &n);
        std::string visit_ArrayLiteral(const ArrayLiteral &n);

        // Statements
        std::string visit_ExprStmt(const ExprStmt &n);
        std::string visit_VarDecl(const VarDecl &n);
        std::string visit_FuncDecl(const FuncDecl &n);
        std::string visit_ReturnStmt(const ReturnStmt &n);
        std::string visit_IfStmt(const IfStmt &n);
        std::string visit_WhileStmt(const WhileStmt &n);
        std::string visit_ForStmt(const ForStmt &n);
        std::string visit_BlockStmt(const BlockStmt &n);
        std::string visit_BreakStmt(const BreakStmt &n);
        std::string visit_ContinueStmt(const ContinueStmt &n);
        std::string visit_PrintStmt(const PrintStmt &n);
        std::string visit_Program(const Program &n);
    };
}  // namespace jsv

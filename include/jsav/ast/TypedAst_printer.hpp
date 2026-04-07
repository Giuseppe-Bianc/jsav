/*
 * Created by gbian on 06/04/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once
#include "jsav/ast/TypedVisitor.hpp"
#include "jsav/error/ErrorReporter.hpp"

namespace jsv {

    // ============================================================
    // Typed AST Pretty Printer — formato ad albero con caratteri Unicode
    // ============================================================
    class TypedAstPrinter : public TypedVisitor<TypedAstPrinter> {
        friend class TypedVisitor<TypedAstPrinter>;

    public:
        /**
         * @brief Default constructor for TypedAstPrinter.
         *
         * Constructs a TypedAstPrinter with default initial state, ready to print typed AST nodes.
         */
        TypedAstPrinter() = default;

        /**
         * @brief Prints the typed AST node in a tree-like Unicode format.
         *
         * @param node The root typed AST node to print. The entire subtree will be traversed
         *             and printed to standard output with proper indentation and type annotations.
         *
         * @code
         * TypedAstPrinter printer;
         * printer.print(myTypedAstNode);
         * @endcode
         */
        void print(const TypedNode &node);

    private:
        std::vector<bool> prefix_stack_;
        bool next_is_last_ = true;

        void print_prefix() const;
        void print_line(std::string_view msg, bool is_last) const;
        void print_value(std::string_view label, std::string_view value, bool is_last) const;
        void print_type_annotation(const TypePtr &type, bool is_last) const;

        void push_indent(bool is_last) { prefix_stack_.push_back(is_last); }
        void pop_indent() { prefix_stack_.pop_back(); }

        void visit_child(const TypedNode &node, bool is_last) {
            next_is_last_ = is_last;
            visit(node);
        }

        struct IndentGuard {
            TypedAstPrinter &p;
            IndentGuard(TypedAstPrinter &printer, bool is_last) : p(printer) { printer.push_indent(is_last); }
            ~IndentGuard() { p.pop_indent(); }
            IndentGuard(const IndentGuard&) = delete;
            IndentGuard& operator=(const IndentGuard&) = delete;
        };

        // ========== Expressions ==========

        // cppcheck-suppress functionConst
        void visit_IntegerLiteral(const TypedIntegerLiteral &node);
        // cppcheck-suppress functionConst
        void visit_FloatLiteral(const TypedFloatLiteral &node);
        // cppcheck-suppress functionConst
        void visit_StringLiteral(const TypedStringLiteral &node);
        // cppcheck-suppress functionConst
        void visit_CharLiteral(const TypedCharLiteral &node);
        // cppcheck-suppress functionConst
        void visit_BoolLiteral(const TypedBoolLiteral &node);
        // cppcheck-suppress functionConst
        void visit_NullLiteral(const TypedNullLiteral &node);
        // cppcheck-suppress functionConst
        void visit_Identifier(const TypedIdentifier &node);
        void visit_UnaryExpr(const TypedUnaryExpr &node);
        void visit_BinaryExpr(const TypedBinaryExpr &node);
        void visit_TernaryExpr(const TypedTernaryExpr &node);
        void visit_CallExpr(const TypedCallExpr &node);
        void visit_IndexExpr(const TypedIndexExpr &node);
        void visit_MemberExpr(const TypedMemberExpr &node);
        void visit_AssignExpr(const TypedAssignExpr &node);
        void visit_CastExpr(const TypedCastExpr &node);
        void visit_ArrayLiteral(const TypedArrayLiteral &node);
        void visit_GroupingExpr(const TypedGroupingExpr &node);

        // ========== Statements ==========

        void visit_ExprStmt(const TypedExprStmt &node);
        void visit_VarDecl(const TypedVarDecl &node);
        void visit_FuncDecl(const TypedFuncDecl &node);
        void visit_ReturnStmt(const TypedReturnStmt &node);
        void visit_IfStmt(const TypedIfStmt &node);
        void visit_WhileStmt(const TypedWhileStmt &node);
        void visit_ForStmt(const TypedForStmt &node);
        void visit_BlockStmt(const TypedBlockStmt &node);
        // cppcheck-suppress functionConst
        void visit_BreakStmt(const TypedBreakStmt &node);
        // cppcheck-suppress functionConst
        void visit_ContinueStmt(const TypedContinueStmt &node);
        void visit_MainStmt(const TypedMainStmt &node);

        void visit_Program(const TypedProgram &node);
    };

}  // namespace jsv

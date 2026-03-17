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
        void print(const Node &node) {
            indent_ = 0;
            visit(node);
        }

    private:
        int indent_ = 0;

        void print_indent() {
            for(int i = 0; i < indent_; ++i) std::cout << "  ";
        }

        void println(std::string_view msg) {
            print_indent();
            std::cout << msg << "\n";
        }

        struct IndentGuard {
            AstPrinter &p;
            IndentGuard(AstPrinter &p) : p(p) { ++p.indent_; }
            ~IndentGuard() { --p.indent_; }
        };

        // ========== Expressions ==========

        void visit_IntegerLiteral(const IntegerLiteral &node) { println(FORMAT("IntegerLiteral: {}", node.value())); }

        void visit_FloatLiteral(const FloatLiteral &node) { println(FORMAT("FloatLiteral: {}", node.value())); }

        void visit_StringLiteral(const StringLiteral &node) { println(FORMAT("StringLiteral: \"{}\"", node.value())); }

        void visit_BoolLiteral(const BoolLiteral &node) { println(FORMAT("BoolLiteral: {}", node.value() ? "true" : "false")); }

        void visit_NullLiteral(const NullLiteral &) { println("NullLiteral"); }

        void visit_Identifier(const Identifier &node) { println(FORMAT("Identifier: {}", node.name())); }

        void visit_UnaryExpr(const UnaryExpr &node) {
            println(FORMAT("UnaryExpr: {}", unary_op_symbol(node.op())));
            IndentGuard g{*this};
            visit(node.operand());
        }

        void visit_BinaryExpr(const BinaryExpr &node) {
            println(FORMAT("BinaryExpr: {}", binary_op_symbol(node.op())));
            IndentGuard g{*this};
            visit(node.lhs());
            visit(node.rhs());
        }

        void visit_TernaryExpr(const TernaryExpr &node) {
            println("TernaryExpr:");
            IndentGuard g{*this};
            println("condition:");
            {
                IndentGuard g2{*this};
                visit(node.condition());
            }
            println("then:");
            {
                IndentGuard g2{*this};
                visit(node.then_expr());
            }
            println("else:");
            {
                IndentGuard g2{*this};
                visit(node.else_expr());
            }
        }

        void visit_CallExpr(const CallExpr &node) {
            println("CallExpr:");
            IndentGuard g{*this};
            println("callee:");
            {
                IndentGuard g2{*this};
                visit(node.callee());
            }
            println(FORMAT("args: ({})", node.args().size()));
            for(const auto &arg : node.args()) {
                IndentGuard g2{*this};
                visit(*arg);
            }
        }

        void visit_IndexExpr(const IndexExpr &node) {
            println("IndexExpr:");
            IndentGuard g{*this};
            println("object:");
            {
                IndentGuard g2{*this};
                visit(node.object());
            }
            println("index:");
            {
                IndentGuard g2{*this};
                visit(node.index());
            }
        }

        void visit_MemberExpr(const MemberExpr &node) {
            println(FORMAT("MemberExpr: .{}", node.member()));
            IndentGuard g{*this};
            visit(node.object());
        }

        void visit_AssignExpr(const AssignExpr &node) {
            println("AssignExpr:");
            IndentGuard g{*this};
            println("target:");
            {
                IndentGuard g2{*this};
                visit(node.target());
            }
            println("value:");
            {
                IndentGuard g2{*this};
                visit(node.value());
            }
        }

        void visit_CastExpr(const CastExpr &node) {
            println(FORMAT("CastExpr: -> {}", node.target_type()));
            IndentGuard g{*this};
            visit(node.operand());
        }

        void visit_ArrayLiteral(const ArrayLiteral &node) {
            println(FORMAT("ArrayLiteral: [{} elements]", node.elements().size()));
            IndentGuard g{*this};
            for(const auto &elem : node.elements()) visit(*elem);
        }

        // ========== Statements ==========

        void visit_ExprStmt(const ExprStmt &node) {
            println("ExprStmt:");
            IndentGuard g{*this};
            visit(node.expression());
        }

        void visit_VarDecl(const VarDecl &node) {
            std::string info = FORMAT("VarDecl: {} {}", node.is_const() ? "const" : "var", node.name());
            if(node.type_annotation()) info += FORMAT(": {}", *node.type_annotation());
            println(info);
            if(node.has_initializer()) {
                IndentGuard g{*this};
                println("initializer:");
                IndentGuard g2{*this};
                visit(node.initializer());
            }
        }

        void visit_FuncDecl(const FuncDecl &node) {
            std::string info = FORMAT("FuncDecl: {}", node.name());
            if(node.return_type()) info += FORMAT(" -> {}", *node.return_type());
            println(info);
            {
                IndentGuard g{*this};
                if(!node.params().empty()) {
                    println("params:");
                    IndentGuard g2{*this};
                    for(const auto &p : node.params()) println(FORMAT("{}: {}", p.name, p.type));
                }
                println("body:");
                IndentGuard g3{*this};
                visit_BlockStmt(node.body());
            }
        }

        void visit_ReturnStmt(const ReturnStmt &node) {
            println("ReturnStmt:");
            if(node.has_value()) {
                IndentGuard g{*this};
                visit(node.value());
            }
        }

        void visit_IfStmt(const IfStmt &node) {
            println("IfStmt:");
            IndentGuard g{*this};
            println("condition:");
            {
                IndentGuard g2{*this};
                visit(node.condition());
            }
            println("then:");
            {
                IndentGuard g2{*this};
                visit(node.then_branch());
            }
            if(node.has_else()) {
                println("else:");
                IndentGuard g2{*this};
                visit(node.else_branch());
            }
        }

        void visit_WhileStmt(const WhileStmt &node) {
            println("WhileStmt:");
            IndentGuard g{*this};
            println("condition:");
            {
                IndentGuard g2{*this};
                visit(node.condition());
            }
            println("body:");
            {
                IndentGuard g2{*this};
                visit(node.body());
            }
        }

        void visit_ForStmt(const ForStmt &node) {
            println("ForStmt:");
            IndentGuard g{*this};
            if(node.has_init()) {
                println("init:");
                IndentGuard g2{*this};
                visit(node.init());
            }
            if(node.has_condition()) {
                println("condition:");
                IndentGuard g2{*this};
                visit(node.condition());
            }
            if(node.has_increment()) {
                println("increment:");
                IndentGuard g2{*this};
                visit(node.increment());
            }
            println("body:");
            {
                IndentGuard g2{*this};
                visit(node.body());
            }
        }

        void visit_BlockStmt(const BlockStmt &node) {
            println("BlockStmt:");
            IndentGuard g{*this};
            for(const auto &stmt : node.statements()) visit(*stmt);
        }

        void visit_BreakStmt(const BreakStmt &) { println("BreakStmt"); }

        void visit_ContinueStmt(const ContinueStmt &) { println("ContinueStmt"); }

        void visit_PrintStmt(const PrintStmt &node) {
            println("PrintStmt:");
            IndentGuard g{*this};
            visit(node.expression());
        }

        void visit_Program(const Program &node) {
            println("Program:");
            IndentGuard g{*this};
            for(const auto &stmt : node.statements()) visit(*stmt);
        }
    };

    // ============================================================
    // S-Expression Printer — restituisce una stringa
    // ============================================================
    class SExprPrinter : public Visitor<SExprPrinter, std::string> {
        friend class Visitor<SExprPrinter, std::string>;

    public:
        std::string to_string(const Node &node) { return visit(node); }

    private:
        // Expressions
        std::string visit_IntegerLiteral(const IntegerLiteral &n) { return std::to_string(n.value()); }

        std::string visit_FloatLiteral(const FloatLiteral &n) { return FORMAT("{}", n.value()); }

        std::string visit_StringLiteral(const StringLiteral &n) { return FORMAT("\"{}\"", n.value()); }

        std::string visit_BoolLiteral(const BoolLiteral &n) { return n.value() ? "true" : "false"; }

        std::string visit_NullLiteral(const NullLiteral &) { return "null"; }

        std::string visit_Identifier(const Identifier &n) { return n.name(); }

        std::string visit_UnaryExpr(const UnaryExpr &n) { return FORMAT("({} {})", unary_op_symbol(n.op()), visit(n.operand())); }

        std::string visit_BinaryExpr(const BinaryExpr &n) {
            return FORMAT("({} {} {})", binary_op_symbol(n.op()), visit(n.lhs()), visit(n.rhs()));
        }

        std::string visit_TernaryExpr(const TernaryExpr &n) {
            return FORMAT("(?: {} {} {})", visit(n.condition()), visit(n.then_expr()), visit(n.else_expr()));
        }

        std::string visit_CallExpr(const CallExpr &n) {
            std::string result = FORMAT("(call {}", visit(n.callee()));
            for(const auto &arg : n.args()) result += " " + visit(*arg);
            result += ")";
            return result;
        }

        std::string visit_IndexExpr(const IndexExpr &n) { return FORMAT("(index {} {})", visit(n.object()), visit(n.index())); }

        std::string visit_MemberExpr(const MemberExpr &n) { return FORMAT("(. {} {})", visit(n.object()), n.member()); }

        std::string visit_AssignExpr(const AssignExpr &n) { return FORMAT("(= {} {})", visit(n.target()), visit(n.value())); }

        std::string visit_CastExpr(const CastExpr &n) { return FORMAT("(cast {} {})", n.target_type(), visit(n.operand())); }

        std::string visit_ArrayLiteral(const ArrayLiteral &n) {
            std::string result = "[";
            for(std::size_t i = 0; i < n.elements().size(); ++i) {
                if(i > 0) result += " ";
                result += visit(*n.elements()[i]);
            }
            result += "]";
            return result;
        }

        // Statements
        std::string visit_ExprStmt(const ExprStmt &n) { return FORMAT("(expr-stmt {})", visit(n.expression())); }

        std::string visit_VarDecl(const VarDecl &n) {
            std::string result = FORMAT("({} {}", n.is_const() ? "const" : "var", n.name());
            if(n.type_annotation()) result += FORMAT(" : {}", *n.type_annotation());
            if(n.has_initializer()) result += " " + visit(n.initializer());
            result += ")";
            return result;
        }

        std::string visit_FuncDecl(const FuncDecl &n) {
            std::string result = FORMAT("(fn {} (", n.name());
            for(std::size_t i = 0; i < n.params().size(); ++i) {
                if(i > 0) result += " ";
                result += FORMAT("({} {})", n.params()[i].name, n.params()[i].type);
            }
            result += ")";
            if(n.return_type()) result += FORMAT(" -> {}", *n.return_type());
            result += " " + visit_BlockStmt(n.body()) + ")";
            return result;
        }

        std::string visit_ReturnStmt(const ReturnStmt &n) {
            if(n.has_value()) return FORMAT("(return {})", visit(n.value()));
            return "(return)";
        }

        std::string visit_IfStmt(const IfStmt &n) {
            std::string result = FORMAT("(if {} {}", visit(n.condition()), visit(n.then_branch()));
            if(n.has_else()) result += " " + visit(n.else_branch());
            result += ")";
            return result;
        }

        std::string visit_WhileStmt(const WhileStmt &n) { return FORMAT("(while {} {})", visit(n.condition()), visit(n.body())); }

        std::string visit_ForStmt(const ForStmt &n) {
            std::string result = "(for ";
            result += n.has_init() ? visit(n.init()) : "_";
            result += " ";
            result += n.has_condition() ? visit(n.condition()) : "_";
            result += " ";
            result += n.has_increment() ? visit(n.increment()) : "_";
            result += " " + visit(n.body()) + ")";
            return result;
        }

        std::string visit_BlockStmt(const BlockStmt &n) {
            std::string result = "(block";
            for(const auto &s : n.statements()) result += " " + visit(*s);
            result += ")";
            return result;
        }

        std::string visit_BreakStmt(const BreakStmt &) { return "(break)"; }

        std::string visit_ContinueStmt(const ContinueStmt &) { return "(continue)"; }

        std::string visit_PrintStmt(const PrintStmt &n) { return FORMAT("(print {})", visit(n.expression())); }

        std::string visit_Program(const Program &n) {
            std::string result = "(program";
            for(const auto &s : n.statements()) result += " " + visit(*s);
            result += ")";
            return result;
        }
    };
}  // namespace jsv

/*
 * Created by gbian on 17/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#include "../../../include/jsav/ast/Ast_printer.hpp"

namespace jsv {

    // ============================================================
    // AstPrinter implementation
    // ============================================================

    void AstPrinter::print(const Node &node) {
        indent_ = 0;
        visit(node);
    }

    void AstPrinter::print_indent() {
        for(int i = 0; i < indent_; ++i) std::cout << "  ";
    }

    void AstPrinter::println(std::string_view msg) {
        print_indent();
        std::cout << msg << "\n";
    }

    // ========== Expressions ==========

    void AstPrinter::visit_IntegerLiteral(const IntegerLiteral &node) { println(FORMAT("IntegerLiteral: {}", node.value())); }

    void AstPrinter::visit_FloatLiteral(const FloatLiteral &node) { println(FORMAT("FloatLiteral: {}", node.value())); }

    void AstPrinter::visit_StringLiteral(const StringLiteral &node) { println(FORMAT("StringLiteral: \"{}\"", node.value())); }

    void AstPrinter::visit_BoolLiteral(const BoolLiteral &node) { println(FORMAT("BoolLiteral: {}", node.value() ? "true" : "false")); }

    void AstPrinter::visit_NullLiteral(const NullLiteral &) { println("NullLiteral"); }

    void AstPrinter::visit_Identifier(const Identifier &node) { println(FORMAT("Identifier: {}", node.name())); }

    void AstPrinter::visit_UnaryExpr(const UnaryExpr &node) {
        println(FORMAT("UnaryExpr: {}", unary_op_symbol(node.op())));
        IndentGuard g{*this};
        visit(node.operand());
    }

    void AstPrinter::visit_BinaryExpr(const BinaryExpr &node) {
        println(FORMAT("BinaryExpr: {}", binary_op_symbol(node.op())));
        IndentGuard g{*this};
        visit(node.lhs());
        visit(node.rhs());
    }

    void AstPrinter::visit_TernaryExpr(const TernaryExpr &node) {
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

    void AstPrinter::visit_CallExpr(const CallExpr &node) {
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

    void AstPrinter::visit_IndexExpr(const IndexExpr &node) {
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

    void AstPrinter::visit_MemberExpr(const MemberExpr &node) {
        println(FORMAT("MemberExpr: .{}", node.member()));
        IndentGuard g{*this};
        visit(node.object());
    }

    void AstPrinter::visit_AssignExpr(const AssignExpr &node) {
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

    void AstPrinter::visit_CastExpr(const CastExpr &node) {
        println(FORMAT("CastExpr: -> {}", node.target_type()));
        IndentGuard g{*this};
        visit(node.operand());
    }

    void AstPrinter::visit_ArrayLiteral(const ArrayLiteral &node) {
        println(FORMAT("ArrayLiteral: [{} elements]", node.elements().size()));
        IndentGuard g{*this};
        for(const auto &elem : node.elements()) visit(*elem);
    }

    // ========== Statements ==========

    void AstPrinter::visit_ExprStmt(const ExprStmt &node) {
        println("ExprStmt:");
        IndentGuard g{*this};
        visit(node.expression());
    }

    void AstPrinter::visit_VarDecl(const VarDecl &node) {
        const std::string keyword = node.is_const() ? "const" : "var";

        // Multi-variable declaration: var a, b: i64 = 10, 20;
        if(node.num_variables() > 1) {
            std::string info = FORMAT("{}: ", keyword);
            for(std::size_t i = 0; i < node.names().size(); ++i) {
                if(i > 0) info += ", ";
                info += node.names()[i];
            }
            if(node.type_annotation()) { info += FORMAT(": {}", *node.type_annotation()); }
            println(info);

            // Print each initializer indented with index
            if(!node.initializers().empty()) {
                IndentGuard g{*this};
                println("initializers:");
                for(std::size_t i = 0; i < node.initializers().size(); ++i) {
                    if(node.initializers()[i]) {
                        println(FORMAT("[{}] ", i));
                        IndentGuard g2{*this};
                        visit(*node.initializers()[i]);
                    }
                }
            }
        } else {
            // Single variable declaration (backward compatible format)
            std::string info = FORMAT("{}: {}", keyword, node.name());
            if(node.type_annotation()) { info += FORMAT(": {}", *node.type_annotation()); }
            if(node.has_initializer()) {
                println(info);
                IndentGuard g{*this};
                println("initializer:");
                IndentGuard g2{*this};
                visit(node.initializer());
            } else {
                println(info);
            }
        }
    }

    void AstPrinter::visit_FuncDecl(const FuncDecl &node) {
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

    void AstPrinter::visit_ReturnStmt(const ReturnStmt &node) {
        println("ReturnStmt:");
        if(node.has_value()) {
            IndentGuard g{*this};
            visit(node.value());
        }
    }

    void AstPrinter::visit_IfStmt(const IfStmt &node) {
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

    void AstPrinter::visit_WhileStmt(const WhileStmt &node) {
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

    void AstPrinter::visit_ForStmt(const ForStmt &node) {
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

    void AstPrinter::visit_BlockStmt(const BlockStmt &node) {
        println("BlockStmt:");
        IndentGuard g{*this};
        for(const auto &stmt : node.statements()) visit(*stmt);
    }

    void AstPrinter::visit_BreakStmt(const BreakStmt &) { println("BreakStmt"); }

    void AstPrinter::visit_ContinueStmt(const ContinueStmt &) { println("ContinueStmt"); }

    void AstPrinter::visit_PrintStmt(const PrintStmt &node) {
        println("PrintStmt:");
        IndentGuard g{*this};
        visit(node.expression());
    }

    void AstPrinter::visit_Program(const Program &node) {
        println("Program:");
        IndentGuard g{*this};
        for(const auto &stmt : node.statements()) visit(*stmt);
    }

    // ============================================================
    // SExprPrinter implementation
    // ============================================================

    std::string SExprPrinter::to_string(const Node &node) { return visit(node); }

    // Expressions
    std::string SExprPrinter::visit_IntegerLiteral(const IntegerLiteral &n) { return std::to_string(n.value()); }

    std::string SExprPrinter::visit_FloatLiteral(const FloatLiteral &n) { return FORMAT("{}", n.value()); }

    std::string SExprPrinter::visit_StringLiteral(const StringLiteral &n) { return FORMAT("\"{}\"", n.value()); }

    std::string SExprPrinter::visit_BoolLiteral(const BoolLiteral &n) { return n.value() ? "true" : "false"; }

    std::string SExprPrinter::visit_NullLiteral(const NullLiteral &) { return "null"; }

    std::string SExprPrinter::visit_Identifier(const Identifier &n) { return n.name(); }

    std::string SExprPrinter::visit_UnaryExpr(const UnaryExpr &n) { return FORMAT("({} {})", unary_op_symbol(n.op()), visit(n.operand())); }

    std::string SExprPrinter::visit_BinaryExpr(const BinaryExpr &n) {
        return FORMAT("({} {} {})", binary_op_symbol(n.op()), visit(n.lhs()), visit(n.rhs()));
    }

    std::string SExprPrinter::visit_TernaryExpr(const TernaryExpr &n) {
        return FORMAT("(?: {} {} {})", visit(n.condition()), visit(n.then_expr()), visit(n.else_expr()));
    }

    std::string SExprPrinter::visit_CallExpr(const CallExpr &n) {
        std::string result = FORMAT("(call {}", visit(n.callee()));
        for(const auto &arg : n.args()) result += " " + visit(*arg);
        result += ")";
        return result;
    }

    std::string SExprPrinter::visit_IndexExpr(const IndexExpr &n) { return FORMAT("(index {} {})", visit(n.object()), visit(n.index())); }

    std::string SExprPrinter::visit_MemberExpr(const MemberExpr &n) { return FORMAT("(. {} {})", visit(n.object()), n.member()); }

    std::string SExprPrinter::visit_AssignExpr(const AssignExpr &n) { return FORMAT("(= {} {})", visit(n.target()), visit(n.value())); }

    std::string SExprPrinter::visit_CastExpr(const CastExpr &n) { return FORMAT("(cast {} {})", n.target_type(), visit(n.operand())); }

    std::string SExprPrinter::visit_ArrayLiteral(const ArrayLiteral &n) {
        std::string result = "[";
        for(std::size_t i = 0; i < n.elements().size(); ++i) {
            if(i > 0) result += " ";
            result += visit(*n.elements()[i]);
        }
        result += "]";
        return result;
    }

    // Statements
    std::string SExprPrinter::visit_ExprStmt(const ExprStmt &n) { return FORMAT("(expr-stmt {})", visit(n.expression())); }

    std::string SExprPrinter::visit_VarDecl(const VarDecl &n) {
        // Multi-variable declaration: var a, b: i64 = 10, 20;
        if(n.num_variables() > 1) {
            std::string result = FORMAT("({}", n.is_const() ? "const" : "var");
            for(std::size_t i = 0; i < n.names().size(); ++i) { result += FORMAT(" {}", n.names()[i]); }
            if(n.type_annotation()) result += FORMAT(" : {}", *n.type_annotation());
            if(!n.initializers().empty()) {
                result += " (";
                for(std::size_t i = 0; i < n.initializers().size(); ++i) {
                    if(i > 0) result += " ";
                    if(n.initializers()[i]) {
                        result += visit(*n.initializers()[i]);
                    } else {
                        result += "null";
                    }
                }
                result += ")";
            }
            result += ")";
            return result;
        }

        // Single variable declaration (backward compatible)
        std::string result = FORMAT("({} {}", n.is_const() ? "const" : "var", n.name());
        if(n.type_annotation()) result += FORMAT(" : {}", *n.type_annotation());
        if(n.has_initializer()) result += " " + visit(n.initializer());
        result += ")";
        return result;
    }

    std::string SExprPrinter::visit_FuncDecl(const FuncDecl &n) {
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

    std::string SExprPrinter::visit_ReturnStmt(const ReturnStmt &n) {
        if(n.has_value()) return FORMAT("(return {})", visit(n.value()));
        return "(return)";
    }

    std::string SExprPrinter::visit_IfStmt(const IfStmt &n) {
        std::string result = FORMAT("(if {} {}", visit(n.condition()), visit(n.then_branch()));
        if(n.has_else()) result += " " + visit(n.else_branch());
        result += ")";
        return result;
    }

    std::string SExprPrinter::visit_WhileStmt(const WhileStmt &n) { return FORMAT("(while {} {})", visit(n.condition()), visit(n.body())); }

    std::string SExprPrinter::visit_ForStmt(const ForStmt &n) {
        std::string result = "(for ";
        result += n.has_init() ? visit(n.init()) : "_";
        result += " ";
        result += n.has_condition() ? visit(n.condition()) : "_";
        result += " ";
        result += n.has_increment() ? visit(n.increment()) : "_";
        result += " " + visit(n.body()) + ")";
        return result;
    }

    std::string SExprPrinter::visit_BlockStmt(const BlockStmt &n) {
        std::string result = "(block";
        for(const auto &s : n.statements()) result += " " + visit(*s);
        result += ")";
        return result;
    }

    std::string SExprPrinter::visit_BreakStmt(const BreakStmt &) { return "(break)"; }

    std::string SExprPrinter::visit_ContinueStmt(const ContinueStmt &) { return "(continue)"; }

    std::string SExprPrinter::visit_PrintStmt(const PrintStmt &n) { return FORMAT("(print {})", visit(n.expression())); }

    std::string SExprPrinter::visit_Program(const Program &n) {
        std::string result = "(program";
        for(const auto &s : n.statements()) result += " " + visit(*s);
        result += ")";
        return result;
    }

}  // namespace jsv

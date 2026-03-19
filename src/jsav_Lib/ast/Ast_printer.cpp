/*
 * Created by gbian on 17/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#include "jsav/ast/Ast_printer.hpp"
namespace jsv {

    // ============================================================
    // AstPrinter - Core methods
    // ============================================================

    void AstPrinter::print(const Node &node) {
        prefix_stack_.clear();
        next_is_last_ = true;
        visit(node);
    }

    void AstPrinter::print_prefix() {
        for(bool is_last : prefix_stack_) { fmt::print("{}", is_last ? "    " : "│   "); }
    }

    void AstPrinter::print_line(std::string_view msg, bool is_last) {
        print_prefix();
        fmt::println("{}{}", is_last ? "└── " : "├── ", msg);
    }

    void AstPrinter::print_value(std::string_view label, std::string_view value, bool is_last) {
        print_prefix();
        fmt::println("{}{}{}", is_last ? "└── " : "├── ", label, value);
    }

    // ============================================================
    // Expressions
    // ============================================================

    void AstPrinter::visit_IntegerLiteral(const IntegerLiteral &node) {
        bool is_last = next_is_last_;
        print_value("Literal ", FORMAT("{}", node.value()), is_last);
    }

    void AstPrinter::visit_FloatLiteral(const FloatLiteral &node) {
        bool is_last = next_is_last_;
        print_value("Literal ", FORMAT("{}f", node.value()), is_last);
    }

    void AstPrinter::visit_StringLiteral(const StringLiteral &node) {
        bool is_last = next_is_last_;
        print_value("Literal ", FORMAT("\"{}\"", node.value()), is_last);
    }

    void AstPrinter::visit_BoolLiteral(const BoolLiteral &node) {
        bool is_last = next_is_last_;
        print_value("Literal ", node.value() ? "true" : "false", is_last);
    }

    void AstPrinter::visit_NullLiteral(const NullLiteral &) {
        bool is_last = next_is_last_;
        print_line("Literal null", is_last);
    }

    void AstPrinter::visit_Identifier(const Identifier &node) {
        bool is_last = next_is_last_;
        print_value("Identifier ", node.name(), is_last);
    }

    void AstPrinter::visit_UnaryExpr(const UnaryExpr &node) {
        bool is_last = next_is_last_;
        print_value("UnaryExpr '", FORMAT("{}'", unary_op_symbol(node.op())), is_last);
        IndentGuard g{*this, is_last};

        print_line("Operand:", true);
        {
            IndentGuard g2{*this, true};
            visit_child(node.operand(), true);
        }
    }

    void AstPrinter::visit_BinaryExpr(const BinaryExpr &node) {
        bool is_last = next_is_last_;
        print_value("BinaryExpr '", FORMAT("{}'", binary_op_symbol(node.op())), is_last);
        IndentGuard g{*this, is_last};

        print_line("Left:", false);
        {
            IndentGuard g2{*this, false};
            visit_child(node.lhs(), true);
        }

        print_line("Right:", true);
        {
            IndentGuard g2{*this, true};
            visit_child(node.rhs(), true);
        }
    }

    void AstPrinter::visit_TernaryExpr(const TernaryExpr &node) {
        bool is_last = next_is_last_;
        print_line("TernaryExpr", is_last);
        IndentGuard g{*this, is_last};

        print_line("Condition:", false);
        {
            IndentGuard g2{*this, false};
            visit_child(node.condition(), true);
        }

        print_line("Then:", false);
        {
            IndentGuard g2{*this, false};
            visit_child(node.then_expr(), true);
        }

        print_line("Else:", true);
        {
            IndentGuard g2{*this, true};
            visit_child(node.else_expr(), true);
        }
    }

    void AstPrinter::visit_CallExpr(const CallExpr &node) {
        bool is_last = next_is_last_;
        print_line("CallExpr", is_last);
        IndentGuard g{*this, is_last};

        print_line("Callee:", false);
        {
            IndentGuard g2{*this, false};
            visit_child(node.callee(), true);
        }

        print_value("Arguments: (", FORMAT("{})", node.args().size()), true);
        if(!node.args().empty()) {
            IndentGuard g2{*this, true};
            for(std::size_t i = 0; i < node.args().size(); ++i) {
                bool arg_last = (i == node.args().size() - 1);
                visit_child(*node.args()[i], arg_last);
            }
        }
    }

    void AstPrinter::visit_IndexExpr(const IndexExpr &node) {
        bool is_last = next_is_last_;
        print_line("IndexExpr", is_last);
        IndentGuard g{*this, is_last};

        print_line("Object:", false);
        {
            IndentGuard g2{*this, false};
            visit_child(node.object(), true);
        }

        print_line("Index:", true);
        {
            IndentGuard g2{*this, true};
            visit_child(node.index(), true);
        }
    }

    void AstPrinter::visit_MemberExpr(const MemberExpr &node) {
        bool is_last = next_is_last_;
        print_value("MemberExpr .", node.member(), is_last);
        IndentGuard g{*this, is_last};

        print_line("Object:", true);
        {
            IndentGuard g2{*this, true};
            visit_child(node.object(), true);
        }
    }

    void AstPrinter::visit_AssignExpr(const AssignExpr &node) {
        bool is_last = next_is_last_;
        print_line("AssignExpr", is_last);
        IndentGuard g{*this, is_last};

        print_line("Target:", false);
        {
            IndentGuard g2{*this, false};
            visit_child(node.target(), true);
        }

        print_line("Value:", true);
        {
            IndentGuard g2{*this, true};
            visit_child(node.value(), true);
        }
    }

    void AstPrinter::visit_CastExpr(const CastExpr &node) {
        bool is_last = next_is_last_;
        print_value("CastExpr -> ", node.target_type(), is_last);
        IndentGuard g{*this, is_last};

        print_line("Operand:", true);
        {
            IndentGuard g2{*this, true};
            visit_child(node.operand(), true);
        }
    }

    void AstPrinter::visit_ArrayLiteral(const ArrayLiteral &node) {
        bool is_last = next_is_last_;
        print_value("ArrayLiteral [", FORMAT("{} elements]", node.elements().size()), is_last);

        if(!node.elements().empty()) {
            IndentGuard g{*this, is_last};
            for(std::size_t i = 0; i < node.elements().size(); ++i) {
                bool elem_last = (i == node.elements().size() - 1);
                visit_child(*node.elements()[i], elem_last);
            }
        }
    }

    void AstPrinter::visit_GroupingExpr(const GroupingExpr &node) {
        bool is_last = next_is_last_;
        print_line("GroupingExpr", is_last);
        IndentGuard g{*this, is_last};
        visit_child(node.expression(), true);
    }

    // ============================================================
    // Statements
    // ============================================================

    void AstPrinter::visit_ExprStmt(const ExprStmt &node) {
        bool is_last = next_is_last_;
        print_line("ExprStmt", is_last);
        IndentGuard g{*this, is_last};
        visit_child(node.expression(), true);
    }

    void AstPrinter::visit_VarDecl(const VarDecl &node) {
        bool is_last = next_is_last_;
        const std::string keyword = node.is_const() ? "Const" : "Var";

        if(node.num_variables() > 1) {
            // Multi-variable declaration
            print_line(keyword, is_last);
            IndentGuard g{*this, is_last};

            // Names
            print_line("Names:", false);
            {
                IndentGuard g2{*this, false};
                for(std::size_t i = 0; i < node.names().size(); ++i) {
                    bool name_last = (i == node.names().size() - 1);
                    print_line(node.names()[i], name_last);
                }
            }

            // Type annotation
            if(node.type_annotation()) { print_value("Type: ", *node.type_annotation(), false); }

            // Initializers
            if(!node.initializers().empty()) {
                print_line("Initializers:", true);
                IndentGuard g2{*this, true};
                for(std::size_t i = 0; i < node.initializers().size(); ++i) {
                    bool init_last = (i == node.initializers().size() - 1);
                    if(node.initializers()[i]) {
                        print_value("", FORMAT("[{}]", i), init_last);
                        IndentGuard g3{*this, init_last};
                        visit_child(*node.initializers()[i], true);
                    }
                }
            }
        } else {
            // Single variable declaration
            print_value(keyword, FORMAT(" '{}'", node.name()), is_last);
            IndentGuard g{*this, is_last};

            bool has_type = node.type_annotation().has_value();
            bool has_init = node.has_initializer();

            if(has_type) { print_value("Type: ", *node.type_annotation(), !has_init); }

            if(has_init) {
                print_line("Initializer:", true);
                IndentGuard g2{*this, true};
                visit_child(node.initializer(), true);
            }
        }
    }

    void AstPrinter::visit_FuncDecl(const FuncDecl &node) {
        bool is_last = next_is_last_;
        print_line("Function", is_last);
        IndentGuard g{*this, is_last};

        bool has_params = !node.params().empty();
        bool has_return = node.return_type().has_value();

        // Name
        print_line("Name:", false);
        {
            IndentGuard g2{*this, false};
            print_line(node.name(), true);
        }

        // Parameters
        if(has_params) {
            print_line("Parameters:", !has_return);
            IndentGuard g2{*this, !has_return};
            for(std::size_t i = 0; i < node.params().size(); ++i) {
                bool param_last = (i == node.params().size() - 1);
                const auto &param = node.params()[i];
                print_value("Parameter '", FORMAT("{}'", param.name), param_last);
                IndentGuard g3{*this, param_last};
                print_value("Type: ", param.type, true);
            }
        } else {
            print_line("Parameters: (none)", !has_return);
        }

        // Return Type
        if(has_return) {
            print_line("Return Type:", false);
            IndentGuard g2{*this, false};
            print_line(*node.return_type(), true);
        }

        // Body
        print_line("Body:", true);
        {
            IndentGuard g2{*this, true};
            visit_child(node.body(), true);
        }
    }

    void AstPrinter::visit_ReturnStmt(const ReturnStmt &node) {
        bool is_last = next_is_last_;
        print_line("Return", is_last);

        if(node.has_value()) {
            IndentGuard g{*this, is_last};
            print_line("Value:", true);
            IndentGuard g2{*this, true};
            visit_child(node.value(), true);
        }
    }

    void AstPrinter::visit_IfStmt(const IfStmt &node) {
        bool is_last = next_is_last_;
        print_line("If", is_last);
        IndentGuard g{*this, is_last};

        bool has_else = node.has_else();

        print_line("Condition:", false);
        {
            IndentGuard g2{*this, false};
            visit_child(node.condition(), true);
        }

        print_line("Then:", !has_else);
        {
            IndentGuard g2{*this, !has_else};
            visit_child(node.then_branch(), true);
        }

        if(has_else) {
            print_line("Else:", true);
            IndentGuard g2{*this, true};
            visit_child(node.else_branch(), true);
        }
    }

    void AstPrinter::visit_WhileStmt(const WhileStmt &node) {
        bool is_last = next_is_last_;
        print_line("While", is_last);
        IndentGuard g{*this, is_last};

        print_line("Condition:", false);
        {
            IndentGuard g2{*this, false};
            visit_child(node.condition(), true);
        }

        print_line("Body:", true);
        {
            IndentGuard g2{*this, true};
            visit_child(node.body(), true);
        }
    }

    void AstPrinter::visit_ForStmt(const ForStmt &node) {
        bool is_last = next_is_last_;
        print_line("For", is_last);
        IndentGuard g{*this, is_last};

        bool has_init = node.has_init();
        bool has_cond = node.has_condition();
        bool has_incr = node.has_increment();

        if(has_init) {
            print_line("Init:", false);
            IndentGuard g2{*this, false};
            visit_child(node.init(), true);
        } else {
            print_line("Init: (none)", false);
        }

        if(has_cond) {
            print_line("Condition:", false);
            IndentGuard g2{*this, false};
            visit_child(node.condition(), true);
        } else {
            print_line("Condition: (none)", false);
        }

        if(has_incr) {
            print_line("Increment:", false);
            IndentGuard g2{*this, false};
            visit_child(node.increment(), true);
        } else {
            print_line("Increment: (none)", false);
        }

        print_line("Body:", true);
        {
            IndentGuard g2{*this, true};
            visit_child(node.body(), true);
        }
    }

    void AstPrinter::visit_BlockStmt(const BlockStmt &node) {
        bool is_last = next_is_last_;
        print_line("Block", is_last);

        if(!node.statements().empty()) {
            IndentGuard g{*this, is_last};
            for(std::size_t i = 0; i < node.statements().size(); ++i) {
                bool stmt_last = (i == node.statements().size() - 1);
                visit_child(*node.statements()[i], stmt_last);
            }
        }
    }

    void AstPrinter::visit_BreakStmt(const BreakStmt &) {
        bool is_last = next_is_last_;
        print_line("Break", is_last);
    }

    void AstPrinter::visit_ContinueStmt(const ContinueStmt &) {
        bool is_last = next_is_last_;
        print_line("Continue", is_last);
    }

    void AstPrinter::visit_PrintStmt(const PrintStmt &node) {
        bool is_last = next_is_last_;
        print_line("Print", is_last);
        IndentGuard g{*this, is_last};
        visit_child(node.expression(), true);
    }

    void AstPrinter::visit_Program(const Program &node) {
        // Program è la radice, stampa senza prefisso
        fmt::println("Program");

        if(!node.statements().empty()) {
            for(std::size_t i = 0; i < node.statements().size(); ++i) {
                bool stmt_last = (i == node.statements().size() - 1);
                visit_child(*node.statements()[i], stmt_last);
            }
        }
    }

    // ============================================================
    // SExprPrinter implementation (invariato - mantieni come prima)
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

    std::string SExprPrinter::visit_GroupingExpr(const GroupingExpr &n) { return FORMAT("(group {})", visit(n.expression())); }

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
                    result += n.initializers()[i] ? visit(*n.initializers()[i]) : "null";
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

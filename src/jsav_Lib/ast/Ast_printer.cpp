/*
 * Created by gbian on 17/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */

// NOLINTBEGIN(*-include-cleaner)
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
        for(const bool is_last : prefix_stack_) { fmt::print("{}", is_last ? "    " : "│   "); }
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
        const bool is_last = next_is_last_;
        print_value("Literal ", FORMAT("{}", node.value()), is_last);
    }

    void AstPrinter::visit_FloatLiteral(const FloatLiteral &node) {
        const bool is_last = next_is_last_;
        print_value("Literal ", FORMAT("{}f", node.value()), is_last);
    }

    void AstPrinter::visit_StringLiteral(const StringLiteral &node) {
        const bool is_last = next_is_last_;
        print_value("Literal ", FORMAT("\"{}\"", node.value()), is_last);
    }

    void AstPrinter::visit_BoolLiteral(const BoolLiteral &node) {
        const bool is_last = next_is_last_;
        print_value("Literal ", node.value() ? "true" : "false", is_last);
    }

    void AstPrinter::visit_NullLiteral(const NullLiteral & /*unused*/) {
        const bool is_last = next_is_last_;
        print_line("Literal null", is_last);
    }

    void AstPrinter::visit_Identifier(const Identifier &node) {
        const bool is_last = next_is_last_;
        print_value("Identifier ", node.name(), is_last);
    }

    void AstPrinter::visit_UnaryExpr(const UnaryExpr &node) {
        const bool is_last = next_is_last_;
        print_value("UnaryExpr '", FORMAT("{}'", unary_op_symbol(node.op())), is_last);
        const IndentGuard guard{*this, is_last};

        print_line("Operand:", true);
        {
            const IndentGuard guard2{*this, true};
            visit_child(node.operand(), true);
        }
    }

    void AstPrinter::visit_BinaryExpr(const BinaryExpr &node) {
        const bool is_last = next_is_last_;
        print_value("BinaryExpr '", FORMAT("{}'", binary_op_symbol(node.op())), is_last);
        const IndentGuard guard{*this, is_last};

        print_line("Left:", false);
        {
            const IndentGuard guard2{*this, false};
            visit_child(node.lhs(), true);
        }

        print_line("Right:", true);
        {
            const IndentGuard guard2{*this, true};
            visit_child(node.rhs(), true);
        }
    }

    void AstPrinter::visit_TernaryExpr(const TernaryExpr &node) {
        const bool is_last = next_is_last_;
        print_line("TernaryExpr", is_last);
        const IndentGuard guard{*this, is_last};

        print_line("Condition:", false);
        {
            const IndentGuard guard2{*this, false};
            visit_child(node.condition(), true);
        }

        print_line("Then:", false);
        {
            const IndentGuard guard2{*this, false};
            visit_child(node.then_expr(), true);
        }

        print_line("Else:", true);
        {
            const IndentGuard guard2{*this, true};
            visit_child(node.else_expr(), true);
        }
    }

    void AstPrinter::visit_CallExpr(const CallExpr &node) {
        const bool is_last = next_is_last_;
        print_line("CallExpr", is_last);
        const IndentGuard guard{*this, is_last};

        print_line("Callee:", false);
        {
            const IndentGuard guard2{*this, false};
            visit_child(node.callee(), true);
        }

        print_value("Arguments: (", FORMAT("{})", node.args().size()), true);
        if(!node.args().empty()) {
            const IndentGuard guard2{*this, true};
            for(std::size_t i = 0; i < node.args().size(); ++i) {
                const bool arg_last = (i == node.args().size() - 1);
                visit_child(*node.args()[i], arg_last);
            }
        }
    }

    void AstPrinter::visit_IndexExpr(const IndexExpr &node) {
        const bool is_last = next_is_last_;
        print_line("IndexExpr", is_last);
        const IndentGuard guard{*this, is_last};

        print_line("Object:", false);
        {
            const IndentGuard guard2{*this, false};
            visit_child(node.object(), true);
        }

        print_line("Index:", true);
        {
            const IndentGuard guard2{*this, true};
            visit_child(node.index(), true);
        }
    }

    void AstPrinter::visit_MemberExpr(const MemberExpr &node) {
        const bool is_last = next_is_last_;
        print_value("MemberExpr .", node.member(), is_last);
        const IndentGuard guard{*this, is_last};

        print_line("Object:", true);
        {
            const IndentGuard guard2{*this, true};
            visit_child(node.object(), true);
        }
    }

    void AstPrinter::visit_AssignExpr(const AssignExpr &node) {
        const bool is_last = next_is_last_;
        print_line("AssignExpr", is_last);
        const IndentGuard guard{*this, is_last};

        print_line("Target:", false);
        {
            const IndentGuard guard2{*this, false};
            visit_child(node.target(), true);
        }

        print_line("Value:", true);
        {
            const IndentGuard guard2{*this, true};
            visit_child(node.value(), true);
        }
    }

    void AstPrinter::visit_CastExpr(const CastExpr &node) {
        const bool is_last = next_is_last_;
        print_value("CastExpr -> ", node.target_type(), is_last);
        const IndentGuard guard{*this, is_last};

        print_line("Operand:", true);
        {
            const IndentGuard guard2{*this, true};
            visit_child(node.operand(), true);
        }
    }

    void AstPrinter::visit_ArrayLiteral(const ArrayLiteral &node) {
        const bool is_last = next_is_last_;
        print_value("ArrayLiteral [", FORMAT("{} elements]", node.elements().size()), is_last);

        if(!node.elements().empty()) {
            const IndentGuard guard{*this, is_last};
            for(std::size_t i = 0; i < node.elements().size(); ++i) {
                const bool elem_last = (i == node.elements().size() - 1);
                visit_child(*node.elements()[i], elem_last);
            }
        }
    }

    void AstPrinter::visit_GroupingExpr(const GroupingExpr &node) {
        const bool is_last = next_is_last_;
        print_line("GroupingExpr", is_last);
        const IndentGuard guard{*this, is_last};
        visit_child(node.expression(), true);
    }

    // ============================================================
    // Statements
    // ============================================================

    void AstPrinter::visit_ExprStmt(const ExprStmt &node) {
        const bool is_last = next_is_last_;
        print_line("ExprStmt", is_last);
        const IndentGuard guard{*this, is_last};
        visit_child(node.expression(), true);
    }

    void AstPrinter::visit_VarDecl(const VarDecl &node) {
        const bool is_last = next_is_last_;
        const std::string keyword = node.is_const() ? "Const" : "Var";

        if(node.num_variables() > 1) {
            // Multi-variable declaration
            print_line(keyword, is_last);
            const IndentGuard guard{*this, is_last};

            // Names
            print_line("Names:", false);
            {
                const IndentGuard guard2{*this, false};
                for(std::size_t i = 0; i < node.names().size(); ++i) {
                    const bool name_last = (i == node.names().size() - 1);
                    print_line(node.names()[i], name_last);
                }
            }

            // Type annotation
            if(node.type_annotation()) { print_value("Type: ", node.type_annotation().value(), false); }

            // Initializers
            if(!node.initializers().empty()) {
                print_line("Initializers:", true);
                const IndentGuard guard2{*this, true};
                for(std::size_t i = 0; i < node.initializers().size(); ++i) {
                    const bool init_last = (i == node.initializers().size() - 1);
                    if(node.initializers()[i]) {
                        print_value("", FORMAT("[{}]", i), init_last);
                        const IndentGuard guard3{*this, init_last};
                        visit_child(*node.initializers()[i], true);
                    }
                }
            }
        } else {
            // Single variable declaration
            print_value(keyword, FORMAT(" '{}'", node.name()), is_last);
            const IndentGuard guard{*this, is_last};

            const bool has_type = node.type_annotation().has_value();
            const bool has_init = node.has_initializer();

            if(has_type) { print_value("Type: ", node.type_annotation().value(), !has_init); }

            if(has_init) {
                print_line("Initializer:", true);
                const IndentGuard guard2{*this, true};
                visit_child(node.initializer(), true);
            }
        }
    }

    void AstPrinter::visit_FuncDecl(const FuncDecl &node) {
        const bool is_last = next_is_last_;
        print_line("Function", is_last);
        const IndentGuard guard{*this, is_last};

        const bool has_params = !node.params().empty();
        const bool has_return = node.return_type().has_value();

        // Name
        print_line("Name:", false);
        {
            const IndentGuard guard2{*this, false};
            print_line(node.name(), true);
        }

        // Parameters
        if(has_params) {
            print_line("Parameters:", !has_return);
            const IndentGuard guard2{*this, !has_return};
            for(std::size_t i = 0; i < node.params().size(); ++i) {
                const bool param_last = (i == node.params().size() - 1);
                const auto &param = node.params()[i];
                print_value("Parameter '", FORMAT("{}'", param.name), param_last);
                const IndentGuard guard3{*this, param_last};
                print_value("Type: ", param.type, true);
            }
        } else {
            print_line("Parameters: (none)", !has_return);
        }

        // Return Type
        if(has_return) {
            print_line("Return Type:", false);
            const IndentGuard guard2{*this, false};
            print_line(node.return_type().value(), true);
        }

        // Body
        print_line("Body:", true);
        {
            const IndentGuard guard2{*this, true};
            visit_child(node.body(), true);
        }
    }

    void AstPrinter::visit_ReturnStmt(const ReturnStmt &node) {
        const bool is_last = next_is_last_;
        print_line("Return", is_last);

        if(node.has_value()) {
            const IndentGuard guard{*this, is_last};
            print_line("Value:", true);
            const IndentGuard guard2{*this, true};
            visit_child(node.value(), true);
        }
    }

    void AstPrinter::visit_IfStmt(const IfStmt &node) {
        const bool is_last = next_is_last_;
        print_line("If", is_last);
        const IndentGuard guard{*this, is_last};

        const bool has_else = node.has_else();

        print_line("Condition:", false);
        {
            const IndentGuard guard2{*this, false};
            visit_child(node.condition(), true);
        }

        print_line("Then:", !has_else);
        {
            const IndentGuard guard2{*this, !has_else};
            visit_child(node.then_branch(), true);
        }

        if(has_else) {
            print_line("Else:", true);
            const IndentGuard guard2{*this, true};
            visit_child(node.else_branch(), true);
        }
    }

    void AstPrinter::visit_WhileStmt(const WhileStmt &node) {
        const bool is_last = next_is_last_;
        print_line("While", is_last);
        const IndentGuard guard{*this, is_last};

        print_line("Condition:", false);
        {
            const IndentGuard guard2{*this, false};
            visit_child(node.condition(), true);
        }

        print_line("Body:", true);
        {
            const IndentGuard guard2{*this, true};
            visit_child(node.body(), true);
        }
    }

    void AstPrinter::visit_ForStmt(const ForStmt &node) {
        const bool is_last = next_is_last_;
        print_line("For", is_last);
        const IndentGuard guard{*this, is_last};

        const bool has_init = node.has_init();
        const bool has_cond = node.has_condition();
        const bool has_incr = node.has_increment();

        if(has_init) {
            print_line("Init:", false);
            const IndentGuard guard2{*this, false};
            visit_child(node.init(), true);
        } else {
            print_line("Init: (none)", false);
        }

        if(has_cond) {
            print_line("Condition:", false);
            const IndentGuard guard2{*this, false};
            visit_child(node.condition(), true);
        } else {
            print_line("Condition: (none)", false);
        }

        if(has_incr) {
            print_line("Increment:", false);
            const IndentGuard guard2{*this, false};
            visit_child(node.increment(), true);
        } else {
            print_line("Increment: (none)", false);
        }

        print_line("Body:", true);
        {
            const IndentGuard guard2{*this, true};
            visit_child(node.body(), true);
        }
    }

    void AstPrinter::visit_BlockStmt(const BlockStmt &node) {
        const bool is_last = next_is_last_;
        print_line("Block", is_last);

        if(!node.statements().empty()) {
            const IndentGuard guard{*this, is_last};
            for(std::size_t i = 0; i < node.statements().size(); ++i) {
                const bool stmt_last = (i == node.statements().size() - 1);
                visit_child(*node.statements()[i], stmt_last);
            }
        }
    }

    void AstPrinter::visit_BreakStmt(const BreakStmt & /*unused*/) {
        const bool is_last = next_is_last_;
        print_line("Break", is_last);
    }

    void AstPrinter::visit_ContinueStmt(const ContinueStmt & /*unused*/) {
        const bool is_last = next_is_last_;
        print_line("Continue", is_last);
    }

    void AstPrinter::visit_PrintStmt(const PrintStmt &node) {
        const bool is_last = next_is_last_;
        print_line("Print", is_last);
        const IndentGuard guard{*this, is_last};
        visit_child(node.expression(), true);
    }

    void AstPrinter::visit_Program(const Program &node) {
        // Program is the root — print without prefix
        fmt::println("Program");

        if(!node.statements().empty()) {
            for(std::size_t i = 0; i < node.statements().size(); ++i) {
                const bool stmt_last = (i == node.statements().size() - 1);
                visit_child(*node.statements()[i], stmt_last);
            }
        }
    }

    // ============================================================
    // SExprPrinter implementation
    // ============================================================

    std::string SExprPrinter::to_string(const Node &node) { return visit(node); }

    // Expressions
    std::string SExprPrinter::visit_IntegerLiteral(const IntegerLiteral &node) { return std::to_string(node.value()); }

    std::string SExprPrinter::visit_FloatLiteral(const FloatLiteral &node) { return FORMAT("{}", node.value()); }

    std::string SExprPrinter::visit_StringLiteral(const StringLiteral &node) { return FORMAT("\"{}\"", node.value()); }

    std::string SExprPrinter::visit_BoolLiteral(const BoolLiteral &node) { return node.value() ? "true" : "false"; }

    std::string SExprPrinter::visit_NullLiteral(const NullLiteral & /*unused*/) { return "null"; }

    std::string SExprPrinter::visit_Identifier(const Identifier &node) { return node.name(); }

    std::string SExprPrinter::visit_UnaryExpr(const UnaryExpr &node) {
        return FORMAT("({} {})", unary_op_symbol(node.op()), visit(node.operand()));
    }

    std::string SExprPrinter::visit_BinaryExpr(const BinaryExpr &node) {
        return FORMAT("({} {} {})", binary_op_symbol(node.op()), visit(node.lhs()), visit(node.rhs()));
    }

    std::string SExprPrinter::visit_TernaryExpr(const TernaryExpr &node) {
        return FORMAT("(?: {} {} {})", visit(node.condition()), visit(node.then_expr()), visit(node.else_expr()));
    }

    std::string SExprPrinter::visit_CallExpr(const CallExpr &node) {
        std::string result = FORMAT("(call {}", visit(node.callee()));
        for(const auto &arg : node.args()) { result += " " + visit(*arg); }
        result += ")";
        return result;
    }

    std::string SExprPrinter::visit_IndexExpr(const IndexExpr &node) {
        return FORMAT("(index {} {})", visit(node.object()), visit(node.index()));
    }

    std::string SExprPrinter::visit_MemberExpr(const MemberExpr &node) { return FORMAT("(. {} {})", visit(node.object()), node.member()); }

    std::string SExprPrinter::visit_AssignExpr(const AssignExpr &node) {
        return FORMAT("(= {} {})", visit(node.target()), visit(node.value()));
    }

    std::string SExprPrinter::visit_CastExpr(const CastExpr &node) {
        return FORMAT("(cast {} {})", node.target_type(), visit(node.operand()));
    }

    std::string SExprPrinter::visit_ArrayLiteral(const ArrayLiteral &node) {
        std::string result = "[";
        for(std::size_t i = 0; i < node.elements().size(); ++i) {
            if(i > 0) { result += " "; }
            result += visit(*node.elements()[i]);
        }
        result += "]";
        return result;
    }

    std::string SExprPrinter::visit_GroupingExpr(const GroupingExpr &node) { return FORMAT("(group {})", visit(node.expression())); }

    // Statements
    std::string SExprPrinter::visit_ExprStmt(const ExprStmt &node) { return FORMAT("(expr-stmt {})", visit(node.expression())); }

    std::string SExprPrinter::visit_VarDecl(const VarDecl &node) {
        // Multi-variable declaration: var a, b: i64 = 10, 20;
        if(node.num_variables() > 1) {
            std::string result = FORMAT("({}", node.is_const() ? "const" : "var");
            for(const auto &varname : node.names()) { result += FORMAT(" {}", varname); }
            if(node.type_annotation()) { result += FORMAT(" : {}", node.type_annotation().value()); }
            if(!node.initializers().empty()) {
                result += " (";
                for(std::size_t i = 0; i < node.initializers().size(); ++i) {
                    if(i > 0) { result += " "; }
                    result += node.initializers()[i] ? visit(*node.initializers()[i]) : "null";
                }
                result += ")";
            }
            result += ")";
            return result;
        }

        // Single variable declaration (backward compatible)
        std::string result = FORMAT("({} {}", node.is_const() ? "const" : "var", node.name());
        if(node.type_annotation()) { result += FORMAT(" : {}", node.type_annotation().value()); }
        if(node.has_initializer()) { result += " " + visit(node.initializer()); }
        result += ")";
        return result;
    }

    std::string SExprPrinter::visit_FuncDecl(const FuncDecl &node) {
        std::string result = FORMAT("(fn {} (", node.name());
        for(std::size_t i = 0; i < node.params().size(); ++i) {
            if(i > 0) { result += " "; }
            result += FORMAT("({} {})", node.params()[i].name, node.params()[i].type);
        }
        result += ")";
        if(node.return_type()) { result += FORMAT(" -> {}", node.return_type().value()); }
        result += " " + visit_BlockStmt(node.body()) + ")";
        return result;
    }

    std::string SExprPrinter::visit_ReturnStmt(const ReturnStmt &node) {
        if(node.has_value()) { return FORMAT("(return {})", visit(node.value())); }
        return "(return)";
    }

    std::string SExprPrinter::visit_IfStmt(const IfStmt &node) {
        std::string result = FORMAT("(if {} {}", visit(node.condition()), visit(node.then_branch()));
        if(node.has_else()) { result += " " + visit(node.else_branch()); }
        result += ")";
        return result;
    }

    std::string SExprPrinter::visit_WhileStmt(const WhileStmt &node) {
        return FORMAT("(while {} {})", visit(node.condition()), visit(node.body()));
    }

    std::string SExprPrinter::visit_ForStmt(const ForStmt &node) {
        std::string result = "(for ";
        result += node.has_init() ? visit(node.init()) : "_";
        result += " ";
        result += node.has_condition() ? visit(node.condition()) : "_";
        result += " ";
        result += node.has_increment() ? visit(node.increment()) : "_";
        result += " " + visit(node.body()) + ")";
        return result;
    }

    std::string SExprPrinter::visit_BlockStmt(const BlockStmt &node) {
        std::string result = "(block";
        for(const auto &stmt : node.statements()) { result += " " + visit(*stmt); }
        result += ")";
        return result;
    }

    std::string SExprPrinter::visit_BreakStmt(const BreakStmt & /*unused*/) { return "(break)"; }

    std::string SExprPrinter::visit_ContinueStmt(const ContinueStmt & /*unused*/) { return "(continue)"; }

    std::string SExprPrinter::visit_PrintStmt(const PrintStmt &node) { return FORMAT("(print {})", visit(node.expression())); }

    std::string SExprPrinter::visit_Program(const Program &node) {
        std::string result = "(program";
        for(const auto &stmt : node.statements()) { result += " " + visit(*stmt); }
        result += ")";
        return result;
    }

}  // namespace jsv
// NOLINTEND(*-include-cleaner)
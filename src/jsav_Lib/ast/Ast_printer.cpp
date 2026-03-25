/*
 * Created by gbian on 17/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */

// NOLINTBEGIN(*-include-cleaner)
#include "jsav/ast/Ast_printer.hpp"
#include "jsav/util/AnsiStyles.hpp"

namespace jsv {

    // ============================================================
    // AstPrinter - Core methods
    // ============================================================

    void AstPrinter::print(const Node &node) {
        prefix_stack_.clear();
        next_is_last_ = true;
        visit(node);
    }

    void AstPrinter::print_prefix() const {
        for(const bool is_last : prefix_stack_) { fmt::print("{}", is_last ? "    " : "│   "); }
    }

    void AstPrinter::print_line(std::string_view msg, bool is_last) const {
        print_prefix();
        fmt::println("{}{}", is_last ? "└── " : "├── ", msg);
    }

    void AstPrinter::print_value(std::string_view label, std::string_view value, bool is_last) const {
        print_prefix();
        fmt::println("{}{}{}", is_last ? "└── " : "├── ", label, value);
    }

    // ============================================================
    // Expressions
    // ============================================================

    // cppcheck-suppress functionConst
    void AstPrinter::visit_IntegerLiteral(const IntegerLiteral &node) {
        const bool is_last = next_is_last_;
        std::string value_str;
        if(const auto &type_suffix = node.type_suffix(); type_suffix.has_value()) {
            value_str = FORMAT("{}{}", node.value(), *type_suffix);
        } else {
            value_str = FORMAT("{}", node.value());
        }
        print_value(ansi::green("Literal "), value_str, is_last);
    }

    // cppcheck-suppress functionConst
    void AstPrinter::visit_FloatLiteral(const FloatLiteral &node) {
        const bool is_last = next_is_last_;
        print_value(ansi::green("Literal "), FORMAT("{}f", node.value()), is_last);
    }

    // cppcheck-suppress functionConst
    void AstPrinter::visit_StringLiteral(const StringLiteral &node) {
        const bool is_last = next_is_last_;
        print_value(ansi::green("Literal "), FORMAT("\"{}\"", node.value()), is_last);
    }

    // cppcheck-suppress functionConst
    void AstPrinter::visit_CharLiteral(const CharLiteral &node) {
        const bool is_last = next_is_last_;
        print_value(ansi::green("Literal "), FORMAT("'{}'", node.value()), is_last);
    }

    // cppcheck-suppress functionConst
    void AstPrinter::visit_BoolLiteral(const BoolLiteral &node) {
        const bool is_last = next_is_last_;
        print_value(ansi::green("Literal "), node.value() ? "true" : "false", is_last);
    }

    // cppcheck-suppress functionConst
    void AstPrinter::visit_NullLiteral(const NullLiteral & /*unused*/) {
        const bool is_last = next_is_last_;
        print_line(ansi::green("Literal null"), is_last);
    }

    // cppcheck-suppress functionConst
    void AstPrinter::visit_Identifier(const Identifier &node) {
        const bool is_last = next_is_last_;
        print_value(ansi::blue("Identifier "), node.name(), is_last);
    }

    void AstPrinter::visit_UnaryExpr(const UnaryExpr &node) {
        const bool is_last = next_is_last_;
        const std::string_view op_str = unary_op_symbol(node.op());
        std::string position;
        if(node.op() == UnaryOp::PreInc || node.op() == UnaryOp::PreDec) {
            position = " (prefix)";
        } else if(node.op() == UnaryOp::PostInc || node.op() == UnaryOp::PostDec) {
            position = " (postfix)";
        }
        print_value(ansi::cyan("UnaryExpr "), FORMAT("'{}'{}", op_str, position), is_last);
        const IndentGuard guard{*this, is_last};

        print_line(ansi::cyan("Operand:"), true);
        {
            const IndentGuard guard2{*this, true};
            visit_child(node.operand(), true);
        }
    }

    void AstPrinter::visit_BinaryExpr(const BinaryExpr &node) {
        const bool is_last = next_is_last_;
        print_value(ansi::cyan("BinaryExpr "), FORMAT("'{}'", binary_op_symbol(node.op())), is_last);
        const IndentGuard guard{*this, is_last};

        print_line(ansi::cyan("Left:"), false);
        {
            const IndentGuard guard2{*this, false};
            visit_child(node.lhs(), true);
        }

        print_line(ansi::cyan("Right:"), true);
        {
            const IndentGuard guard2{*this, true};
            visit_child(node.rhs(), true);
        }
    }

    void AstPrinter::visit_TernaryExpr(const TernaryExpr &node) {
        const bool is_last = next_is_last_;
        print_line(ansi::cyan("TernaryExpr"), is_last);
        const IndentGuard guard{*this, is_last};

        print_line(ansi::cyan("Condition:"), false);
        {
            const IndentGuard guard2{*this, false};
            visit_child(node.condition(), true);
        }

        print_line(ansi::cyan("Then:"), false);
        {
            const IndentGuard guard2{*this, false};
            visit_child(node.then_expr(), true);
        }

        print_line(ansi::cyan("Else:"), true);
        {
            const IndentGuard guard2{*this, true};
            visit_child(node.else_expr(), true);
        }
    }

    void AstPrinter::visit_CallExpr(const CallExpr &node) {
        const bool is_last = next_is_last_;
        print_line(ansi::cyan("CallExpr"), is_last);
        const IndentGuard guard{*this, is_last};

        print_line(ansi::red("Callee:"), false);
        {
            const IndentGuard guard2{*this, false};
            visit_child(node.callee(), true);
        }

        print_value(ansi::green("Arguments: "), FORMAT("({})", node.args().size()), true);
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
        print_line(ansi::cyan("IndexExpr"), is_last);
        const IndentGuard guard{*this, is_last};

        print_line(ansi::blue("Object:"), false);
        {
            const IndentGuard guard2{*this, false};
            visit_child(node.object(), true);
        }

        print_line(ansi::red("Index:"), true);
        {
            const IndentGuard guard2{*this, true};
            visit_child(node.index(), true);
        }
    }

    void AstPrinter::visit_MemberExpr(const MemberExpr &node) {
        const bool is_last = next_is_last_;
        print_value(ansi::cyan("MemberExpr "), FORMAT(".{}", node.member()), is_last);
        const IndentGuard guard{*this, is_last};

        print_line(ansi::blue("Object:"), true);
        {
            const IndentGuard guard2{*this, true};
            visit_child(node.object(), true);
        }
    }

    void AstPrinter::visit_AssignExpr(const AssignExpr &node) {
        const bool is_last = next_is_last_;
        print_line(ansi::cyan("AssignExpr"), is_last);
        const IndentGuard guard{*this, is_last};

        print_line(ansi::blue("Target:"), false);
        {
            const IndentGuard guard2{*this, false};
            visit_child(node.target(), true);
        }

        print_line(ansi::red("Value:"), true);
        {
            const IndentGuard guard2{*this, true};
            visit_child(node.value(), true);
        }
    }

    void AstPrinter::visit_CastExpr(const CastExpr &node) {
        const bool is_last = next_is_last_;
        print_value(ansi::cyan("CastExpr "), FORMAT("-> {}", node.target_type()), is_last);
        const IndentGuard guard{*this, is_last};

        print_line(ansi::green("Operand:"), true);
        {
            const IndentGuard guard2{*this, true};
            visit_child(node.operand(), true);
        }
    }

    void AstPrinter::visit_ArrayLiteral(const ArrayLiteral &node) {
        const bool is_last = next_is_last_;
        print_line(ansi::cyan("Array Literal"), is_last);

        if(!node.elements().empty()) {
            const IndentGuard guard{*this, is_last};
            print_line(ansi::cyan("Elements:"), true);
            const IndentGuard guard2{*this, true};
            for(std::size_t i = 0; i < node.elements().size(); ++i) {
                const bool elem_last = (i == node.elements().size() - 1);
                visit_child(*node.elements()[i], elem_last);
            }
        }
    }

    void AstPrinter::visit_GroupingExpr(const GroupingExpr &node) {
        const bool is_last = next_is_last_;
        print_line(ansi::cyan("GroupingExpr"), is_last);
        const IndentGuard guard{*this, is_last};
        visit_child(node.expression(), true);
    }

    // ============================================================
    // Statements
    // ============================================================

    void AstPrinter::visit_ExprStmt(const ExprStmt &node) {
        const bool is_last = next_is_last_;
        print_line(ansi::yellow("ExprStmt"), is_last);
        const IndentGuard guard{*this, is_last};
        visit_child(node.expression(), true);
    }

    void AstPrinter::visit_VarDecl(const VarDecl &node) {
        const bool is_last = next_is_last_;
        const std::string keyword = node.is_const() ? ansi::blue_bold("ConstDeclaration") : ansi::blue("VarDeclaration");

        if(node.num_variables() > 1) {
            // Multi-variable declaration
            print_line(keyword, is_last);
            const IndentGuard guard{*this, is_last};

            // Names
            print_line(ansi::yellow("Variables:"), false);
            {
                const IndentGuard guard2{*this, false};
                for(std::size_t i = 0; i < node.names().size(); ++i) {
                    const bool name_last = (i == node.names().size() - 1);
                    print_line(node.names()[i], name_last);
                }
            }

            // Type annotation
            const bool has_init = !node.initializers().empty();
            if(const auto &type_ann = node.type_annotation(); type_ann.has_value()) {
                print_value(ansi::magenta("Type: "), *type_ann, !has_init);
            }

            // Initializers
            if(has_init) {
                print_line(ansi::red("Initializers:"), true);
                const IndentGuard guard2{*this, true};
                for(std::size_t i = 0; i < node.initializers().size(); ++i) {
                    const bool init_last = (i == node.initializers().size() - 1);
                    if(node.initializers()[i]) { visit_child(*node.initializers()[i], init_last); }
                }
            }
        } else {
            // Single variable declaration
            print_line(keyword, is_last);
            const IndentGuard guard{*this, is_last};

            const auto &type_ann = node.type_annotation();
            const bool has_type = type_ann.has_value();
            const bool has_init = node.has_initializer();

            // Variables
            print_line(ansi::yellow("Variables:"), false);
            {
                const IndentGuard guard2{*this, false};
                print_line(node.name(), true);
            }

            // Type annotation
            if(has_type) { print_value(ansi::magenta("Type: "), *type_ann, !has_init); }

            // Initializers
            if(has_init) {
                print_line(ansi::red("Initializers:"), true);
                const IndentGuard guard2{*this, true};
                visit_child(node.initializer(), true);
            }
        }
    }

    void AstPrinter::visit_FuncDecl(const FuncDecl &node) {
        const bool is_last = next_is_last_;
        print_line(ansi::blue_bold("Function"), is_last);
        const IndentGuard guard{*this, is_last};

        const bool has_params = !node.params().empty();
        const auto &ret_type = node.return_type();
        const bool has_return = ret_type.has_value();

        // Name
        print_line(ansi::yellow("Name:"), false);
        {
            const IndentGuard guard2{*this, false};
            print_line(node.name(), true);
        }

        // Parameters
        if(has_params) {
            print_line(ansi::green("Parameters:"), !has_return);
            const IndentGuard guard2{*this, !has_return};
            for(std::size_t i = 0; i < node.params().size(); ++i) {
                const bool param_last = (i == node.params().size() - 1);
                const auto &param = node.params()[i];
                print_value(ansi::green("Parameter '"), FORMAT("{}'", param.name), param_last);
                const IndentGuard guard3{*this, param_last};
                if(param.type_annotation) {
                    print_value(ansi::magenta("Type: "), FORMAT("{}", param.type_annotation), true);
                } else {
                    print_value(ansi::magenta("Type: "), "none", true);
                }
            }
        } else {
            print_line(ansi::green("Parameters: (none)"), !has_return);
        }

        // Return Type
        if(has_return) {
            print_line(ansi::magenta("Return Type:"), false);
            const IndentGuard guard2{*this, false};
            print_line(FORMAT("{}", *ret_type), true);
        }

        // Body
        print_line(ansi::blue("Body:"), true);
        {
            const IndentGuard guard2{*this, true};
            visit_child(node.body(), true);
        }
    }

    void AstPrinter::visit_ReturnStmt(const ReturnStmt &node) {
        const bool is_last = next_is_last_;
        print_line(ansi::yellow("Return"), is_last);

        if(node.has_value()) {
            const IndentGuard guard{*this, is_last};
            print_line(ansi::yellow("Value:"), true);
            const IndentGuard guard2{*this, true};
            visit_child(node.value(), true);
        }
    }

    void AstPrinter::visit_IfStmt(const IfStmt &node) {
        const bool is_last = next_is_last_;
        print_line(ansi::yellow("If"), is_last);
        const IndentGuard guard{*this, is_last};

        const bool has_else = node.has_else();

        print_line(ansi::yellow("Condition:"), false);
        {
            const IndentGuard guard2{*this, false};
            visit_child(node.condition(), true);
        }

        print_line(ansi::yellow("Then:"), !has_else);
        {
            const IndentGuard guard2{*this, !has_else};
            visit_child(node.then_branch(), true);
        }

        if(has_else) {
            print_line(ansi::yellow("Else:"), true);
            const IndentGuard guard2{*this, true};
            visit_child(node.else_branch(), true);
        }
    }

    void AstPrinter::visit_WhileStmt(const WhileStmt &node) {
        const bool is_last = next_is_last_;
        print_line(ansi::yellow("While"), is_last);
        const IndentGuard guard{*this, is_last};

        print_line(ansi::yellow("Condition:"), false);
        {
            const IndentGuard guard2{*this, false};
            visit_child(node.condition(), true);
        }

        print_line(ansi::yellow("Body:"), true);
        {
            const IndentGuard guard2{*this, true};
            visit_child(node.body(), true);
        }
    }

    void AstPrinter::visit_ForStmt(const ForStmt &node) {
        const bool is_last = next_is_last_;
        print_line(ansi::yellow("For"), is_last);
        const IndentGuard guard{*this, is_last};

        const bool has_init = node.has_init();
        const bool has_cond = node.has_condition();
        const bool has_incr = node.has_increment();

        if(has_init) {
            print_line(ansi::yellow("Init:"), false);
            const IndentGuard guard2{*this, false};
            visit_child(node.init(), true);
        } else {
            print_line(ansi::yellow("Init: (none)"), false);
        }

        if(has_cond) {
            print_line(ansi::yellow("Condition:"), false);
            const IndentGuard guard2{*this, false};
            visit_child(node.condition(), true);
        } else {
            print_line(ansi::yellow("Condition: (none)"), false);
        }

        if(has_incr) {
            print_line(ansi::yellow("Increment:"), false);
            const IndentGuard guard2{*this, false};
            visit_child(node.increment(), true);
        } else {
            print_line(ansi::yellow("Increment: (none)"), false);
        }

        print_line(ansi::yellow("Body:"), true);
        {
            const IndentGuard guard2{*this, true};
            visit_child(node.body(), true);
        }
    }

    void AstPrinter::visit_BlockStmt(const BlockStmt &node) {
        const bool is_last = next_is_last_;
        print_line(ansi::yellow("Block"), is_last);

        if(!node.statements().empty()) {
            const IndentGuard guard{*this, is_last};
            for(std::size_t i = 0; i < node.statements().size(); ++i) {
                const bool stmt_last = (i == node.statements().size() - 1);
                visit_child(*node.statements()[i], stmt_last);
            }
        }
    }

    // cppcheck-suppress functionConst
    void AstPrinter::visit_BreakStmt(const BreakStmt & /*unused*/) {
        const bool is_last = next_is_last_;
        print_line(ansi::yellow("Break"), is_last);
    }

    // cppcheck-suppress functionConst
    void AstPrinter::visit_ContinueStmt(const ContinueStmt & /*unused*/) {
        const bool is_last = next_is_last_;
        print_line(ansi::yellow("Continue"), is_last);
    }

    void AstPrinter::visit_MainStmt(const MainStmt &node) {
        const bool is_last = next_is_last_;
        print_line(ansi::yellow("Main"), is_last);
        const IndentGuard guard{*this, is_last};
        visit_child(node.expression(), true);
    }

    void AstPrinter::visit_Program(const Program &node) {
        // Program is the root — print without prefix
        fmt::println("{}", ansi::cyan_bold("Program"));

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

    std::string SExprPrinter::visit_CharLiteral(const CharLiteral &node) { return FORMAT("'{}'", node.value()); }

    std::string SExprPrinter::visit_BoolLiteral(const BoolLiteral &node) { return node.value() ? "true" : "false"; }

    std::string SExprPrinter::visit_NullLiteral(const NullLiteral & /*unused*/) { return "null"; }

    std::string SExprPrinter::visit_Identifier(const Identifier &node) { return node.name(); }

    std::string SExprPrinter::visit_UnaryExpr(const UnaryExpr &node) {
        const std::string_view op_str = unary_op_symbol(node.op());
        if(node.op() == UnaryOp::PreInc || node.op() == UnaryOp::PreDec) {
            return FORMAT("({} prefix {})", op_str, visit(node.operand()));
        } else if(node.op() == UnaryOp::PostInc || node.op() == UnaryOp::PostDec) {
            return FORMAT("({} postfix {})", op_str, visit(node.operand()));
        }
        return FORMAT("({} {})", op_str, visit(node.operand()));
    }

    std::string SExprPrinter::visit_BinaryExpr(const BinaryExpr &node) {
        return FORMAT("({} {} {})", binary_op_symbol(node.op()), visit(node.lhs()), visit(node.rhs()));
    }

    std::string SExprPrinter::visit_TernaryExpr(const TernaryExpr &node) {
        return FORMAT("(?: {} {} {})", visit(node.condition()), visit(node.then_expr()), visit(node.else_expr()));
    }

    std::string SExprPrinter::visit_CallExpr(const CallExpr &node) {
        std::string result;
        const auto callee_str = visit(node.callee());
        // PERF: reserve estimated size to avoid reallocations
        const std::size_t estimated_size = 8 + callee_str.size() + (node.args().size() * 16);
        result.reserve(estimated_size);
        auto out = std::back_inserter(result);
        FORMAT_TO(out, "(call {}", callee_str);
        for(const auto &arg : node.args()) { FORMAT_TO(out, " {}", visit(*arg)); }
        FORMAT_TO(out, ")");
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
        std::string result;
        // PERF: reserve estimated size to avoid reallocations
        const std::size_t estimated_size = 2 + (node.elements().size() * 12);
        result.reserve(estimated_size);
        auto out = std::back_inserter(result);
        FORMAT_TO(out, "[");
        for(std::size_t i = 0; i < node.elements().size(); ++i) {
            if(i > 0) { FORMAT_TO(out, " "); }
            FORMAT_TO(out, "{}", visit(*node.elements()[i]));
        }
        FORMAT_TO(out, "]");
        return result;
    }

    std::string SExprPrinter::visit_GroupingExpr(const GroupingExpr &node) { return FORMAT("(group {})", visit(node.expression())); }

    // Statements
    std::string SExprPrinter::visit_ExprStmt(const ExprStmt &node) { return FORMAT("(expr-stmt {})", visit(node.expression())); }

    std::string SExprPrinter::visit_VarDecl(const VarDecl &node) {
        // Multi-variable declaration: var a, b: i64 = 10, 20;
        if(node.num_variables() > 1) {
            std::string result;
            // PERF: reserve estimated size to avoid reallocations
            const std::size_t estimated_size = 16 + (node.names().size() * 8) + (node.initializers().size() * 16);
            result.reserve(estimated_size);
            auto out = std::back_inserter(result);
            FORMAT_TO(out, "({}", node.is_const() ? "const" : "var");
            for(const auto &varname : node.names()) { FORMAT_TO(out, " {}", varname); }
            if(const auto &type_ann = node.type_annotation(); type_ann.has_value()) { FORMAT_TO(out, " : {}", *type_ann); }
            if(!node.initializers().empty()) {
                FORMAT_TO(out, " (");
                for(std::size_t i = 0; i < node.initializers().size(); ++i) {
                    if(i > 0) { FORMAT_TO(out, " "); }
                    FORMAT_TO(out, "{}", node.initializers()[i] ? visit(*node.initializers()[i]) : "null");
                }
                FORMAT_TO(out, ")");
            }
            FORMAT_TO(out, ")");
            return result;
        }

        // Single variable declaration (backward compatible)
        std::string result;
        // PERF: reserve estimated size to avoid reallocations
        const std::size_t estimated_size = 32 + node.name().size() +
                                           (node.type_annotation().has_value() ? node.type_annotation().value().size() : 0);
        result.reserve(estimated_size);
        auto out = std::back_inserter(result);
        FORMAT_TO(out, "({} {}", node.is_const() ? "const" : "var", node.name());
        if(const auto &type_ann = node.type_annotation(); type_ann.has_value()) { FORMAT_TO(out, " : {}", *type_ann); }
        if(node.has_initializer()) { FORMAT_TO(out, " {}", visit(node.initializer())); }
        FORMAT_TO(out, ")");
        return result;
    }

    std::string SExprPrinter::visit_FuncDecl(const FuncDecl &node) {
        std::string result;
        // PERF: reserve estimated size to avoid reallocations
        const std::size_t estimated_size = 64 + (node.params().size() * 24) + (node.return_type().has_value() ? 16 : 0);
        result.reserve(estimated_size);
        auto out = std::back_inserter(result);
        FORMAT_TO(out, "(fn {} (", node.name());
        for(std::size_t i = 0; i < node.params().size(); ++i) {
            if(i > 0) { FORMAT_TO(out, " "); }
            const auto &param = node.params()[i];
            if(param.type_annotation) {
                FORMAT_TO(out, "({} {})", param.name, param.type_annotation);
            } else {
                FORMAT_TO(out, "({})", param.name);
            }
        }
        FORMAT_TO(out, ")");
        if(const auto &ret_type = node.return_type(); ret_type.has_value()) { FORMAT_TO(out, " -> {}", *ret_type); }
        FORMAT_TO(out, " {})", visit_BlockStmt(node.body()));
        return result;
    }

    std::string SExprPrinter::visit_ReturnStmt(const ReturnStmt &node) {
        if(node.has_value()) { return FORMAT("(return {})", visit(node.value())); }
        return "(return)";
    }

    std::string SExprPrinter::visit_IfStmt(const IfStmt &node) {
        std::string result;
        auto out = std::back_inserter(result);
        FORMAT_TO(out, "(if {} {}", visit(node.condition()), visit(node.then_branch()));
        if(node.has_else()) { FORMAT_TO(out, " {}", visit(node.else_branch())); }
        FORMAT_TO(out, ")");
        return result;
    }

    std::string SExprPrinter::visit_WhileStmt(const WhileStmt &node) {
        return FORMAT("(while {} {})", visit(node.condition()), visit(node.body()));
    }

    std::string SExprPrinter::visit_ForStmt(const ForStmt &node) {
        std::string result;
        auto out = std::back_inserter(result);
        FORMAT_TO(out, "(for ");
        FORMAT_TO(out, "{}", node.has_init() ? visit(node.init()) : "_");
        FORMAT_TO(out, " ");
        FORMAT_TO(out, "{}", node.has_condition() ? visit(node.condition()) : "_");
        FORMAT_TO(out, " ");
        FORMAT_TO(out, "{}", node.has_increment() ? visit(node.increment()) : "_");
        FORMAT_TO(out, " {})", visit(node.body()));
        return result;
    }

    std::string SExprPrinter::visit_BlockStmt(const BlockStmt &node) {
        std::string result;
        auto out = std::back_inserter(result);
        FORMAT_TO(out, "(block");
        for(const auto &stmt : node.statements()) { FORMAT_TO(out, " {}", visit(*stmt)); }
        FORMAT_TO(out, ")");
        return result;
    }

    std::string SExprPrinter::visit_BreakStmt(const BreakStmt & /*unused*/) { return "(break)"; }

    std::string SExprPrinter::visit_ContinueStmt(const ContinueStmt & /*unused*/) { return "(continue)"; }

    std::string SExprPrinter::visit_MainStmt(const MainStmt &node) { return FORMAT("(main {})", visit(node.expression())); }

    std::string SExprPrinter::visit_Program(const Program &node) {
        std::string result;
        auto out = std::back_inserter(result);
        FORMAT_TO(out, "(program");
        for(const auto &stmt : node.statements()) { FORMAT_TO(out, " {}", visit(*stmt)); }
        FORMAT_TO(out, ")");
        return result;
    }

}  // namespace jsv
// NOLINTEND(*-include-cleaner)
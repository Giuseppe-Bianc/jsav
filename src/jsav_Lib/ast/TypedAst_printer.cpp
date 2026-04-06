/*
 * Created by gbian on 06/04/2026.
 * Copyright (c) 2026 All rights reserved.
 */

// NOLINTBEGIN(*-include-cleaner)
#include "jsav/ast/TypedAst_printer.hpp"
#include "jsav/util/AnsiStyles.hpp"

namespace jsv {

    // ============================================================
    // TypedAstPrinter - Core methods
    // ============================================================

    void TypedAstPrinter::print(const TypedNode &node) {
        prefix_stack_.clear();
        next_is_last_ = true;
        visit(node);
    }

    void TypedAstPrinter::print_prefix() const {
        for(const bool is_last : prefix_stack_) { fmt::print("{}", is_last ? "    " : "│   "); }
    }

    void TypedAstPrinter::print_line(std::string_view msg, bool is_last) const {
        print_prefix();
        fmt::println("{}{}", is_last ? "└── " : "├── ", msg);
    }

    void TypedAstPrinter::print_value(std::string_view label, std::string_view value, bool is_last) const {
        print_prefix();
        fmt::println("{}{}{}", is_last ? "└── " : "├── ", label, value);
    }

    void TypedAstPrinter::print_type_annotation(const TypePtr &type, bool is_last) const {
        print_value(ansi::magenta("Type: "), type ? type->to_string() : "none", is_last);
    }

    // ============================================================
    // Expressions
    // ============================================================

    // cppcheck-suppress functionConst
    void TypedAstPrinter::visit_IntegerLiteral(const TypedIntegerLiteral &node) {
        const bool is_last = next_is_last_;
        std::string value_str;
        if(const auto &type_suffix = node.type_suffix(); type_suffix.has_value()) {
            value_str = FORMAT("{}{}", node.value(), *type_suffix);
        } else {
            value_str = FORMAT("{}", node.value());
        }
        print_value(ansi::green("TypedLiteral "), FORMAT("{} [{}]", value_str, node.node_type() ? node.node_type()->to_string() : "none"), is_last);
    }

    // cppcheck-suppress functionConst
    void TypedAstPrinter::visit_FloatLiteral(const TypedFloatLiteral &node) {
        const bool is_last = next_is_last_;
        print_value(ansi::green("TypedLiteral "), FORMAT("{}f [{}]", node.value(), node.node_type() ? node.node_type()->to_string() : "none"), is_last);
    }

    // cppcheck-suppress functionConst
    void TypedAstPrinter::visit_StringLiteral(const TypedStringLiteral &node) {
        const bool is_last = next_is_last_;
        print_value(ansi::green("TypedLiteral "), FORMAT("\"{}\" [{}]", node.value(), node.node_type() ? node.node_type()->to_string() : "none"), is_last);
    }

    // cppcheck-suppress functionConst
    void TypedAstPrinter::visit_CharLiteral(const TypedCharLiteral &node) {
        const bool is_last = next_is_last_;
        print_value(ansi::green("TypedLiteral "), FORMAT("'{}' [{}]", node.value(), node.node_type() ? node.node_type()->to_string() : "none"), is_last);
    }

    // cppcheck-suppress functionConst
    void TypedAstPrinter::visit_BoolLiteral(const TypedBoolLiteral &node) {
        const bool is_last = next_is_last_;
        print_value(ansi::green("TypedLiteral "), FORMAT("{} [{}]", node.value() ? "true" : "false", node.node_type() ? node.node_type()->to_string() : "none"), is_last);
    }

    // cppcheck-suppress functionConst
    void TypedAstPrinter::visit_NullLiteral(const TypedNullLiteral &node) {
        const bool is_last = next_is_last_;
        print_value(ansi::green("TypedLiteral null"), FORMAT(" [{}]", node.node_type() ? node.node_type()->to_string() : "none"), is_last);
    }

    // cppcheck-suppress functionConst
    void TypedAstPrinter::visit_Identifier(const TypedIdentifier &node) {
        const bool is_last = next_is_last_;
        print_value(ansi::blue("TypedIdentifier "), FORMAT("{} [{}]", node.name(), node.node_type() ? node.node_type()->to_string() : "none"), is_last);
    }

    void TypedAstPrinter::visit_UnaryExpr(const TypedUnaryExpr &node) {
        const bool is_last = next_is_last_;
        const std::string_view op_str = unary_op_symbol(node.op());
        std::string_view position;
        if(node.op() == UnaryOp::PreInc || node.op() == UnaryOp::PreDec) {
            position = " (prefix)";
        } else if(node.op() == UnaryOp::PostInc || node.op() == UnaryOp::PostDec) {
            position = " (postfix)";
        }
        print_value(ansi::cyan("TypedUnaryExpr "), FORMAT("'{}'{} [{}]", op_str, position, node.node_type() ? node.node_type()->to_string() : "none"), is_last);
        const IndentGuard guard{*this, is_last};

        print_line(ansi::cyan("Operand:"), true);
        {
            const IndentGuard guard2{*this, true};
            visit_child(node.operand(), true);
        }
    }

    void TypedAstPrinter::visit_BinaryExpr(const TypedBinaryExpr &node) {
        const bool is_last = next_is_last_;
        print_value(ansi::cyan("TypedBinaryExpr "), FORMAT("'{}' [{}]", binary_op_symbol(node.op()), node.node_type() ? node.node_type()->to_string() : "none"), is_last);
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

    void TypedAstPrinter::visit_TernaryExpr(const TypedTernaryExpr &node) {
        const bool is_last = next_is_last_;
        print_value(ansi::cyan("TypedTernaryExpr"), FORMAT(" [{}]", node.node_type() ? node.node_type()->to_string() : "none"), is_last);
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

    void TypedAstPrinter::visit_CallExpr(const TypedCallExpr &node) {
        const bool is_last = next_is_last_;
        print_value(ansi::cyan("TypedCallExpr"), FORMAT(" [{}]", node.node_type() ? node.node_type()->to_string() : "none"), is_last);
        const IndentGuard guard{*this, is_last};

        print_line(ansi::red("Callee:"), false);
        {
            const IndentGuard guard2{*this, false};
            visit_child(node.callee(), true);
        }

        print_value(ansi::green("Arguments: "), FORMAT("({})", node.args().size()), true);
        if(!node.args().empty()) {
            const IndentGuard guard2{*this, true};
            const auto &args = node.args();
            for(const auto &[i, arg] : std::views::enumerate(args)) {
                visit_child(*arg, i == std::ssize(args) - 1);
            }
        }
    }

    void TypedAstPrinter::visit_IndexExpr(const TypedIndexExpr &node) {
        const bool is_last = next_is_last_;
        print_value(ansi::cyan("TypedIndexExpr"), FORMAT(" [{}]", node.node_type() ? node.node_type()->to_string() : "none"), is_last);
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

    void TypedAstPrinter::visit_MemberExpr(const TypedMemberExpr &node) {
        const bool is_last = next_is_last_;
        print_value(ansi::cyan("TypedMemberExpr "), FORMAT(".{} [{}]", node.member(), node.node_type() ? node.node_type()->to_string() : "none"), is_last);
        const IndentGuard guard{*this, is_last};

        print_line(ansi::blue("Object:"), true);
        {
            const IndentGuard guard2{*this, true};
            visit_child(node.object(), true);
        }
    }

    void TypedAstPrinter::visit_AssignExpr(const TypedAssignExpr &node) {
        const bool is_last = next_is_last_;
        print_value(ansi::cyan("TypedAssignExpr"), FORMAT(" [{}]", node.node_type() ? node.node_type()->to_string() : "none"), is_last);
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

    void TypedAstPrinter::visit_CastExpr(const TypedCastExpr &node) {
        const bool is_last = next_is_last_;
        print_value(ansi::cyan("TypedCastExpr "), FORMAT("-> {} [{}]", node.target_type(), node.node_type() ? node.node_type()->to_string() : "none"), is_last);
        const IndentGuard guard{*this, is_last};

        print_line(ansi::green("Operand:"), true);
        {
            const IndentGuard guard2{*this, true};
            visit_child(node.operand(), true);
        }
    }

    void TypedAstPrinter::visit_ArrayLiteral(const TypedArrayLiteral &node) {
        const bool is_last = next_is_last_;
        print_value(ansi::cyan("TypedArray Literal"), FORMAT(" [{}]", node.node_type() ? node.node_type()->to_string() : "none"), is_last);

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

    void TypedAstPrinter::visit_GroupingExpr(const TypedGroupingExpr &node) {
        const bool is_last = next_is_last_;
        print_value(ansi::cyan("TypedGroupingExpr"), FORMAT(" [{}]", node.node_type() ? node.node_type()->to_string() : "none"), is_last);
        const IndentGuard guard{*this, is_last};
        visit_child(node.expression(), true);
    }

    // ============================================================
    // Statements
    // ============================================================

    void TypedAstPrinter::visit_ExprStmt(const TypedExprStmt &node) {
        const bool is_last = next_is_last_;
        print_value(ansi::yellow("TypedExprStmt"), FORMAT(" [{}]", node.node_type() ? node.node_type()->to_string() : "none"), is_last);
        const IndentGuard guard{*this, is_last};
        visit_child(node.expression(), true);
    }

    void TypedAstPrinter::visit_VarDecl(const TypedVarDecl &node) {
        const bool is_last = next_is_last_;
        const std::string keyword = node.is_const() ? ansi::blue_bold("TypedConstDeclaration") : ansi::blue("TypedVarDeclaration");

        print_value(keyword, FORMAT(" {} [{}]", node.name(), node.node_type() ? node.node_type()->to_string() : "none"), is_last);
        const IndentGuard guard{*this, is_last};

        // Initializer
        if(node.has_initializer()) {
            print_line(ansi::red("Initializer:"), true);
            const IndentGuard guard2{*this, true};
            visit_child(node.initializer(), true);
        }
    }

    void TypedAstPrinter::visit_FuncDecl(const TypedFuncDecl &node) {
        const bool is_last = next_is_last_;
        print_value(ansi::blue_bold("TypedFunction"), FORMAT(" {} [{}]", node.name(), node.node_type() ? node.node_type()->to_string() : "none"), is_last);
        const IndentGuard guard{*this, is_last};

        const bool has_params = !node.params().empty();
        const auto &ret_type = node.return_type();
        const bool has_return = ret_type.has_value();

        // Parameters
        if(has_params) {
            print_line(ansi::green("Parameters:"), !has_return);
            const IndentGuard guard2{*this, !has_return};
            for(std::size_t i = 0; i < node.params().size(); ++i) {
                const bool param_last = (i == node.params().size() - 1);
                const auto &param = node.params()[i];
                print_value(ansi::green("TypedParameter '"), FORMAT("{}'", param.name), param_last);
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

    void TypedAstPrinter::visit_ReturnStmt(const TypedReturnStmt &node) {
        const bool is_last = next_is_last_;
        print_value(ansi::yellow("TypedReturn"), FORMAT(" [{}]", node.node_type() ? node.node_type()->to_string() : "none"), is_last);

        if(node.has_value()) {
            const IndentGuard guard{*this, is_last};
            print_line(ansi::yellow("Value:"), true);
            const IndentGuard guard2{*this, true};
            visit_child(node.value(), true);
        }
    }

    void TypedAstPrinter::visit_IfStmt(const TypedIfStmt &node) {
        const bool is_last = next_is_last_;
        print_value(ansi::yellow("TypedIf"), FORMAT(" [{}]", node.node_type() ? node.node_type()->to_string() : "none"), is_last);
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

    void TypedAstPrinter::visit_WhileStmt(const TypedWhileStmt &node) {
        const bool is_last = next_is_last_;
        print_value(ansi::yellow("TypedWhile"), FORMAT(" [{}]", node.node_type() ? node.node_type()->to_string() : "none"), is_last);
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

    void TypedAstPrinter::visit_ForStmt(const TypedForStmt &node) {
        const bool is_last = next_is_last_;
        print_value(ansi::yellow("TypedFor"), FORMAT(" [{}]", node.node_type() ? node.node_type()->to_string() : "none"), is_last);
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

    void TypedAstPrinter::visit_BlockStmt(const TypedBlockStmt &node) {
        const bool is_last = next_is_last_;
        print_value(ansi::yellow("TypedBlock"), FORMAT(" [{}]", node.node_type() ? node.node_type()->to_string() : "none"), is_last);

        if(!node.statements().empty()) {
            const IndentGuard guard{*this, is_last};
            for(std::size_t i = 0; i < node.statements().size(); ++i) {
                const bool stmt_last = (i == node.statements().size() - 1);
                visit_child(*node.statements()[i], stmt_last);
            }
        }
    }

    // cppcheck-suppress functionConst
    void TypedAstPrinter::visit_BreakStmt(const TypedBreakStmt &node) {
        const bool is_last = next_is_last_;
        print_value(ansi::yellow("TypedBreak"), FORMAT(" [{}]", node.node_type() ? node.node_type()->to_string() : "none"), is_last);
    }

    // cppcheck-suppress functionConst
    void TypedAstPrinter::visit_ContinueStmt(const TypedContinueStmt &node) {
        const bool is_last = next_is_last_;
        print_value(ansi::yellow("TypedContinue"), FORMAT(" [{}]", node.node_type() ? node.node_type()->to_string() : "none"), is_last);
    }

    void TypedAstPrinter::visit_MainStmt(const TypedMainStmt &node) {
        const bool is_last = next_is_last_;
        print_value(ansi::yellow("TypedMain"), FORMAT(" [{}]", node.node_type() ? node.node_type()->to_string() : "none"), is_last);
        const IndentGuard guard{*this, is_last};
        visit_child(node.body(), true);
    }

    void TypedAstPrinter::visit_Program(const TypedProgram &node) {
        // Program is the root — print without prefix
        fmt::println("{} [{}]", ansi::cyan_bold("TypedProgram"), node.node_type() ? node.node_type()->to_string() : "none");

        if(!node.statements().empty()) {
            for(std::size_t i = 0; i < node.statements().size(); ++i) {
                const bool stmt_last = (i == node.statements().size() - 1);
                visit_child(*node.statements()[i], stmt_last);
            }
        }
    }

}  // namespace jsv
// NOLINTEND(*-include-cleaner)

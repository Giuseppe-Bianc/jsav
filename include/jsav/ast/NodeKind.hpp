/*
 * Created by gbian on 17/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once
#include "jsav/headers.hpp"

namespace jsv {
    // ============================================================
    // Tipo di nodo: ogni nodo concreto ha un tag univoco
    // ============================================================
    enum class NodeKind : std::uint8_t {
        // Expressions
        IntegerLiteral,
        FloatLiteral,
        StringLiteral,
        BoolLiteral,
        NullLiteral,
        Identifier,
        UnaryExpr,
        BinaryExpr,
        TernaryExpr,
        CallExpr,
        IndexExpr,   // array[index]
        MemberExpr,  // obj.field
        AssignExpr,
        CastExpr,
        ArrayLiteral,
        GroupingExpr,  // (expr)

        // Statements
        ExprStmt,
        VarDecl,
        FuncDecl,
        ReturnStmt,
        IfStmt,
        WhileStmt,
        ForStmt,
        BlockStmt,
        BreakStmt,
        ContinueStmt,
        MainStmt,

        // Top-level
        Program,
    };

    [[nodiscard]] std::string_view node_kind_name(NodeKind kind);

    // ============================================================
    // Operatori
    // ============================================================
    enum class UnaryOp : std::uint8_t {
        Negate,   // -
        Not,      // !
        BitNot,   // ~
        PreInc,   // ++x
        PreDec,   // --x
        PostInc,  // x++
        PostDec,  // x--
    };

    [[nodiscard]] std::string_view unary_op_symbol(UnaryOp opcode);

    enum class BinaryOp : std::uint8_t {
        Add,
        Sub,
        Mul,
        Div,
        Mod,
        Eq,
        Neq,
        Lt,
        Gt,
        Le,
        Ge,
        And,
        Or,
        BitAnd,
        BitOr,
        BitXor,
        Shl,
        Shr,
    };

    [[nodiscard]] std::string_view binary_op_symbol(BinaryOp opcode);

}  // namespace jsv
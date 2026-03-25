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
        CharLiteral,
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

    [[nodiscard]] constexpr std::string_view node_kind_name(NodeKind kind) {
        switch(kind) {
        case NodeKind::IntegerLiteral:
            return "IntegerLiteral";
        case NodeKind::FloatLiteral:
            return "FloatLiteral";
        case NodeKind::StringLiteral:
            return "StringLiteral";
        case NodeKind::CharLiteral:
            return "CharLiteral";
        case NodeKind::BoolLiteral:
            return "BoolLiteral";
        case NodeKind::NullLiteral:
            return "NullLiteral";
        case NodeKind::Identifier:
            return "Identifier";
        case NodeKind::UnaryExpr:
            return "UnaryExpr";
        case NodeKind::BinaryExpr:
            return "BinaryExpr";
        case NodeKind::TernaryExpr:
            return "TernaryExpr";
        case NodeKind::CallExpr:
            return "CallExpr";
        case NodeKind::IndexExpr:
            return "IndexExpr";
        case NodeKind::MemberExpr:
            return "MemberExpr";
        case NodeKind::AssignExpr:
            return "AssignExpr";
        case NodeKind::CastExpr:
            return "CastExpr";
        case NodeKind::ArrayLiteral:
            return "ArrayLiteral";
        case NodeKind::GroupingExpr:
            return "GroupingExpr";
        case NodeKind::ExprStmt:
            return "ExprStmt";
        case NodeKind::VarDecl:
            return "VarDecl";
        case NodeKind::FuncDecl:
            return "FuncDecl";
        case NodeKind::ReturnStmt:
            return "ReturnStmt";
        case NodeKind::IfStmt:
            return "IfStmt";
        case NodeKind::WhileStmt:
            return "WhileStmt";
        case NodeKind::ForStmt:
            return "ForStmt";
        case NodeKind::BlockStmt:
            return "BlockStmt";
        case NodeKind::BreakStmt:
            return "BreakStmt";
        case NodeKind::ContinueStmt:
            return "ContinueStmt";
        case NodeKind::MainStmt:
            return "MainStmt";
        case NodeKind::Program:
            return "Program";
        }
        return "Unknown";
    }

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

    [[nodiscard]] constexpr std::string_view unary_op_symbol(UnaryOp opcode) {
        switch(opcode) {
        case UnaryOp::Negate:
            return "-";
        case UnaryOp::Not:
            return "!";
        case UnaryOp::BitNot:
            return "~";
        case UnaryOp::PreInc:
            return "++";
        case UnaryOp::PreDec:
            return "--";
        case UnaryOp::PostInc:
            return "++";
        case UnaryOp::PostDec:
            return "--";
        }
        return "?";
    }

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

    [[nodiscard]] constexpr std::string_view binary_op_symbol(BinaryOp opcode) {
        switch(opcode) {
        case BinaryOp::Add:
            return "+";
        case BinaryOp::Sub:
            return "-";
        case BinaryOp::Mul:
            return "*";
        case BinaryOp::Div:
            return "/";
        case BinaryOp::Mod:
            return "%";
        case BinaryOp::Eq:
            return "==";
        case BinaryOp::Neq:
            return "!=";
        case BinaryOp::Lt:
            return "<";
        case BinaryOp::Gt:
            return ">";
        case BinaryOp::Le:
            return "<=";
        case BinaryOp::Ge:
            return ">=";
        case BinaryOp::And:
            return "&&";
        case BinaryOp::Or:
            return "||";
        case BinaryOp::BitAnd:
            return "&";
        case BinaryOp::BitOr:
            return "|";
        case BinaryOp::BitXor:
            return "^";
        case BinaryOp::Shl:
            return "<<";
        case BinaryOp::Shr:
            return ">>";
        }
        return "?";
    }

}  // namespace jsv
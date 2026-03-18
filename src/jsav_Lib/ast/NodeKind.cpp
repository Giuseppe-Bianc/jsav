/*
 * Created by gbian on 17/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#include "jsav/ast/NodeKind.hpp"

namespace jsv {

    std::string_view node_kind_name(NodeKind kind) {
        switch(kind) {
        case NodeKind::IntegerLiteral:
            return "IntegerLiteral";
        case NodeKind::FloatLiteral:
            return "FloatLiteral";
        case NodeKind::StringLiteral:
            return "StringLiteral";
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
        case NodeKind::PrintStmt:
            return "PrintStmt";
        case NodeKind::Program:
            return "Program";
        }
        return "Unknown";
    }

    std::string_view unary_op_symbol(UnaryOp op) {
        switch(op) {
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

    std::string_view binary_op_symbol(BinaryOp op) {
        switch(op) {
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
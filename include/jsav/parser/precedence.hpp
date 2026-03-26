/*
 * Created by gbian on 23/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

#include "../ast/NodeKind.hpp"
#include "../error/CompileError.hpp"
#include "../lexer/Token.hpp"

namespace jsv {

    /**
     * @brief Get the binding power for a token (Pratt parsing).
     *
     * Returns left and right binding powers used for operator precedence parsing.
     * Higher values indicate higher precedence. Right > Left creates left-associativity.
     *
     * @param token The token to get binding power for.
     * @return std::pair<std::size_t, std::size_t> {lbp, rbp} where lbp is left binding power
     *         and rbp is right binding power. Returns {0, 0} for non-operators.
     *
     * @code
     * auto [lbp, rbp] = bindingPower(plusToken);  // Returns {17, 18}
     * @endcode
     */
    [[nodiscard]] inline std::pair<std::size_t, std::size_t> binding_power(const Token &token) {
        switch(token.getKind()) {
        case TokenKind::OrOr:  // ||
            return {1, 2};
        case TokenKind::AndAnd:  // &&
            return {3, 4};
        case TokenKind::Or:  // |
            return {5, 6};
        case TokenKind::Xor:  // ^
            return {7, 8};
        case TokenKind::And:  // &
            return {9, 10};
        case TokenKind::EqualEqual:  // ==
        case TokenKind::NotEqual:    // !=
            return {11, 12};
        case TokenKind::Less:          // <
        case TokenKind::LessEqual:     // <=
        case TokenKind::Greater:       // >
        case TokenKind::GreaterEqual:  // >=
            return {13, 14};
        case TokenKind::ShiftLeft:   // <<
        case TokenKind::ShiftRight:  // >>
            return {15, 16};
        case TokenKind::Plus:   // +
        case TokenKind::Minus:  // -
            return {17, 18};
        case TokenKind::Star:     // *
        case TokenKind::Slash:    // /
        case TokenKind::Percent:  // %
            return {19, 20};
        case TokenKind::Equal:  // = (assignment)
            return {21, 22};
        case TokenKind::PlusPlus:    // ++ (postfix)
        case TokenKind::MinusMinus:  // -- (postfix)
            return {23, 24};
        default:
            return {0, 0};  // Not an operator
        }
    }

    /**
     * @brief Get the binding power for unary prefix operators.
     *
     * For unary operators, lbp is always 0 (they don't have a left operand).
     * The rbp determines how tightly the operator binds to its operand.
     *
     * @param token The token to get unary binding power for.
     * @return std::pair<std::size_t, std::size_t> {0, rbp} for unary operators,
     *         {0, 0} for non-unary operators.
     *
     * @code
     * auto [lbp, rbp] = unaryBindingPower(minusToken);  // Returns {0, 22}
     * @endcode
     */
    [[nodiscard]] inline std::pair<std::size_t, std::size_t> unary_binding_power(const Token &token) {
        switch(token.getKind()) {
        case TokenKind::Minus:  // - (Negate)
            return {0, 22};
        case TokenKind::Not:  // ! (Not)
            return {0, 21};
        // TODO: Add bitwise NOT when TokenKind::Tilde is implemented
        // case TokenKind::Tilde:         // ~ (BitNot)
        //     return {0, 23};
        case TokenKind::PlusPlus:  // ++ (PreInc/PostInc)
            return {0, 24};
        case TokenKind::MinusMinus:  // -- (PreDec/PostDec)
            return {0, 25};
        default:
            return {0, 0};  // Not a unary operator
        }
    }

    /**
     * @brief Convert a token to its corresponding BinaryOp enum value.
     *
     * Maps operator tokens to the BinaryOp enum for use in AST construction.
     * Returns an error if the token is not a valid binary operator.
     *
     * @param token The token to convert.
     * @return std::expected<BinaryOp, CompileError> The BinaryOp on success,
     *         or a SyntaxError if the token is not a binary operator.
     *
     * @code
     * auto result = get_binary_op(plusToken);
     * if (result) {
     *     BinaryOp op = *result;  // BinaryOp::Add
     * }
     * @endcode
     */
    [[nodiscard]] inline std::expected<BinaryOp, CompileError> get_binary_op(const Token &token) {
        switch(token.getKind()) {
        case TokenKind::Plus:
            return BinaryOp::Add;
        case TokenKind::Minus:
            return BinaryOp::Sub;
        case TokenKind::Star:
            return BinaryOp::Mul;
        case TokenKind::Slash:
            return BinaryOp::Div;
        case TokenKind::Percent:
            return BinaryOp::Mod;
        case TokenKind::EqualEqual:
            return BinaryOp::Eq;
        case TokenKind::NotEqual:
            return BinaryOp::Neq;
        case TokenKind::Less:
            return BinaryOp::Lt;
        case TokenKind::LessEqual:
            return BinaryOp::Le;
        case TokenKind::Greater:
            return BinaryOp::Gt;
        case TokenKind::GreaterEqual:
            return BinaryOp::Ge;
        case TokenKind::AndAnd:
            return BinaryOp::And;
        case TokenKind::OrOr:
            return BinaryOp::Or;
        case TokenKind::And:
            return BinaryOp::BitAnd;
        case TokenKind::Or:
            return BinaryOp::BitOr;
        case TokenKind::Xor:
            return BinaryOp::BitXor;
        case TokenKind::ShiftLeft:
            return BinaryOp::Shl;
        case TokenKind::ShiftRight:
            return BinaryOp::Shr;
        default:
            {
                return std::unexpected(CompileError::SyntaxError(ErrorCode::E1005, "Invalid binary operator", token.getSpan(),
                                                                 "This token cannot be used as a binary operator"));
            }
        }
    }

}  // namespace jsv

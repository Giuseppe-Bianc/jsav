/*
 * Created by gbian on 23/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

#include "../ast/Expressions.hpp"
#include "../ast/Node.hpp"
#include "../ast/Program.hpp"
#include "../ast/Statements.hpp"
#include "../ast/Type.hpp"
#include "../error/CompileError.hpp"
#include "../headers.hpp"
#include "../lexer/Token.hpp"

namespace jsv {

    class Parser {
    public:
        explicit Parser(std::span<const Token> tokens);
        [[nodiscard]] std::optional<ExprPtr> parse_condition(std::string_view keyword);
        [[nodiscard]] std::optional<StmtPtr> parse_for_initializer();
        [[nodiscard]] std::optional<ExprPtr> parse_identifier_or_call(const Token &token);
        [[nodiscard]] std::pair<std::optional<TypePtr>, std::optional<std::string>> parse_type();
        [[nodiscard]] std::optional<StmtPtr> parse_function();
        [[nodiscard]] std::optional<StmtPtr> parse_main_function();
        [[nodiscard]] std::optional<StmtPtr> parse_if();
        [[nodiscard]] std::optional<StmtPtr> parse_var_declaration();
        [[nodiscard]] std::optional<StmtPtr> parse_return();
        [[nodiscard]] std::optional<StmtPtr> parse_while();
        [[nodiscard]] std::optional<StmtPtr> parse_for();
        [[nodiscard]] std::optional<StmtPtr> parse_break();
        [[nodiscard]] std::optional<StmtPtr> parse_continue();
        [[nodiscard]] std::optional<StmtPtr> parse_block_stmt();
        [[nodiscard]] std::optional<StmtPtr> parse_expression_stmt();
        [[nodiscard]] std::optional<StmtPtr> parse_stmt();
        [[nodiscard]] std::pair<std::unique_ptr<Program>, std::vector<CompileError>> parse();

        // Expression parsing (Pratt parser)
        [[nodiscard]] std::optional<ExprPtr> parse_expr(std::size_t min_bp);
        [[nodiscard]] std::optional<ExprPtr> parse_expr_inner(std::size_t min_bp);
        [[nodiscard]] std::optional<ExprPtr> nud();
        [[nodiscard]] std::optional<ExprPtr> led(ExprPtr left);

        // Expression helpers
        void extract_elements(TokenKind kind, std::vector<ExprPtr> &elements);
        [[nodiscard]] ExprPtr parse_unary(UnaryOp operation, const Token &token);
        [[nodiscard]] static ExprPtr parse_postfix_unary(ExprPtr operand, UnaryOp operation, const Token &token);
        [[nodiscard]] std::optional<ExprPtr> parse_array_literal(const Token &start_token);
        [[nodiscard]] std::optional<ExprPtr> parse_binary(ExprPtr left, const Token &token);
        [[nodiscard]] std::optional<ExprPtr> parse_grouping(const Token &start_token);
        [[nodiscard]] std::optional<ExprPtr> parse_assignment(ExprPtr left, const Token &token);
        [[nodiscard]] std::optional<ExprPtr> parse_call(ExprPtr callee, const Token &start_token);
        [[nodiscard]] std::optional<ExprPtr> parse_array_access(ExprPtr array, const Token &start_token);
        [[nodiscard]] const Token &peek() const noexcept;
        [[nodiscard]] const Token &previous() const noexcept;
        const Token &advance() noexcept;
        [[nodiscard]] bool is_end_of_statement() const noexcept;
        [[nodiscard]] bool check(TokenKind kind) const noexcept;
        [[nodiscard]] bool is_at_end() const noexcept;
        bool match_token(TokenKind kind) noexcept;
        [[nodiscard]] bool expect(TokenKind kind, std::string_view context);
        void syntax_error(std::string_view message, const Token &token, std::optional<std::string> help,
                          std::optional<ErrorCode> error_code);
        void report_peek_error(std::string_view message, const std::optional<std::string> &help);
        [[nodiscard]] static SourceSpan calculate_return_span(const Token &start, const std::optional<ExprPtr> &value);

        // Recursion guard
        [[nodiscard]] bool check_recursion_limit();
        void enter_recursion() noexcept;
        void exit_recursion() noexcept;
        [[nodiscard]] SourceSpan merged_span(const Token &start_token) const noexcept;
        [[nodiscard]] std::optional<std::string_view> consume_identifier();

    private:
        std::span<const Token> tokens_;
        std::size_t current_{0};
        std::size_t recursion_depth_{0};
        std::vector<CompileError> errors_;
        static constexpr std::size_t MAX_RECURSION_DEPTH{1000};
    };

}  // namespace jsv


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
        explicit Parser(const std::vector<Token> &tokens);
        std::optional<ExprPtr> parse_condition(const std::string_view keyword);
        std::optional<StmtPtr> parse_for_initializer();
        [[nodiscard]] std::optional<ExprPtr> parse_identifier_or_call(const Token &token);
        std::pair<std::optional<TypePtr>, std::optional<std::string>> parse_type();
        std::optional<StmtPtr> parse_function();
        std::optional<StmtPtr> parse_main_function();
        std::optional<StmtPtr> parse_if();
        std::optional<StmtPtr> parse_var_declaration();
        std::optional<StmtPtr> parse_return();
        std::optional<StmtPtr> parse_while();
        std::optional<StmtPtr> parse_for();
        std::optional<StmtPtr> parse_break();
        std::optional<StmtPtr> parse_continue();
        std::optional<StmtPtr> parse_block_stmt();
        std::optional<StmtPtr> parse_expression_stmt();
        std::optional<StmtPtr> parse_stmt();
        std::pair<std::unique_ptr<Program>, std::vector<CompileError>> parse();

        // Expression parsing methods (Pratt parser)
        [[nodiscard]] std::optional<ExprPtr> parse_expr(std::size_t min_bp);
        [[nodiscard]] std::optional<ExprPtr> parse_expr_inner(std::size_t min_bp);
        [[nodiscard]] std::optional<ExprPtr> nud();
        [[nodiscard]] std::optional<ExprPtr> led(ExprPtr left);

        // Expression helpers
        void extract_elements(const TokenKind kind, std::vector<ExprPtr>& elements);
        [[nodiscard]] ExprPtr parse_unary(UnaryOp op, const Token &token);
        [[nodiscard]] std::optional<ExprPtr> parse_array_literal(const Token &start_token);
        [[nodiscard]] std::optional<ExprPtr> parse_binary(ExprPtr left, const Token &token);
        [[nodiscard]] std::optional<ExprPtr> parse_grouping(const Token &start_token);
        [[nodiscard]] std::optional<ExprPtr> parse_assignment(ExprPtr left, const Token &token);
        [[nodiscard]] std::optional<ExprPtr> parse_call(ExprPtr callee, const Token &start_token);
        [[nodiscard]] std::optional<ExprPtr> parse_array_access(ExprPtr array, const Token &start_token);
        bool is_end_of_statement() const;
        Token peek() const;
        Token previous() const;
        bool check(const TokenKind kind) const;
        bool is_at_end() const;
        Token advance();
        bool match_token(const TokenKind kind);
        [[nodiscard]] bool expect(const TokenKind kind, std::string_view context);
        void syntax_error(std::string_view message, const Token &token, std::optional<std::string> help,
                          std::optional<ErrorCode> error_code);
        void report_peek_error(std::string_view message, std::optional<std::string> help);
        [[nodiscard]] SourceSpan calculate_return_span(const Token& start,
                                                    const std::optional<ExprPtr>& value) const;

        // Recursion control
        [[nodiscard]] bool check_recursion_limit();
        void enter_recursion();
        void exit_recursion();
        SourceSpan merged_span(const Token &start_token) const;
        std::optional<std::string_view> consume_identifier();
    private:
        std::vector<Token> tokens_;
        std::size_t current_;
        std::size_t recursion_depth_;
        std::vector<CompileError> errors_;
        static constexpr std::size_t MAX_RECURSION_DEPTH = 1000;
    };

}  // namespace jsv

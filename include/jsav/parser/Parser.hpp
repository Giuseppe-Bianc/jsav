/*
 * Created by gbian on 23/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

#include "../ast/Expressions.hpp"
#include "../ast/Node.hpp"
#include "../ast/Program.hpp"
#include "../ast/Statements.hpp"
#include "../error/CompileError.hpp"
#include "../headers.hpp"
#include "../lexer/Token.hpp"

namespace jsv {

    class Parser {
    public:
        explicit Parser(const std::vector<Token> &tokens);
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
        Token peek() const;
        Token previous() const;
        bool check(const TokenKind kind) const;
        bool is_at_end() const;
        Token advance();
        bool match_token(const TokenKind kind);
        [[nodiscard]] bool expect(const TokenKind kind, std::string_view context);
        void syntax_error(std::string_view message, const Token &token, std::optional<std::string> help,
                          std::optional<ErrorCode> error_code);

    private:
        std::vector<Token> tokens_;
        std::size_t current_;
        std::vector<CompileError> errors_;
    };

}  // namespace jsv

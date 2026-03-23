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
// clang-format off
namespace jsv {

    class Parser {
    public:
        explicit Parser(const std::vector<Token> &tokens);
        std::pair<std::unique_ptr<Program>, std::vector<CompileError>> parse();
        Token peek() const;
        Token previous() const;
        bool check(const TokenKind kind) const;
        bool is_at_end() const;
        Token advance();
        bool match_token(const TokenKind kind);
        [[nodiscard]] bool expect(const TokenKind kind, std::string_view context);
    private:
        std::vector<Token> tokens_;
        std::size_t current_;
        std::vector<CompileError> errors_;
    };

}  // namespace jsv
   // clang-format on
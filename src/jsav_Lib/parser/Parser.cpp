/*
 * Created by gbian on 23/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#include "jsav/parser/Parser.hpp"

namespace jsv {
    Parser::Parser(const std::vector<Token> &tokens) : tokens_(tokens), current_(0) {
        // Reserve space for errors to avoid reallocations
        // Typical programs have few syntax errors, 8 is a reasonable initial capacity
        errors_.reserve(8);
    }
    std::pair<std::unique_ptr<Program>, std::vector<CompileError>> Parser::parse() {
        std::vector<StmtPtr> stmts;
        stmts.reserve(tokens_.size() / 4);
        while(!is_at_end()) {
            if(auto stmt = parse_stmt(); stmt.has_value()) {
                stmts.push_back(std::move(stmt.value()));
            } else {
                // Skip the current token to avoid infinite loop on syntax error
                advance();
            }
        }
        return std::make_pair(std::make_unique<Program>(vnd_move(stmts)), vnd_move(errors_));
    }

    std::optional<StmtPtr> Parser::parse_stmt() {
        const auto token = peek();
        switch(token.getKind()) {
        case TokenKind::KeywordFun:
            return parse_function();
        case TokenKind::KeywordMain:
            return parse_main_function();
        case TokenKind::KeywordIf:
            return parse_if();
        case TokenKind::KeywordVar:
        case TokenKind::KeywordConst:
            return parse_var_declaration();
        case TokenKind::KeywordReturn:
            return parse_return();
        case TokenKind::KeywordWhile:
            return parse_while();
        case TokenKind::KeywordFor:
            return parse_for();
        case TokenKind::KeywordBreak:
            return parse_break();
        case TokenKind::KeywordContinue:
            return parse_continue();
        case TokenKind::OpenBrace:
            return parse_block_stmt();
        default:
            return parse_expression_stmt();
        }
    }
    std::optional<StmtPtr> Parser::parse_function() { return std::nullopt; }
    std::optional<StmtPtr> Parser::parse_main_function() {
        const auto start_token = advance();
        auto body = parse_block_stmt();
        if(!body.has_value()) return std::nullopt;
        const auto end_span = body.value()->location();
        const auto function_span = start_token.getSpan().merged(end_span).value_or(start_token.getSpan());
        return std::make_unique<MainStmt>(std::move(body.value()), function_span);
    }
    std::optional<StmtPtr> Parser::parse_if() { return std::nullopt; }
    std::optional<StmtPtr> Parser::parse_var_declaration() { return std::nullopt; }
    std::optional<StmtPtr> Parser::parse_return() { return std::nullopt; }
    std::optional<StmtPtr> Parser::parse_while() { return std::nullopt; }
    std::optional<StmtPtr> Parser::parse_for() { return std::nullopt; }
    std::optional<StmtPtr> Parser::parse_break() {
        const auto span = advance().getSpan();
        return std::make_unique<BreakStmt>(span);
    }
    std::optional<StmtPtr> Parser::parse_continue() {
        const auto span = advance().getSpan();
        return std::make_unique<ContinueStmt>(span);
    }
    std::optional<StmtPtr> Parser::parse_block_stmt() {
        const auto start_token = advance();
        std::vector<StmtPtr> statements;
        while(!check(TokenKind::CloseBrace) && !is_at_end()) {
            if(auto stmt = parse_stmt()) {
                statements.push_back(std::move(stmt.value()));
            } else {
                advance();
            }
        }
        [[maybe_unused]] auto e = expect(TokenKind::CloseBrace, "end of block");
        statements.shrink_to_fit();
        return std::make_unique<BlockStmt>(std::move(statements), start_token.getSpan());
    }
    std::optional<StmtPtr> Parser::parse_expression_stmt() { return std::nullopt; }

    Token Parser::peek() const { return tokens_.at(current_); }
    Token Parser::previous() const { return tokens_.at(current_ - 1); }
    bool Parser::check(const TokenKind kind) const { return peek().getKind() == kind; }
    bool Parser::is_at_end() const { return check(TokenKind::Eof); }
    Token Parser::advance() {
        if(!is_at_end()) { ++current_; }
        return previous();
    }
    bool Parser::match_token(const TokenKind kind) {
        if(check(kind)) {
            advance();
            return true;
        }
        return false;
    }

    [[nodiscard]] bool Parser::expect(const TokenKind kind, std::string_view context) {
        if(match_token(kind)) { return true; }

        const std::optional<Token> current_token = is_at_end() ? std::nullopt : std::optional{peek()};

        const std::string found_str = current_token ? std::string{tokenKindToString(current_token->getKind())} : "end of input";

        const auto span = current_token ? current_token->getSpan() : SourceSpan{};

        const auto help_message = FORMAT("Try adding a {}", tokenKindToString(kind));

        errors_.push_back(CompileError::SyntaxError(
            ErrorCode::E1004, FORMAT("Expected {} in {}, found {}.", tokenKindToString(kind), context, found_str), span, help_message));

        return false;
    }

    void Parser::syntax_error(const std::string_view message, const Token &token, std::optional<std::string> help,
                              std::optional<ErrorCode> error_code) {
        errors_.push_back(CompileError::SyntaxError(error_code, message, token.getSpan(), help));
    }
}  // namespace jsv
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
        return std::make_pair(std::make_unique<Program>(vnd_move(stmts)), vnd_move(errors_));
    }

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
        if (match_token(kind)) {
            return true;
        }
        
        const std::optional<Token> current_token = is_at_end() ? std::nullopt : std::optional{peek()};
        
        const std::string found_str = current_token 
            ? std::string{tokenKindToString(current_token->getKind())}
            : "end of input";
        
        const auto span = current_token ? current_token->getSpan() : SourceSpan{};
        
        const auto help_message = FORMAT("Try adding a {}", tokenKindToString(kind));
        
        errors_.push_back(CompileError::SyntaxError(
            ErrorCode::E1004,
            FORMAT("Expected {} in {}, found {}.", tokenKindToString(kind), context, found_str),
            span,
            help_message
        ));
        
        return false;
    }

}  // namespace jsv
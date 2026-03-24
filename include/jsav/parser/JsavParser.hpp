/*
 * Created by gbian on 23/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

#include "../ast/Expressions.hpp"
#include "../ast/Statements.hpp"
#include "../error/CompileError.hpp"
#include "../lexer/Token.hpp"
#include "../location/SourceSpan.hpp"

#include <span>
#include <vector>
#include <optional>
#include <cstdint>
#include <memory>

namespace jsv {

/// Parameter structure for function declarations
struct Parameter {
    std::string name;
    jsv::Type type_annotation;
    SourceSpan span;
};

/// Result type containing parsed statements and errors
struct ParseResult {
    std::vector<StmtPtr> statements;
    std::vector<CompileError> errors;
};

/// Pratt parser implementation for jsav language
class JsavParser {
public:
    /// Construct parser with non-owning view of tokens
    explicit JsavParser(std::span<const Token> tokens);

    /// Parse all tokens and return statements with errors
    [[nodiscard]] ParseResult parse();

private:
    // Statement parsing methods
    [[nodiscard]] std::optional<StmtPtr> parse_stmt();
    [[nodiscard]] std::optional<StmtPtr> parse_main_function();
    [[nodiscard]] std::optional<StmtPtr> parse_break();
    [[nodiscard]] std::optional<StmtPtr> parse_continue();
    [[nodiscard]] std::optional<StmtPtr> parse_block_stmt();
    [[nodiscard]] std::optional<StmtPtr> parse_return();
    [[nodiscard]] std::optional<StmtPtr> parse_function();
    [[nodiscard]] std::optional<StmtPtr> parse_if();
    [[nodiscard]] std::optional<StmtPtr> parse_while();
    [[nodiscard]] std::optional<StmtPtr> parse_for();
    [[nodiscard]] std::optional<StmtPtr> parse_var_declaration();
    [[nodiscard]] std::optional<StmtPtr> parse_expression_stmt();

    // Expression parsing methods (Pratt parser)
    [[nodiscard]] std::optional<ExprPtr> parse_expr(std::uint8_t min_bp);
    [[nodiscard]] std::optional<ExprPtr> parse_expr_inner(std::uint8_t min_bp);
    [[nodiscard]] std::optional<ExprPtr> nud();
    [[nodiscard]] std::optional<ExprPtr> led(const ExprPtr& left);

    // Expression helpers
    [[nodiscard]] ExprPtr parse_unary(UnaryOp op, const Token& token);
    [[nodiscard]] std::optional<ExprPtr> parse_array_literal(const Token& start_token);
    [[nodiscard]] std::optional<ExprPtr> parse_binary(const ExprPtr& left, const Token& token);
    [[nodiscard]] std::optional<ExprPtr> parse_grouping(const Token& start_token);
    [[nodiscard]] std::optional<ExprPtr> parse_assignment(const ExprPtr& left, const Token& token);
    [[nodiscard]] std::optional<ExprPtr> parse_call(const ExprPtr& callee, const Token& start_token);
    [[nodiscard]] std::optional<ExprPtr> parse_array_access(const ExprPtr& array, const Token& start_token);

    // Type parsing
    [[nodiscard]] std::optional<jsv::Type> parse_type();

    // Condition parsing helper
    [[nodiscard]] std::optional<ExprPtr> parse_condition(std::string_view keyword);

    // For loop helpers
    [[nodiscard]] std::optional<StmtPtr> parse_for_initializer();

    // Element extraction helper
    void extract_elements(TokenKind kind, std::vector<ExprPtr>& elements);

    // Identifier handling
    [[nodiscard]] std::optional<std::string> consume_identifier();

    // Token navigation (const methods - no state mutation except current_)
    [[nodiscard]] const Token& peek() const;
    [[nodiscard]] const Token* previous() const;
    [[nodiscard]] bool check(TokenKind kind) const;
    [[nodiscard]] bool is_at_end() const;
    const Token& advance();
    bool match_token(TokenKind kind);

    // Error handling
    [[nodiscard]] bool expect(TokenKind kind, std::string_view context);
    void syntax_error(std::string_view message, const Token& token,
                      std::optional<std::string> help, std::optional<ErrorCode> error_code);
    void report_peek_error(std::string_view message, std::optional<std::string> help);

    // Span utilities
    [[nodiscard]] SourceSpan merged_span(const Token& start_token) const;
    [[nodiscard]] SourceSpan calculate_return_span(const Token& start,
                                                    const std::optional<ExprPtr>& value) const;

    // Recursion control
    [[nodiscard]] bool check_recursion_limit();
    void enter_recursion();
    void exit_recursion();

    // Statement end detection
    [[nodiscard]] bool is_end_of_statement() const;

    // Data members
    std::span<const Token> tokens_;  // Non-owning view (Rust: &'a [Token])
    std::size_t current_;            // Current token index
    std::vector<CompileError> errors_;
    std::size_t recursion_depth_;

    static constexpr std::size_t MAX_RECURSION_DEPTH = 1000;
};

}  // namespace jsv

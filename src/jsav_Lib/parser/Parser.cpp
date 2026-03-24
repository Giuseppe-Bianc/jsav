/*
 * Created by gbian on 23/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#include "jsav/parser/Parser.hpp"
#include "jsav/parser/precedence.hpp"

namespace jsv {
    Parser::Parser(const std::vector<Token> &tokens) : tokens_(tokens), current_(0), recursion_depth_(0) {
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
    std::optional<StmtPtr> Parser::parse_return() {
        const auto start_token = advance();
        std::optional<ExprPtr> return_value;
        if(!is_end_of_statement()) { return_value = parse_expr(0); }
        auto stmt = std::make_unique<ReturnStmt>(
            return_value ? std::move(*return_value) : nullptr,
            calculate_return_span(start_token, return_value)
        );
        return stmt;
    }
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
    std::optional<StmtPtr> Parser::parse_expression_stmt() {
        auto expr = parse_expr(0);
        if (!expr) {
            // Se non c'è un'espressione valida, segnala errore e avanza
            syntax_error("Expected expression statement", peek(), std::nullopt, ErrorCode::E1004);
            advance();
            return std::nullopt;
        }
        // Consuma opzionalmente il punto e virgola
        match_token(TokenKind::Semicolon);
        return std::make_unique<ExprStmt>(std::move(expr.value()));
    }

    std::optional<ExprPtr> Parser::parse_expr(const std::size_t min_bp) {
        if(check_recursion_limit()) { return std::nullopt; }
        enter_recursion();
        auto result = parse_expr_inner(min_bp);
        exit_recursion();
        return result;
    }

    std::optional<ExprPtr> Parser::parse_expr_inner(const std::size_t min_bp) {
        auto left = nud();
        if(!left) return std::nullopt;
        while(true) {
            const auto &token = peek();
            const auto [lbp, _] = binding_power(token);
            if(lbp <= min_bp) { break; }
            auto new_left = led(std::move(*left));
            left = std::nullopt;
            if(!new_left) break;
            left = std::move(new_left);
        }
        return left;
    }

    std::optional<ExprPtr> Parser::nud() {
        const auto token = advance();
        switch(token.getKind()) {
        case TokenKind::Numeric:
            {
                const auto text = token.getText();
                const auto value = std::strtoll(text.data(), nullptr, 10);
                return std::make_unique<IntegerLiteral>(value, token.getSpan());
            }
        case TokenKind::KeywordBool:
            {
                const auto text = token.getText();
                const auto value = (text == "true");
                return std::make_unique<BoolLiteral>(value, token.getSpan());
            }
        case TokenKind::KeywordNullptr:
            return std::make_unique<NullLiteral>(token.getSpan());
        case TokenKind::StringLiteral:
            return std::make_unique<StringLiteral>(std::string{token.getText()}, token.getSpan());
        case TokenKind::CharLiteral:
            {
                const auto text = token.getText();
                const auto value = text.empty() ? '\0' : text[0];
                return std::make_unique<CharLiteral>(value, token.getSpan());
            }
        case TokenKind::Minus:
            return parse_unary(UnaryOp::Negate, token);
        case TokenKind::Not:
            return parse_unary(UnaryOp::Not, token);
        case TokenKind::OpenBrace:
            return parse_array_literal(token);
        case TokenKind::OpenParen:
            return parse_grouping(token);
        case TokenKind::IdentifierAscii:
        case TokenKind::IdentifierUnicode:
            return std::make_unique<Identifier>(std::string{token.getText()}, token.getSpan());
        default:
            syntax_error("Unexpected token", token, "Expected an expression (number, string, variable, or operator)", ErrorCode::E1004);
            return std::nullopt;
        }
    }

    std::optional<ExprPtr> Parser::led(ExprPtr left) {
        const auto token = advance();
        switch(token.getKind()) {
        case TokenKind::Plus:
        case TokenKind::Minus:
        case TokenKind::Star:
        case TokenKind::Slash:
        case TokenKind::Percent:
        case TokenKind::EqualEqual:
        case TokenKind::NotEqual:
        case TokenKind::Less:
        case TokenKind::LessEqual:
        case TokenKind::Greater:
        case TokenKind::GreaterEqual:
        case TokenKind::AndAnd:
        case TokenKind::OrOr:
        case TokenKind::And:
        case TokenKind::Or:
        case TokenKind::Xor:
        case TokenKind::ShiftLeft:
        case TokenKind::ShiftRight:
            return parse_binary(std::move(left), token);  // ← std::move
        case TokenKind::Equal:
            return parse_assignment(std::move(left), token);
        case TokenKind::OpenParen:
            return parse_call(std::move(left), token);
        case TokenKind::OpenBracket:
            return parse_array_access(std::move(left), token);
        default:
            syntax_error("Unexpected operator", token, "This operator is not supported in this context", ErrorCode::E1004);
            return std::nullopt;
        }
    }

    ExprPtr Parser::parse_unary(const UnaryOp op, const Token &token) {
        const auto [_, rbp] = unary_binding_power(token);
        auto expr = parse_expr(rbp);
        if(!expr) { expr = std::make_unique<NullLiteral>(token.getSpan()); }
        return std::make_unique<UnaryExpr>(op, std::move(expr.value()), token.getSpan());
    }

    void Parser::extract_elements(const TokenKind kind, std::vector<ExprPtr>& elements) {
    while (!check(kind) && !is_at_end()) {
        if (auto expr = parse_expr(0)) {
            elements.push_back(std::move(expr.value()));
        }
        if (!match_token(TokenKind::Comma)) {
            break;
        }
    }}

    std::optional<ExprPtr> Parser::parse_array_literal([[maybe_unused]] const [[maybe_unused]] Token &start_token) {
        std::vector<ExprPtr> elements;
        extract_elements(TokenKind::CloseBrace, elements);
        if(!expect(TokenKind::CloseBrace, "end of array literal")) { return std::nullopt; }
        elements.shrink_to_fit();
        return std::make_unique<ArrayLiteral>(std::move(elements), merged_span(start_token));
    }

    std::optional<ExprPtr> Parser::parse_binary(ExprPtr left, const Token &token) {
        auto op_result = get_binary_op(token);
        if(!op_result.has_value()) {
            errors_.push_back(std::move(op_result.error()));
            return std::nullopt;
        }
        const BinaryOp op = op_result.value();  // ← was missing the type
        const auto [_, rbp] = binding_power(token);
        auto right = parse_expr(rbp);
        if(!right) { right = std::make_unique<NullLiteral>(token.getSpan()); }
        const auto span = left->location().merged(right.value()->location()).value_or(token.getSpan());
        return std::make_unique<BinaryExpr>(op, std::move(left), std::move(right.value()), span);
    }
    std::optional<ExprPtr> Parser::parse_grouping([[maybe_unused]] const [[maybe_unused]] Token &start_token) {
        auto expr = parse_expr(0);
        if(!expect(TokenKind::CloseParen, "end of grouping")) { return std::nullopt; }
        return std::make_unique<GroupingExpr>(std::move(expr.value()), merged_span(start_token));
    }

    std::optional<ExprPtr> Parser::parse_assignment([[maybe_unused]] ExprPtr left, [[maybe_unused]] const Token &token) {
        auto value = parse_expr(1);
        if(!value) { value = std::make_unique<NullLiteral>(token.getSpan()); }
        const auto span = left->location().merged(value.value()->location()).value_or(token.getSpan());
        const bool valid = (left->kind() == NodeKind::Identifier || left->kind() == NodeKind::IndexExpr);
        if(!valid) {
            errors_.push_back(CompileError::SyntaxError(ErrorCode::E1003, "Invalid left-hand side in assignment", left->location(),
                                                        "Only variables and array elements can be assigned to. "
                                                        "Consider using a variable name or an array access expression."));
            return std::nullopt;
        }
        return std::make_unique<AssignExpr>(std::move(left), std::move(value.value()), span);
    }

    std::optional<ExprPtr> Parser::parse_call([[maybe_unused]] ExprPtr callee, [[maybe_unused]] const Token &start_token) {
        std::vector<ExprPtr> arguments;
        extract_elements(TokenKind::CloseParen, arguments);
        if(!expect(TokenKind::CloseParen, "end of function call arguments")) { return std::nullopt; }
        arguments.shrink_to_fit();
        return std::make_unique<CallExpr>(std::move(callee), std::move(arguments), merged_span(start_token));
    }

    std::optional<ExprPtr> Parser::parse_array_access([[maybe_unused]] ExprPtr array, [[maybe_unused]] const Token &start_token) {
        auto index = parse_expr(0);
        if(!index) { index = std::make_unique<NullLiteral>(start_token.getSpan()); }
        if(!expect(TokenKind::CloseBracket, "end of array access")) { return std::nullopt; }
        return std::make_unique<IndexExpr>(std::move(array), std::move(index.value()), merged_span(start_token));
    }

    bool Parser::is_end_of_statement() const {
        switch(peek().getKind()) {
        case TokenKind::CloseBrace:
        case TokenKind::Eof:
        case TokenKind::Semicolon:
            return true;
        default:
            return false;
        }
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

    bool Parser::expect(const TokenKind kind, std::string_view context) {
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

    void Parser::report_peek_error(const std::string_view message, const std::optional<std::string> help) {
        if(!is_at_end()) { syntax_error(message, peek(), help, ErrorCode::E1004); }
    }

    bool Parser::check_recursion_limit() {
        if(recursion_depth_ > MAX_RECURSION_DEPTH) {
            if(!is_at_end()) {
                syntax_error("Maximum recursion depth exceeded", peek(), "Simplify the expression or break it into smaller parts", ErrorCode::E1001);
            }
            return true;
        }
        return false;
    }

    void Parser::enter_recursion() { ++recursion_depth_; }

    void Parser::exit_recursion() {
        if(recursion_depth_ > 0) { --recursion_depth_; }
    }
    SourceSpan Parser::calculate_return_span(const Token &start, const std::optional<ExprPtr> &value) const {
        if(value.has_value()) { return start.getSpan().merged(value.value()->location()).value_or(start.getSpan()); }
        return start.getSpan();
    }
    SourceSpan Parser::merged_span(const Token &start_token) const {
        return start_token.getSpan().merged(previous().getSpan()).value_or(start_token.getSpan());
    }
}  // namespace jsv
/*
 * Created by gbian on 23/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#include "jsav/parser/JsavParser.hpp"
#include "jsav/parser/precedence.hpp"

#include <fmt/format.h>

namespace jsv {

JsavParser::JsavParser(std::span<const Token> tokens)
    : tokens_(tokens), current_(0), errors_(), recursion_depth_(0) {
    errors_.reserve(8);
}

ParseResult JsavParser::parse() {
    std::vector<StmtPtr> statements;
    statements.reserve(tokens_.size() / 4);
    while (!is_at_end()) {
        if (auto stmt = parse_stmt()) {
            statements.push_back(std::move(stmt.value()));
        } else {
            advance();
        }
    }
    statements.shrink_to_fit();
    errors_.shrink_to_fit();
    return {std::move(statements), std::move(errors_)};
}

// ============================================================================
// Token navigation methods
// ============================================================================

const Token& JsavParser::peek() const { return tokens_[current_]; }

const Token* JsavParser::previous() const {
    if (current_ == 0) return nullptr;
    return &tokens_[current_ - 1];
}

bool JsavParser::check(const TokenKind kind) const {
    return peek().getKind() == kind;
}

bool JsavParser::is_at_end() const { return check(TokenKind::Eof); }

const Token& JsavParser::advance() {
    if (!is_at_end()) {
        ++current_;
    }
    return *previous();
}

bool JsavParser::match_token(const TokenKind kind) {
    if (check(kind)) {
        advance();
        return true;
    }
    return false;
}

// ============================================================================
// Error handling methods
// ============================================================================

bool JsavParser::expect(const TokenKind kind, const std::string_view context) {
    if (match_token(kind)) {
        return true;
    }
    const auto* current_token = is_at_end() ? nullptr : &peek();
    const std::string found_str = current_token ? std::string{tokenKindToString(current_token->getKind())} : "end of input";
    const auto span = current_token ? current_token->getSpan() : SourceSpan{};
    const auto help_message = fmt::format("Try adding a {}", tokenKindToString(kind));
    errors_.push_back(CompileError::SyntaxError(
        ErrorCode::E1004, fmt::format("Expected {} in {}, found {}.", tokenKindToString(kind), context, found_str), span, help_message));
    return false;
}

void JsavParser::syntax_error(const std::string_view message, const Token& token,
                              const std::optional<std::string> help,
                              const std::optional<ErrorCode> error_code) {
    errors_.push_back(CompileError::SyntaxError(
        error_code, fmt::format("{}: {}", message, tokenKindToString(token.getKind())), token.getSpan(), help));
}

void JsavParser::report_peek_error(const std::string_view message,
                                   const std::optional<std::string> help) {
    if (!is_at_end()) {
        syntax_error(message, peek(), help, ErrorCode::E1004);
    }
}

// ============================================================================
// Recursion control
// ============================================================================

bool JsavParser::check_recursion_limit() {
    if (recursion_depth_ > MAX_RECURSION_DEPTH) {
        if (!is_at_end()) {
            const auto& token = peek();
            errors_.push_back(CompileError::SyntaxError(
                ErrorCode::E1001, "Maximum recursion depth exceeded", token.getSpan(),
                "Simplify the expression or break it into smaller parts"));
        }
        return true;
    }
    return false;
}

void JsavParser::enter_recursion() { ++recursion_depth_; }

void JsavParser::exit_recursion() {
    if (recursion_depth_ > 0) {
        --recursion_depth_;
    }
}

// ============================================================================
// Statement parsing methods
// ============================================================================

std::optional<StmtPtr> JsavParser::parse_stmt() {
    const auto& token = peek();
    switch (token.getKind()) {
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

std::optional<StmtPtr> JsavParser::parse_main_function() {
    const auto start_token = advance();
    auto body = parse_block_stmt();
    if (!body) return std::nullopt;
    const auto end_span = body->location();
    const auto function_span = start_token.getSpan().merged(end_span).value_or(start_token.getSpan());
    return std::make_unique<BlockStmt>(std::vector<StmtPtr>{std::move(body)}, function_span);
}

std::optional<StmtPtr> JsavParser::parse_break() {
    const auto span = advance().getSpan();
    return std::make_unique<BreakStmt>(span);
}

std::optional<StmtPtr> JsavParser::parse_continue() {
    const auto span = advance().getSpan();
    return std::make_unique<ContinueStmt>(span);
}

std::optional<StmtPtr> JsavParser::parse_block_stmt() {
    const auto start_token = advance();
    std::vector<StmtPtr> statements;
    while (!check(TokenKind::CloseBrace) && !is_at_end()) {
        if (auto stmt = parse_stmt()) {
            statements.push_back(std::move(stmt.value()));
        } else {
            advance();
        }
    }
    expect(TokenKind::CloseBrace, "end of block");
    statements.shrink_to_fit();
    return std::make_unique<BlockStmt>(std::move(statements), merged_span(start_token));
}

std::optional<StmtPtr> JsavParser::parse_return() {
    const auto start_token = advance();
    std::optional<ExprPtr> return_value;
    if (!is_end_of_statement()) {
        return_value = parse_expr(0);
    }
    auto stmt = std::make_unique<ReturnStmt>(std::move(return_value), calculate_return_span(start_token, return_value));
    return stmt;
}

bool JsavParser::is_end_of_statement() const {
    switch (peek().getKind()) {
    case TokenKind::CloseBrace:
    case TokenKind::Eof:
    case TokenKind::Semicolon:
        return true;
    default:
        return false;
    }
}

SourceSpan JsavParser::calculate_return_span(const Token& start,
                                              const std::optional<ExprPtr>& value) const {
    if (value.has_value()) {
        return start.getSpan().merged(value->location()).value_or(start.getSpan());
    }
    return start.getSpan();
}

std::optional<StmtPtr> JsavParser::parse_function() {
    const auto start_token = advance();
    auto name = consume_identifier();
    if (!name) return std::nullopt;
    expect(TokenKind::OpenParen, "after function name");
    std::vector<FuncParam> params;
    while (!check(TokenKind::CloseParen) && !is_at_end()) {
        const auto param_start = peek();
        auto param_name = consume_identifier();
        if (!param_name) return std::nullopt;
        expect(TokenKind::Colon, "after parameter name");
        auto type_ann = parse_type();
        if (!type_ann) return std::nullopt;
        const auto type_span = previous()->getSpan();
        const auto param_span = previous()->getSpan().merged(type_span).value_or(param_start.getSpan());
        params.push_back({std::move(name.value()), type_ann.value(), param_span});
        if (!match_token(TokenKind::Comma)) {
            break;
        }
    }
    params.shrink_to_fit();
    expect(TokenKind::CloseParen, "after parameter list");
    std::optional<jsv::Type> return_type;
    if (match_token(TokenKind::Colon)) {
        return_type = parse_type();
    }
    auto body = parse_block_stmt();
    if (!body) return std::nullopt;
    const auto end_span = body->location();
    const auto function_span = start_token.getSpan().merged(end_span).value_or(start_token.getSpan());
    return std::make_unique<FuncDecl>(
        std::move(name.value()), std::move(params),
        return_type.value_or(jsv::Type::Void), std::move(body), function_span);
}

std::optional<ExprPtr> JsavParser::parse_condition(const std::string_view keyword) {
    expect(TokenKind::OpenParen, fmt::format("after '{}'", keyword));
    auto condition = parse_expr(0);
    expect(TokenKind::CloseParen, "after the condition");
    return condition;
}

std::optional<StmtPtr> JsavParser::parse_if() {
    const auto start_token = advance();
    auto condition = parse_condition("if");
    if (!condition) return std::nullopt;
    auto then_branch = parse_block_stmt();
    if (!then_branch) return std::nullopt;
    std::optional<StmtPtr> else_branch;
    if (match_token(TokenKind::KeywordElse)) {
        else_branch = parse_stmt();
    }
    return std::make_unique<IfStmt>(std::move(condition.value()), std::move(then_branch.value()),
                         std::move(else_branch), merged_span(start_token));
}

std::optional<StmtPtr> JsavParser::parse_while() {
    const auto start_token = advance();
    auto condition = parse_condition("while");
    if (!condition) return std::nullopt;
    auto body = parse_block_stmt();
    if (!body) return std::nullopt;
    const auto end_span = body->location();
    const auto function_span = start_token.getSpan().merged(end_span).value_or(start_token.getSpan());
    return std::make_unique<WhileStmt>(std::move(condition.value()), std::move(body.value()), function_span);
}

std::optional<StmtPtr> JsavParser::parse_for_initializer() {
    if (match_token(TokenKind::Semicolon)) {
        return nullptr;
    }
    std::optional<StmtPtr> stmt;
    if (check(TokenKind::KeywordVar) || check(TokenKind::KeywordConst)) {
        stmt = parse_var_declaration();
    } else {
        stmt = parse_expression_stmt();
    }
    expect(TokenKind::Semicolon, "after for loop initializer");
    return stmt;
}

std::optional<StmtPtr> JsavParser::parse_for() {
    const auto start_token = advance();
    expect(TokenKind::OpenParen, "after 'for'");
    auto initializer = parse_for_initializer();
    std::optional<ExprPtr> condition;
    if (check(TokenKind::Semicolon)) {
        advance();
    } else {
        condition = parse_expr(0);
        expect(TokenKind::Semicolon, "after for loop condition");
    }
    std::optional<ExprPtr> increment;
    if (!check(TokenKind::CloseParen)) {
        increment = parse_expr(0);
    }
    expect(TokenKind::CloseParen, "after for loop clauses");
    auto body_stmt = parse_stmt();
    if (!body_stmt) return std::nullopt;
    std::vector<StmtPtr> body;
    if (body_stmt->kind() == NodeKind::BlockStmt) {
        body = static_cast<BlockStmt&>(*body_stmt.value()).statements();
    } else {
        body.push_back(std::move(body_stmt.value()));
    }
    const auto end_span =
        body.empty()
            ? previous()->getSpan()
            : body.back()->location();
    const auto span = start_token.getSpan().merged(end_span).value_or(start_token.getSpan());
    return std::make_unique<ForStmt>(std::move(initializer), std::move(condition),
                          std::move(increment), std::move(body), span);
}

std::optional<StmtPtr> JsavParser::parse_var_declaration() {
    std::optional<Token> start_token;
    bool is_mutable = false;
    if (match_token(TokenKind::KeywordConst)) {
        start_token = previous();
        is_mutable = false;
    } else if (match_token(TokenKind::KeywordVar)) {
        start_token = previous();
        is_mutable = true;
    }
    if (!start_token) return std::nullopt;
    std::vector<std::string> variables;
    while (auto name = consume_identifier()) {
        variables.push_back(std::move(name.value()));
        if (!match_token(TokenKind::Comma)) {
            break;
        }
    }
    if (variables.empty()) {
        syntax_error("Expected at least one variable name", start_token.value(),
                     "Provide at least one variable name after 'var' or 'const'", ErrorCode::E1008);
        return std::nullopt;
    }
    expect(TokenKind::Colon, "after variable name(s)");
    auto type_ann = parse_type();
    if (!type_ann) {
        report_peek_error("Invalid type specification",
                          "Try using a primitive type or a custom type identifier");
        type_ann = jsv::Type::Void;
    }
    expect(TokenKind::Equal, "after type annotation");
    std::vector<ExprPtr> initializers;
    initializers.reserve(variables.size());
    while (true) {
        auto expr = parse_expr(0);
        if (expr) {
            initializers.push_back(std::move(expr.value()));
        } else {
            report_peek_error("Expected initializer expression",
                              "Provide an expression to initialize the variable (e.g., 42, \"text\", variable_name)");
            break;
        }
        if (!match_token(TokenKind::Comma)) {
            break;
        }
    }
    if (variables.size() != initializers.size()) {
        syntax_error(fmt::format("Declaration mismatch: {} variables but {} initializers",
                                 variables.size(), initializers.size()),
                     start_token.value(),
                     "Each variable must have exactly one initializer expression", ErrorCode::E2001);
    }
    return std::make_unique<VarDecl>(std::move(variables), std::nullopt,
                                       std::move(initializers), is_mutable,
                                       merged_span(start_token.value()));
}

std::optional<StmtPtr> JsavParser::parse_expression_stmt() {
    auto expr = parse_expr(0);
    if (!expr) return std::nullopt;
    return std::make_unique<ExprStmt>(std::move(expr.value()));
}

// ============================================================================
// Expression parsing (Pratt parser)
// ============================================================================

std::optional<ExprPtr> JsavParser::parse_expr(const std::uint8_t min_bp) {
    if (check_recursion_limit()) {
        return std::nullopt;
    }
    enter_recursion();
    const auto result = parse_expr_inner(min_bp);
    exit_recursion();
    return result;
}

std::optional<ExprPtr> JsavParser::parse_expr_inner(const std::uint8_t min_bp) {
    auto left = nud();
    if (!left) return std::nullopt;
    while (true) {
        const auto& token = peek();
        const auto [lbp, _] = binding_power(token);
        if (lbp <= min_bp) {
            break;
        }
        auto new_left = led(left);
        if (!new_left) break;
        left = std::move(new_left);
    }
    return left;
}

std::optional<ExprPtr> JsavParser::nud() {
    const auto token = advance();
    switch (token.getKind()) {
    case TokenKind::Numeric: {
        const auto text = token.getText();
        const auto value = std::strtoll(text.data(), nullptr, 10);
        return std::make_unique<IntegerLiteral>(value, token.getSpan());
    }
    case TokenKind::KeywordBool: {
        const auto text = token.getText();
        const auto value = (text == "true");
        return std::make_unique<BoolLiteral>(value, token.getSpan());
    }
    case TokenKind::KeywordNullptr:
        return std::make_unique<NullLiteral>(token.getSpan());
    case TokenKind::StringLiteral:
        return std::make_unique<StringLiteral>(std::string{token.getText()}, token.getSpan());
    case TokenKind::CharLiteral: {
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
        syntax_error("Unexpected token", token,
                     "Expected an expression (number, string, variable, or operator)",
                     ErrorCode::E1004);
        return std::nullopt;
    }
}

std::optional<ExprPtr> JsavParser::led(const ExprPtr& left) {
    const auto token = advance();
    switch (token.getKind()) {
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
        return parse_binary(left, token);
    case TokenKind::Equal:
        return parse_assignment(left, token);
    case TokenKind::OpenParen:
        return parse_call(left, token);
    case TokenKind::OpenBracket:
        return parse_array_access(left, token);
    default:
        syntax_error("Unexpected operator", token,
                     "This operator is not supported in this context",
                     ErrorCode::E1004);
        return std::nullopt;
    }
}

ExprPtr JsavParser::parse_unary(const UnaryOp op, const Token& token) {
    const auto [_, rbp] = unary_binding_power(token);
    auto expr = parse_expr(rbp);
    if (!expr) {
        expr = std::make_unique<NullLiteral>(token.getSpan());
    }
    return std::make_unique<UnaryExpr>(op, std::move(expr.value()), token.getSpan());
}

std::optional<ExprPtr> JsavParser::parse_array_literal(const Token& start_token) {
    std::vector<ExprPtr> elements;
    extract_elements(TokenKind::CloseBrace, elements);
    if (!expect(TokenKind::CloseBrace, "end of array literal")) {
        return std::nullopt;
    }
    elements.shrink_to_fit();
    return std::make_unique<ArrayLiteral>(std::move(elements), merged_span(start_token));
}

std::optional<ExprPtr> JsavParser::parse_binary(const ExprPtr& left, const Token& token) {
    auto op_result = get_binary_op(token);
    if (!op_result.has_value()) {
        errors_.push_back(std::move(op_result.error()));
        return std::nullopt;
    }
    const auto op = op_result.value();
    const auto [_, rbp] = binding_power(token);
    auto right = parse_expr(rbp);
    if (!right) {
        right = std::make_unique<NullLiteral>(token.getSpan());
    }
    return std::make_unique<BinaryExpr>(op, std::move(left), std::move(right.value()), token.getSpan());
}

std::optional<ExprPtr> JsavParser::parse_grouping(const Token& start_token) {
    auto expr = parse_expr(0);
    if (!expect(TokenKind::CloseParen, "end of grouping")) {
        return std::nullopt;
    }
    return std::make_unique<GroupingExpr>(std::move(expr.value()), merged_span(start_token));
}

std::optional<ExprPtr> JsavParser::parse_assignment(const ExprPtr& left, const Token& token) {
    auto value = parse_expr(1);
    if (!value) {
        value = std::make_unique<NullLiteral>(token.getSpan());
    }
    const auto span = left->location().merged(value->location()).value_or(token.getSpan());
    const bool valid = (left->kind() == NodeKind::Identifier ||
                        left->kind() == NodeKind::IndexExpr);
    if (!valid) {
        errors_.push_back(CompileError::SyntaxError(
            ErrorCode::E1003, "Invalid left-hand side in assignment", left->location(),
            "Only variables and array elements can be assigned to. "
            "Consider using a variable name or an array access expression."));
        return std::nullopt;
    }
    return std::make_unique<AssignExpr>(std::move(left), std::move(value.value()), span);
}

std::optional<ExprPtr> JsavParser::parse_call(const ExprPtr& callee, const Token& start_token) {
    std::vector<ExprPtr> arguments;
    extract_elements(TokenKind::CloseParen, arguments);
    if (!expect(TokenKind::CloseParen, "end of function call arguments")) {
        return std::nullopt;
    }
    arguments.shrink_to_fit();
    return std::make_unique<CallExpr>(std::move(callee), std::move(arguments), merged_span(start_token));
}

std::optional<ExprPtr> JsavParser::parse_array_access(const ExprPtr& array,
                                                    const Token& start_token) {
    auto index = parse_expr(0);
    if (!index) {
        index = std::make_unique<NullLiteral>(start_token.getSpan());
    }
    if (!expect(TokenKind::CloseBracket, "end of array access")) {
        return std::nullopt;
    }
    return std::make_unique<IndexExpr>(std::move(array), std::move(index.value()), merged_span(start_token));
}

void JsavParser::extract_elements(const TokenKind kind, std::vector<ExprPtr>& elements) {
    while (!check(kind) && !is_at_end()) {
        if (auto expr = parse_expr(0)) {
            elements.push_back(std::move(expr.value()));
        }
        if (!match_token(TokenKind::Comma)) {
            break;
        }
    }
}

// ============================================================================
// Type parsing
// ============================================================================

std::optional<jsv::Type> JsavParser::parse_type() {
    const auto token = advance();
    jsv::Type base_type;
    switch (token.getKind()) {
    case TokenKind::TypeI8:
        base_type = jsv::Type::I8;
        break;
    case TokenKind::TypeI16:
        base_type = jsv::Type::I16;
        break;
    case TokenKind::TypeI32:
        base_type = jsv::Type::I32;
        break;
    case TokenKind::TypeI64:
        base_type = jsv::Type::I64;
        break;
    case TokenKind::TypeU8:
        base_type = jsv::Type::U8;
        break;
    case TokenKind::TypeU16:
        base_type = jsv::Type::U16;
        break;
    case TokenKind::TypeU32:
        base_type = jsv::Type::U32;
        break;
    case TokenKind::TypeU64:
        base_type = jsv::Type::U64;
        break;
    case TokenKind::TypeF32:
        base_type = jsv::Type::F32;
        break;
    case TokenKind::TypeF64:
        base_type = jsv::Type::F64;
        break;
    case TokenKind::TypeChar:
        base_type = jsv::Type::Char;
        break;
    case TokenKind::TypeString:
        base_type = jsv::Type::String;
        break;
    case TokenKind::TypeBool:
        base_type = jsv::Type::Bool;
        break;
    case TokenKind::IdentifierAscii:
    case TokenKind::IdentifierUnicode:
        // Note: existing Type enum doesn't have Custom variant, use Void as fallback
        base_type = jsv::Type::Void;
        break;
    default:
        syntax_error("Invalid type specification, expected primitive type or custom identifier",
                     token, "Try using a primitive type (like i32, f64) or a custom type identifier",
                     ErrorCode::E1002);
        return std::nullopt;
    }
    // Array dimensions not supported in current Type enum - skip for now
    return base_type;
}

// ============================================================================
// Span utilities
// ============================================================================

SourceSpan JsavParser::merged_span(const Token& start_token) const {
    if (const auto* end = previous()) {
        return start_token.getSpan().merged(end->getSpan()).value_or(start_token.getSpan());
    }
    return start_token.getSpan();
}

}  // namespace jsv

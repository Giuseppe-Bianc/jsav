/*
 * Created by gbian on 23/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner, *-identifier-length)
#include "jsav/parser/Parser.hpp"
#include "jsav/parser/precedence.hpp"

namespace jsv {
    constexpr std::size_t kInitialErrorCapacity = 8;

    static bool all_digits_from(std::string_view text, std::size_t start_index) noexcept {
        for(std::size_t j = start_index; j < text.size(); ++j) {
            if(std::isdigit(C_UC(text[j])) == 0) { return false; }
        }
        return true;
    }

    static std::size_t find_suffix_start(std::string_view text) noexcept {
        std::size_t suffix_start = text.size();
        bool found_suffix = false;

        for(std::size_t index = text.size(); index > 0 && !found_suffix; --index) {
            const char current_char = text[index - 1];
            if(current_char == 'd' || current_char == 'D' || current_char == 'f' || current_char == 'F') {
                suffix_start = index - 1;
                found_suffix = true;
            } else if(current_char == 'u' || current_char == 'U' || current_char == 'i' || current_char == 'I') {
                if(index < text.size()) {
                    if(all_digits_from(text, index)) {
                        suffix_start = index - 1;
                        found_suffix = true;
                    }
                } else {
                    suffix_start = index - 1;
                    found_suffix = true;
                }
            } else if(std::isdigit(C_UC(current_char)) == 0 && current_char != '.') {
                break;
            }
        }
        return suffix_start;
    }

    static std::pair<std::int64_t, std::optional<std::string>> parse_numeric_literal(std::string_view text) {
        if(text.empty()) { return {0, std::nullopt}; }

        const std::size_t suffix_start = find_suffix_start(text);

        std::int64_t value = 0;
        std::from_chars(text.data(), text.data() + suffix_start, value);

        std::optional<std::string> type_suffix;
        if(suffix_start < text.size()) { type_suffix = std::string(text.substr(suffix_start)); }

        return {value, std::move(type_suffix)};
    }

    struct ForInitResult {
        std::optional<StmtPtr> stmt;
        bool was_empty{false};
    };

    static std::optional<std::vector<std::string>> parse_for_var_names(Parser &parser) {
        std::vector<std::string> names;
        while(true) {
            const auto name = parser.consume_identifier();
            if(!name) { return std::nullopt; }
            names.emplace_back(*name);
            if(!parser.match_token(TokenKind::Comma)) { break; }
        }
        return names;
    }

    static std::optional<std::optional<std::string>> parse_for_type_annotation(Parser &parser) {
        if(!parser.match_token(TokenKind::Colon)) { return std::optional<std::string>{}; }
        auto [type_opt, type_str] = parser.parse_type();
        if(!type_opt) { return std::nullopt; }
        return type_str;
    }

    static std::optional<std::vector<ExprPtr>> parse_for_initializers(Parser &parser) {
        if(!parser.match_token(TokenKind::Equal)) { return std::vector<ExprPtr>{}; }
        std::vector<ExprPtr> initializers;
        while(true) {
            auto expr = parser.parse_expr(0);
            if(!expr) { return std::nullopt; }
            initializers.push_back(std::move(expr.value()));
            if(!parser.match_token(TokenKind::Comma)) { break; }
        }
        return initializers;
    }

    static StmtPtr make_for_var_decl(const Token &start_token, std::vector<std::string> &&names,
                                     std::optional<std::string> &&type_annotation, std::vector<ExprPtr> &&initializers) {
        const bool is_const = start_token.getKind() == TokenKind::KeywordConst;
        return std::make_unique<VarDecl>(std::move(names), std::move(type_annotation), std::move(initializers), is_const,
                                         start_token.getSpan());
    }

    // SAFETY: returns std::nullopt on error; otherwise a ForInitResult.
    static std::optional<ForInitResult> parse_for_initializer_clause(Parser &parser) {
        ForInitResult result{};

        if(!parser.check(TokenKind::Semicolon)) {
            result.stmt = parser.parse_for_initializer();
            if(!result.stmt) { return std::nullopt; }
            if(!parser.check(TokenKind::Semicolon)) { return std::nullopt; }
            parser.advance();
        } else {
            parser.advance();
            result.was_empty = true;
        }

        return result;  // NRVO
    }

    static std::optional<std::optional<ExprPtr>> parse_for_condition_clause(Parser &parser, bool initializer_was_empty) {
        std::optional<ExprPtr> condition;

        if(initializer_was_empty && parser.check(TokenKind::Semicolon)) {
            parser.advance();
            return std::optional<ExprPtr>{};  // empty condition — infinite loop
        }

        if(!parser.check(TokenKind::CloseParen)) {
            condition = parser.parse_expr(0);
            if(!condition) { return std::nullopt; }

            if(!parser.check(TokenKind::Semicolon)) {
                if(!parser.check(TokenKind::CloseParen)) { return std::nullopt; }
            } else {
                parser.advance();
            }
        }

        return condition;
    }

    static std::optional<ExprPtr> parse_for_increment_clause(Parser &parser) {
        if(parser.check(TokenKind::CloseParen)) { return std::optional<ExprPtr>{}; }
        return parser.parse_expr(0);
    }

    static StmtPtr make_for_body_block(StmtPtr &&body_stmt) {
        if(body_stmt->kind() == NodeKind::BlockStmt) { return std::move(body_stmt); }
        const SourceSpan body_span = body_stmt->location();
        std::vector<StmtPtr> stmts;
        stmts.push_back(std::move(body_stmt));
        return std::make_unique<BlockStmt>(std::move(stmts), body_span);
    }

    Parser::Parser(std::span<const Token> tokens) : tokens_{tokens}, current_{0}, recursion_depth_{0} {
        // Reserve space for errors to avoid reallocations
        // Typical programs have few syntax errors, 8 is a reasonable initial capacity
        errors_.reserve(kInitialErrorCapacity);
    }

    [[nodiscard]] const Token &Parser::peek() const noexcept {
        assert(current_ < tokens_.size() && "peek() past end of token stream");
        return tokens_[current_];
    }

    [[nodiscard]] const Token &Parser::previous() const noexcept {
        assert(current_ > 0 && "previous() called before any advance()");
        return tokens_[current_ - 1];
    }

    const Token &Parser::advance() noexcept {
        if(!is_at_end()) { ++current_; }
        return previous();
    }

    [[nodiscard]] bool Parser::is_at_end() const noexcept { return check(TokenKind::Eof); }

    [[nodiscard]] bool Parser::check(const TokenKind kind) const noexcept { return peek().getKind() == kind; }

    [[nodiscard]] bool Parser::is_end_of_statement() const noexcept {
        switch(peek().getKind()) {
        case TokenKind::CloseBrace:
        case TokenKind::Eof:
        case TokenKind::Semicolon:
            return true;
        default:
            return false;
        }
    }

    bool Parser::match_token(const TokenKind kind) noexcept {
        if(check(kind)) {
            advance();
            return true;
        }
        return false;
    }

    std::pair<std::unique_ptr<Program>, std::vector<CompileError>> Parser::parse() {
        std::vector<StmtPtr> stmts;
        stmts.reserve(tokens_.size() / 4);
        while(!is_at_end()) {
            if(auto stmt = parse_stmt(); stmt.has_value()) {
                stmts.push_back(std::move(stmt.value()));
            } else {
                // Skip the current token to avoid infinite loop on syntax error
                // Only advance if not at end to avoid out-of-bounds access
                if(!is_at_end()) { advance(); }
            }
        }
        return {std::make_unique<Program>(vnd_move(stmts)), vnd_move(errors_)};
    }

    std::optional<StmtPtr> Parser::parse_stmt() {
        switch(peek().getKind()) {
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
    std::optional<StmtPtr> Parser::parse_function() {
        // Consuma 'fun'
        const auto &fun_token = advance();
        // Nome funzione
        const auto &name_token = advance();
        if(name_token.getKind() != TokenKind::IdentifierAscii && name_token.getKind() != TokenKind::IdentifierUnicode) {
            syntax_error("Expected function name", name_token, {}, ErrorCode::E1001);
            return std::nullopt;
        }
        const std::string name{name_token.getText()};

        if(!expect(TokenKind::OpenParen, "after function name")) { return std::nullopt; }

        std::vector<FuncParam> parameters;
        while(!check(TokenKind::CloseParen) && !is_at_end()) {
            const auto &param_token = advance();  // PERF: reference
            if(param_token.getKind() != TokenKind::IdentifierAscii && param_token.getKind() != TokenKind::IdentifierUnicode) {
                syntax_error("Expected parameter name", param_token, {}, ErrorCode::E1002);
                return std::nullopt;
            }
            const std::string param_name{param_token.getText()};
            if(!expect(TokenKind::Colon, "after parameter name")) { return std::nullopt; }
            auto [type_opt, type_str] = parse_type();
            if(!type_opt) {
                syntax_error("Expected parameter type", peek(), {}, ErrorCode::E1003);
                return std::nullopt;
            }
            parameters.emplace_back(FuncParam{.name = param_name, .type_annotation = *type_opt, .loc = param_token.getSpan()});
            if(!check(TokenKind::CloseParen)) {
                if(!expect(TokenKind::Comma, "between parameters")) { return std::nullopt; }
            }
        }
        if(!expect(TokenKind::CloseParen, "after parameters")) { return std::nullopt; }

        // Tipo di ritorno
        std::optional<TypePtr> return_type;
        if(match_token(TokenKind::Colon)) {
            auto [ret_type_opt, ret_type_str] = parse_type();
            if(!ret_type_opt) {
                syntax_error("Expected return type after ':'", peek(), {}, ErrorCode::E1005);
                return std::nullopt;
            }
            return_type = ret_type_opt;
        } else {
            // Default: void
            return_type = PrimitiveType::void_();
        }

        // Corpo
        auto body = parse_block_stmt();
        if(!body) {
            syntax_error("Expected function body", peek(), {}, ErrorCode::E1006);
            return std::nullopt;
        }

        assert((*body)->kind() == NodeKind::BlockStmt && "parse_block_stmt must return NodeKind::BlockStmt");
        auto block_body = std::unique_ptr<BlockStmt>(static_cast<BlockStmt *>(body->release()));

        return std::make_unique<FuncDecl>(name, std::move(parameters), return_type, std::move(block_body), fun_token.getSpan());
    }
    std::optional<StmtPtr> Parser::parse_main_function() {
        const auto &start_token = advance();  // PERF: reference
        auto body = parse_block_stmt();
        if(!body.has_value()) { return std::nullopt; }
        const auto end_span = body.value()->location();
        const auto function_span = start_token.getSpan().merged(end_span).value_or(start_token.getSpan());
        return std::make_unique<MainStmt>(std::move(body.value()), function_span);
    }
    std::optional<ExprPtr> Parser::parse_condition(const std::string_view keyword) {
        [[maybe_unused]] const auto expect_open = expect(TokenKind::OpenParen, FORMAT("after '{}'", keyword));
        auto condition = parse_expr(0);
        [[maybe_unused]] const auto expect_close = expect(TokenKind::CloseParen, "after the condition");
        return condition;
    }

    std::optional<StmtPtr> Parser::parse_if() {
        const auto &start_token = advance();  // PERF: reference
        auto condition = parse_condition("if");
        if(!condition) { return std::nullopt; }
        auto then_branch = parse_block_stmt();
        if(!then_branch) { return std::nullopt; }
        std::optional<StmtPtr> else_branch;
        if(match_token(TokenKind::KeywordElse)) { else_branch = parse_stmt(); }
        return std::make_unique<IfStmt>(std::move(condition.value()), std::move(then_branch.value()),
                                        else_branch ? std::move(*else_branch) : nullptr, merged_span(start_token));
    }

    std::optional<StmtPtr> Parser::parse_var_declaration() {
        const auto &start_token = advance();  // PERF: reference — var | const
        const bool is_const = start_token.getKind() == TokenKind::KeywordConst;

        std::vector<std::string> names;
        while(true) {
            const auto name = consume_identifier();
            if(!name) { return std::nullopt; }
            names.emplace_back(*name);
            if(!match_token(TokenKind::Comma)) { break; }
        }

        std::optional<std::string> type_annotation;
        if(match_token(TokenKind::Colon)) {
            auto [type_opt, type_str] = parse_type();
            if(!type_opt) { return std::nullopt; }
            type_annotation = std::move(type_str);
        }

        std::vector<ExprPtr> initializers;
        if(match_token(TokenKind::Equal)) {
            while(true) {
                auto expr = parse_expr(0);
                if(!expr) { return std::nullopt; }
                initializers.push_back(std::move(expr.value()));
                if(!match_token(TokenKind::Comma)) { break; }
            }
        }

        match_token(TokenKind::Semicolon);
        return std::make_unique<VarDecl>(std::move(names), std::move(type_annotation), std::move(initializers), is_const,
                                         start_token.getSpan());
    }
    std::optional<StmtPtr> Parser::parse_return() {
        const auto &start_token = advance();  // PERF: reference
        std::optional<ExprPtr> return_value;
        if(!is_end_of_statement()) { return_value = parse_expr(0); }
        const auto span = Parser::calculate_return_span(start_token, return_value);
        return std::make_unique<ReturnStmt>(return_value ? std::move(*return_value) : nullptr, span);
    }

    std::optional<StmtPtr> Parser::parse_while() {
        const auto &start_token = advance();  // PERF: reference
        auto condition = parse_condition("while");
        if(!condition) { return std::nullopt; }
        auto body = parse_block_stmt();
        if(!body) { return std::nullopt; }
        const auto end_span = body.value()->location();
        const auto function_span = start_token.getSpan().merged(end_span).value_or(start_token.getSpan());
        return std::make_unique<WhileStmt>(std::move(condition.value()), std::move(body.value()), function_span);
    }

    std::optional<ExprPtr> Parser::parse_identifier_or_call(const Token &token) {
        auto ident = std::make_unique<Identifier>(std::string{token.getText()}, token.getSpan());
        if(check(TokenKind::OpenParen)) {
            advance();
            return parse_call(std::move(ident), token);
        }
        return ident;
    }

    std::optional<StmtPtr> Parser::parse_for_initializer() {
        if(match_token(TokenKind::Semicolon)) { return nullptr; }

        if(check(TokenKind::KeywordVar) || check(TokenKind::KeywordConst)) {
            const auto &start_token = advance();  // var | const

            auto names = parse_for_var_names(*this);
            if(!names) { return std::nullopt; }

            auto type_annotation = parse_for_type_annotation(*this);
            if(!type_annotation) { return std::nullopt; }

            auto initializers = parse_for_initializers(*this);
            if(!initializers) { return std::nullopt; }

            return make_for_var_decl(start_token, std::move(*names), std::move(*type_annotation), std::move(*initializers));
        }

        auto expr = parse_expr(0);
        if(!expr) {
            syntax_error("Expected expression in for-loop initializer", peek(), std::nullopt, ErrorCode::E1004);
            return std::nullopt;
        }
        const SourceSpan span = expr.value()->location();
        return std::make_unique<ExprStmt>(std::move(expr.value()), span);
    }

    std::optional<StmtPtr> Parser::parse_for() {
        const auto &start_token = advance();  // PERF: reference
        [[maybe_unused]] const auto expect_open = expect(TokenKind::OpenParen, "after 'for'");

        // Parse initializer clause
        auto init_result = parse_for_initializer_clause(*this);
        if(!init_result) { return std::nullopt; }
        // PERF: structured binding on aggregate — no copies, fields moved/read directly
        auto [initializer, initializer_was_empty] = std::move(*init_result);

        // Parse condition clause
        auto condition_opt = parse_for_condition_clause(*this, initializer_was_empty);
        if(!condition_opt) { return std::nullopt; }
        std::optional<ExprPtr> condition = std::move(*condition_opt);

        // Parse increment clause
        auto increment = parse_for_increment_clause(*this);
        if(!increment && !check(TokenKind::CloseParen)) { return std::nullopt; }

        if(!check(TokenKind::CloseParen)) { return std::nullopt; }
        [[maybe_unused]] const auto expect_close = expect(TokenKind::CloseParen, "after for loop clauses");

        auto body_stmt = parse_stmt();
        if(!body_stmt) { return std::nullopt; }

        auto body_ptr = make_for_body_block(std::move(body_stmt.value()));
        const SourceSpan end_span = body_ptr->location();
        const SourceSpan span = start_token.getSpan().merged(end_span).value_or(start_token.getSpan());

        StmtPtr initializer_ptr = initializer ? std::move(*initializer) : nullptr;
        ExprPtr condition_ptr = condition ? std::move(*condition) : nullptr;
        ExprPtr increment_ptr = increment ? std::move(*increment) : nullptr;

        return std::make_unique<ForStmt>(std::move(initializer_ptr), std::move(condition_ptr), std::move(increment_ptr),
                                         std::move(body_ptr), span);
    }

    std::optional<StmtPtr> Parser::parse_break() {
        const SourceSpan span = advance().getSpan();
        return std::make_unique<BreakStmt>(span);
    }

    std::optional<StmtPtr> Parser::parse_continue() {
        const SourceSpan span = advance().getSpan();
        return std::make_unique<ContinueStmt>(span);
    }

    std::optional<StmtPtr> Parser::parse_block_stmt() {
        const auto &start_token = advance();  // PERF: reference
        std::vector<StmtPtr> statements;
        while(!check(TokenKind::CloseBrace) && !is_at_end()) {
            if(auto stmt = parse_stmt()) {
                statements.push_back(std::move(stmt.value()));
            } else {
                if(!is_at_end()) { advance(); }
            }
        }
        [[maybe_unused]] const auto expect_brace = expect(TokenKind::CloseBrace, "end of block");
        return std::make_unique<BlockStmt>(std::move(statements), start_token.getSpan());
    }

    std::optional<StmtPtr> Parser::parse_expression_stmt() {
        auto expr = parse_expr(0);
        if(!expr) {
            syntax_error("Expected expression statement", peek(), std::nullopt, ErrorCode::E1004);
            if(!is_at_end()) { advance(); }
            return std::nullopt;
        }
        match_token(TokenKind::Semicolon);
        const SourceSpan span = expr.value()->location();
        return std::make_unique<ExprStmt>(std::move(expr.value()), span);
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
        if(!left) { return std::nullopt; }
        while(true) {
            const auto &token = peek();
            const auto [lbp, rbp_unused] = binding_power(token);
            if(lbp <= min_bp) { break; }
            auto new_left = led(std::move(*left));
            left = std::nullopt;
            if(!new_left) { break; }
            left = std::move(new_left);
        }
        return left;
    }

    std::optional<ExprPtr> Parser::nud() {
        const auto &token = advance();
        switch(token.getKind()) {
        case TokenKind::Numeric:
            {
                const auto [value, type_suffix] = parse_numeric_literal(token.getText());
                return std::make_unique<IntegerLiteral>(value, token.getSpan(), type_suffix);
            }
        case TokenKind::KeywordBool:
            {
                const bool value = (token.getText() == "true");
                return std::make_unique<BoolLiteral>(value, token.getSpan());
            }
        case TokenKind::KeywordNullptr:
            return std::make_unique<NullLiteral>(token.getSpan());
        case TokenKind::StringLiteral:
            return std::make_unique<StringLiteral>(std::string{token.getText()}, token.getSpan());
        case TokenKind::CharLiteral:
            {
                const auto text = token.getText();
                const char value = text.empty() ? '\0' : text[0];
                return std::make_unique<CharLiteral>(value, token.getSpan());
            }
        case TokenKind::Minus:
            return parse_unary(UnaryOp::Negate, token);
        case TokenKind::Not:
            return parse_unary(UnaryOp::Not, token);
        case TokenKind::PlusPlus:
            return parse_unary(UnaryOp::PreInc, token);
        case TokenKind::MinusMinus:
            return parse_unary(UnaryOp::PreDec, token);
        case TokenKind::OpenBrace:
            return parse_array_literal(token);
        case TokenKind::OpenParen:
            return parse_grouping(token);
        case TokenKind::IdentifierAscii:
        case TokenKind::IdentifierUnicode:
            return parse_identifier_or_call(token);
        default:
            syntax_error("Unexpected token", token, "Expected an expression (number, string, variable, or operator)", ErrorCode::E1004);
            return std::nullopt;
        }
    }

    std::optional<ExprPtr> Parser::led(ExprPtr left) {
        const auto &token = advance();  // PERF: reference
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
            return parse_binary(std::move(left), token);
        case TokenKind::Equal:
            return parse_assignment(std::move(left), token);
        case TokenKind::OpenParen:
            return parse_call(std::move(left), token);
        case TokenKind::OpenBracket:
            return parse_array_access(std::move(left), token);
        case TokenKind::PlusPlus:
            return parse_postfix_unary(std::move(left), UnaryOp::PostInc, token);
        case TokenKind::MinusMinus:
            return parse_postfix_unary(std::move(left), UnaryOp::PostDec, token);
        default:
            syntax_error("Unexpected operator", token, "This operator is not supported in this context", ErrorCode::E1004);
            return std::nullopt;
        }
    }

    ExprPtr Parser::parse_unary(const UnaryOp operation, const Token &token) {
        const auto [lbp_unused, rbp] = unary_binding_power(token);
        auto expr = parse_expr(rbp);
        if(!expr) { expr = std::make_unique<NullLiteral>(token.getSpan()); }
        return std::make_unique<UnaryExpr>(operation, std::move(expr.value()), token.getSpan());
    }

    ExprPtr Parser::parse_postfix_unary(ExprPtr operand, const UnaryOp operation, const Token &token) {
        const auto span = operand->location().merged(token.getSpan()).value_or(token.getSpan());
        return std::make_unique<UnaryExpr>(operation, std::move(operand), span);
    }

    void Parser::extract_elements(const TokenKind kind, std::vector<ExprPtr> &elements) {
        while(!check(kind) && !is_at_end()) {
            if(auto expr = parse_expr(0)) { elements.push_back(std::move(expr.value())); }
            if(!match_token(TokenKind::Comma)) { break; }
        }
    }

    std::optional<ExprPtr> Parser::parse_array_literal(const Token &start_token) {
        std::vector<ExprPtr> elements;
        extract_elements(TokenKind::CloseBrace, elements);
        if(!expect(TokenKind::CloseBrace, "end of array literal")) { return std::nullopt; }
        return std::make_unique<ArrayLiteral>(std::move(elements), merged_span(start_token));
    }

    std::optional<ExprPtr> Parser::parse_binary(ExprPtr left, const Token &token) {
        const auto op_result = get_binary_op(token);
        if(!op_result.has_value()) {
            errors_.push_back(op_result.error());
            return std::nullopt;
        }
        const BinaryOp operation = op_result.value();
        const auto [lbp_unused, rbp] = binding_power(token);
        auto right = parse_expr(rbp);
        if(!right) { right = std::make_unique<NullLiteral>(token.getSpan()); }
        const auto span = left->location().merged(right.value()->location()).value_or(token.getSpan());
        return std::make_unique<BinaryExpr>(operation, std::move(left), std::move(right.value()), span);
    }

    std::optional<ExprPtr> Parser::parse_grouping(const Token &start_token) {
        auto expr = parse_expr(0);
        if(!expect(TokenKind::CloseParen, "end of grouping")) { return std::nullopt; }
        if(!expr) { return std::nullopt; }
        return std::make_unique<GroupingExpr>(std::move(expr.value()), merged_span(start_token));
    }

    std::optional<ExprPtr> Parser::parse_assignment(ExprPtr left, const Token &token) {
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

    std::optional<ExprPtr> Parser::parse_call(ExprPtr callee, const Token &start_token) {
        std::vector<ExprPtr> arguments;
        if(!check(TokenKind::CloseParen)) {
            while(!is_at_end()) {
                auto arg = parse_expr(0);
                if(!arg) {
                    syntax_error("Expected expression in function call argument", peek(), std::nullopt, std::nullopt);
                    return std::nullopt;
                }
                arguments.push_back(std::move(*arg));
                if(!match_token(TokenKind::Comma)) { break; }
            }
        }
        if(!expect(TokenKind::CloseParen, "after function call arguments")) { return std::nullopt; }
        // Lo span della chiamata è dal callee (di solito un Identifier) fino all'ultima parentesi
        const SourceSpan call_span = merged_span(start_token);
        return std::make_unique<CallExpr>(std::move(callee), std::move(arguments), call_span);
    }

    std::optional<ExprPtr> Parser::parse_array_access(ExprPtr array, const Token &start_token) {
        auto index = parse_expr(0);
        if(!index) { index = std::make_unique<NullLiteral>(start_token.getSpan()); }
        if(!expect(TokenKind::CloseBracket, "end of array access")) { return std::nullopt; }
        return std::make_unique<IndexExpr>(std::move(array), std::move(index.value()), merged_span(start_token));
    }

    bool Parser::expect(const TokenKind kind, const std::string_view context) {
        if(match_token(kind)) { return true; }

        const Token *const current_token_ptr = is_at_end() ? nullptr : &peek();

        const std::string found_str = current_token_ptr ? std::string{tokenKindToString(current_token_ptr->getKind())} : "end of input";
        const SourceSpan span = current_token_ptr ? current_token_ptr->getSpan() : SourceSpan{};
        const auto help_message = FORMAT("Try adding a {}", tokenKindToString(kind));

        errors_.push_back(CompileError::SyntaxError(
            ErrorCode::E1004, FORMAT("Expected {} in {}, found {}.", tokenKindToString(kind), context, found_str), span, help_message));
        return false;
    }

    void Parser::syntax_error(const std::string_view message, const Token &token, std::optional<std::string> help,
                              std::optional<ErrorCode> error_code) {
        errors_.push_back(CompileError::SyntaxError(error_code, message, token.getSpan(), std::move(help)));
    }

    void Parser::report_peek_error(const std::string_view message, const std::optional<std::string> &help) {
        if(!is_at_end()) { syntax_error(message, peek(), help, ErrorCode::E1004); }
    }

    bool Parser::check_recursion_limit() {
        if(recursion_depth_ > MAX_RECURSION_DEPTH) {
            if(!is_at_end()) {
                syntax_error("Maximum recursion depth exceeded", peek(), "Simplify the expression or break it into smaller parts",
                             ErrorCode::E1001);
            }
            return true;
        }
        return false;
    }

    void Parser::enter_recursion() noexcept { ++recursion_depth_; }

    void Parser::exit_recursion() noexcept {
        if(recursion_depth_ > 0) { --recursion_depth_; }
    }
    SourceSpan Parser::calculate_return_span(const Token &start, const std::optional<ExprPtr> &value) {
        if(value.has_value()) { return start.getSpan().merged(value.value()->location()).value_or(start.getSpan()); }
        return start.getSpan();
    }
    SourceSpan Parser::merged_span(const Token &start_token) const noexcept {
        return start_token.getSpan().merged(previous().getSpan()).value_or(start_token.getSpan());
    }

    std::optional<std::string_view> Parser::consume_identifier() {
        const auto &token = peek();
        if(token.getKind() == TokenKind::IdentifierAscii || token.getKind() == TokenKind::IdentifierUnicode) {
            advance();               // move past the identifier
            return token.getText();  // token reference still valid — tokens_ is stable
        }
        syntax_error("Expected identifier", peek(), "Provide a valid variable or function name", ErrorCode::E1005);
        return std::nullopt;
    }

    std::pair<std::optional<TypePtr>, std::optional<std::string>> Parser::parse_type() {
        const auto &token = advance();  // PERF: reference
        TypePtr base_type;
        std::string type_string;
        switch(token.getKind()) {
        case TokenKind::TypeI8:
            base_type = PrimitiveType::i8();
            type_string = "i8";
            break;
        case TokenKind::TypeI16:
            base_type = PrimitiveType::i16();
            type_string = "i16";
            break;
        case TokenKind::TypeI32:
            base_type = PrimitiveType::i32();
            type_string = "i32";
            break;
        case TokenKind::TypeI64:
            base_type = PrimitiveType::i64();
            type_string = "i64";
            break;
        case TokenKind::TypeU8:
            base_type = PrimitiveType::u8();
            type_string = "u8";
            break;
        case TokenKind::TypeU16:
            base_type = PrimitiveType::u16();
            type_string = "u16";
            break;
        case TokenKind::TypeU32:
            base_type = PrimitiveType::u32();
            type_string = "u32";
            break;
        case TokenKind::TypeU64:
            base_type = PrimitiveType::u64();
            type_string = "u64";
            break;
        case TokenKind::TypeF32:
            base_type = PrimitiveType::f32();
            type_string = "f32";
            break;
        case TokenKind::TypeF64:
            base_type = PrimitiveType::f64();
            type_string = "f64";
            break;
        case TokenKind::TypeChar:
            base_type = PrimitiveType::char_();
            type_string = "char";
            break;
        case TokenKind::TypeString:
            base_type = PrimitiveType::string();
            type_string = "string";
            break;
        case TokenKind::TypeBool:
            base_type = PrimitiveType::bool_();
            type_string = "bool";
            break;
        case TokenKind::IdentifierAscii:
        case TokenKind::IdentifierUnicode:
            base_type = std::make_shared<const CustomType>(token.getText());
            type_string = std::string(token.getText());
            break;
        default:
            syntax_error("Invalid type specification, expected primitive type or custom identifier", token,
                         "Try using a primitive type (like i32, f64) or a custom type identifier", ErrorCode::E1002);
            return {std::nullopt, std::nullopt};
        }

        std::vector<std::size_t> dimensions;
        while(match_token(TokenKind::OpenBracket)) {
            const auto &dim_token = peek();  // PERF: reference
            if(dim_token.getKind() == TokenKind::Numeric) {
                const auto [dim_value, dim_suffix_unused] = parse_numeric_literal(dim_token.getText());
                if(dim_value <= 0) {
                    syntax_error("Array dimension must be positive", dim_token, "Provide a positive integer greater than 0",
                                 ErrorCode::E1002);
                    break;
                }
                dimensions.push_back(static_cast<std::size_t>(dim_value));
                advance();
            } else {
                syntax_error("Expected array dimension size", dim_token, "Array dimensions must be positive integers", ErrorCode::E1002);
                break;
            }
            if(!expect(TokenKind::CloseBracket, "end of array dimension")) { break; }
        }

        TypePtr result_type = base_type;
        if(!dimensions.empty()) {
            for(const auto dim : dimensions) {
                auto size_literal = std::make_shared<IntegerLiteral>(static_cast<std::int64_t>(dim), token.getSpan(), std::nullopt);
                result_type = std::make_shared<const ArrayType>(result_type, size_literal);
            }
            type_string = result_type->to_string();
        }

        return {result_type, std::move(type_string)};
    }

}  // namespace jsv
// NOLINTEND(*-include-cleaner, *-identifier-length)
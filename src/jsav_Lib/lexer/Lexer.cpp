/*
 * Created by gbian on 28/02/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner,*-identifier-length,*-avoid-magic-numbers,*-magic-numbers)

#include "jsav/lexer/Lexer.hpp"
#include "jsav/lexer/unicode/UnicodeData.hpp"
#include "jsav/lexer/unicode/Utf8.hpp"
namespace jsv {
    Lexer::Lexer(std::string_view source, std::string file_path)
      : m_source{source}, m_file_path{MAKE_SHARED(const std::string, vnd_move(file_path))} {}

    std::vector<Token> Lexer::tokenize() {
        std::vector<Token> tokens;
        tokens.reserve(m_source.size() / 4);  // rough estimate
        // Skip UTF-8 BOM (0xEF 0xBB 0xBF) at start of input if present (FR-019)
        if(m_source.size() >= 3 && C_UC(m_source[0]) == 0xEFU && C_UC(m_source[1]) == 0xBBU && C_UC(m_source[2]) == 0xBFU) {
            m_pos += 3;
            m_column += 3;
        }
        while(true) {
            auto tok = next_token();
            const bool done = (tok.getKind() == TokenKind::Eof);
            tokens.emplace_back(tok);
            if(done) { break; }
        }
        return tokens;
    }

    Token Lexer::next_token() {
        skip_whitespace_and_comments();

        if(is_at_end()) {
            const auto loc = current_location();
            return make_token(TokenKind::Eof, "", loc);
        }

        const auto start = current_location();
        const auto first = C_UC(peek_byte());

        // ── Numeric literal ──────────────────────────────────────────────
        if(std::isdigit(first) != 0) { return scan_numeric_literal(start); }

        // ── Leading-dot numeric: .5, .14, .0 (dot followed by digit) ────
        if(first == '.' && std::isdigit(C_UC(peek_byte(1))) != 0) { return scan_numeric_literal(start); }

        // ── Hash-prefixed numeric (#b, #o, #x) ──────────────────────────
        if(first == '#') { return scan_hash_numeric(start); }

        // ── String / char literals ───────────────────────────────────────
        if(first == '"') { return scan_string_literal(start); }
        if(first == '\'') { return scan_char_literal(start); }

        // ── ASCII identifier / keyword ───────────────────────────────────
        if((std::isalpha(first) != 0) || first == '_') { return scan_identifier_or_keyword(start, false); }

        // ── Non-ASCII: try Unicode identifier start ────────────────────────────
        if(first > 0x7F) {
            const auto res = unicode::decode_utf8(m_source, m_pos);
            if(res.status == unicode::Utf8Status::Ok && unicode::is_id_start(res.codepoint)) {
                return scan_identifier_or_keyword(start, true);
            }
        }

        // ── Operators / punctuation ──────────────────────────────────────
        return scan_operator_or_punctuation(start);
    }
    bool Lexer::is_at_end() const noexcept { return m_pos >= m_source.size(); }

    char Lexer::peek_byte(const std::size_t offset) const noexcept {
        const auto idx = m_pos + offset;
        return (idx < m_source.size()) ? m_source[idx] : '\0';
    }

    char Lexer::advance_byte() noexcept {
        const char c = m_source[m_pos++];
        ++m_column;
        return c;
    }
    char32_t Lexer::peek_codepoint() const noexcept {
        if(is_at_end()) { return U'\0'; }
        return unicode::decode_utf8(m_source, m_pos).codepoint;
    }
    char32_t Lexer::advance_codepoint() noexcept {
        const auto res = unicode::decode_utf8(m_source, m_pos);

        if(res.codepoint == U'\n') {
            m_pos += res.byte_length;
            ++m_line;
            m_column = 1;
        } else {
            m_pos += res.byte_length;
            m_column += res.byte_length;  // byte-based column counter
        }
        return res.codepoint;
    }

    void Lexer::advance_with_utf8_check(bool &has_malformed) noexcept {
        const auto res = unicode::decode_utf8(m_source, m_pos);
        if(res.status != unicode::Utf8Status::Ok) { has_malformed = true; }
        m_pos += res.byte_length;
        m_column += res.byte_length;
    }

    SourceLocation Lexer::current_location() const noexcept { return SourceLocation{m_line, m_column, m_pos}; }

    SourceSpan Lexer::make_span(const SourceLocation &start) const { return SourceSpan{m_file_path, start, current_location()}; }

    Token Lexer::make_token(const TokenKind kind, const std::string_view text, const SourceLocation &start) const {
        return Token{kind, text, make_span(start)};
    }

    Token Lexer::error_token(const std::string_view text, const SourceLocation &start) const {
        return make_token(TokenKind::Error, text, start);
    }

    std::string_view Lexer::current_text(const std::size_t text_start) const noexcept {
        return m_source.substr(text_start, m_pos - text_start);
    }

    std::optional<Token> Lexer::try_two_char_token(const char c1, const char expected, const TokenKind kind, const std::size_t text_start,
                                                   const SourceLocation &start) {
        if(c1 == expected) {
            advance_byte();
            return make_token(kind, current_text(text_start), start);
        }
        return std::nullopt;
    }

    // =========================================================================
    // Whitespace & comments
    // =========================================================================

    bool Lexer::skip_unicode_whitespace() noexcept {
        const auto res = unicode::decode_utf8(m_source, m_pos);
        if(res.status != unicode::Utf8Status::Ok) { return false; }

        // NEL (U+0085) is whitespace + line terminator (not in Zs/Zl/Zp categories)
        if(res.codepoint == U'\u0085') {
            m_pos += res.byte_length;
            ++m_line;
            m_column = 1;
            return true;
        }

        if(!unicode::is_unicode_whitespace(res.codepoint)) { return false; }

        // Line Separator / Paragraph Separator count as newlines
        if(unicode::is_unicode_line_terminator(res.codepoint)) {
            m_pos += res.byte_length;
            ++m_line;
            m_column = 1;
        } else {
            m_pos += res.byte_length;
            m_column += res.byte_length;
        }
        return true;
    }

    void Lexer::skip_block_comment() {
        advance_byte();  // /
        advance_byte();  // *
        while(!is_at_end()) {
            if(peek_byte() == '*' && peek_byte(1) == '/') {
                advance_byte();  // *
                advance_byte();  // /
                break;
            }
            advance_codepoint();
        }
    }

    void Lexer::skip_whitespace_and_comments() {
        while(!is_at_end()) {
            const char c = peek_byte();

            // Plain whitespace (ASCII: space, tab, CR, VT, FF)
            if(is_ascii_horizontal_space(c)) {
                advance_byte();
                continue;
            }
            if(c == '\n') {
                advance_codepoint();  // handles line/column reset
                continue;
            }

            // Non-ASCII: check for Unicode whitespace (Zs, Zl, Zp categories) per FR-023
            if(C_UC(c) > 0x7FU) {
                if(skip_unicode_whitespace()) { continue; }
                break;  // non-whitespace non-ASCII — let next_token() handle it
            }

            // Line comment: // …
            if(c == '/' && peek_byte(1) == '/') {
                advance_byte();
                advance_byte();
                while(!is_at_end() && peek_byte() != '\n') { advance_byte(); }
                continue;
            }

            // Block comment: /* … */  (non-nested)
            if(c == '/' && peek_byte(1) == '*') {
                skip_block_comment();
                continue;
            }

            break;
        }
    }

    // =========================================================================
    // Identifier / keyword scanner
    // =========================================================================

    Token Lexer::scan_identifier_or_keyword(const SourceLocation &start, bool seen_unicode) {
        const auto text_start = m_pos;

        while(!is_at_end()) {
            if(const auto first = C_UC(peek_byte()); first < 0x80) {
                // ASCII fast path
                if((std::isalnum(first) != 0) || first == '_') {
                    advance_byte();
                } else {
                    break;
                }
            } else {
                // Non-ASCII: decode and check XID_Continue
                if(const auto cp = peek_codepoint(); unicode::is_id_continue(cp)) {
                    seen_unicode = true;
                    advance_codepoint();
                } else {
                    break;
                }
            }
        }

        const auto text = m_source.substr(text_start, m_pos - text_start);
        const auto kind = classify_word(text);

        if(kind == TokenKind::IdentifierAscii && seen_unicode) { return make_token(TokenKind::IdentifierUnicode, text, start); }
        return make_token(kind, text, start);
    }

    // =========================================================================
    // Numeric literal scanner
    // =========================================================================

    void Lexer::try_scan_exponent() {
        // Save position for potential rollback (R4: non-destructive lookahead)
        const auto saved_pos = m_pos;
        const auto saved_col = m_column;

        // Consume 'e' or 'E'
        if(is_at_end() || (peek_byte() != 'e' && peek_byte() != 'E')) { return; }
        advance_byte();

        // Consume optional sign
        if(!is_at_end() && (peek_byte() == '+' || peek_byte() == '-')) { advance_byte(); }

        // Consume mandatory digits
        if(is_at_end() || std::isdigit(C_UC(peek_byte())) == 0) {
            // Incomplete exponent: rollback to saved position
            m_pos = saved_pos;
            m_column = saved_col;
            return;
        }

        // Valid exponent: consume all digits
        while(!is_at_end() && std::isdigit(C_UC(peek_byte())) != 0) { advance_byte(); }
    }
    bool Lexer::try_scan_width(const std::initializer_list<char> digits) {
        std::size_t off = 1;
        for(const char d : digits) {
            if(peek_byte(off) != d) { return false; }
            ++off;
        }
        // Width must not be followed by another digit (FR-017)
        if(std::isdigit(C_UC(peek_byte(off))) != 0) { return false; }
        for(std::size_t i = 0; i <= digits.size(); ++i) { advance_byte(); }
        return true;
    }

    void Lexer::try_scan_type_suffix() {
        if(is_at_end()) { return; }
        const char s = peek_byte();

        if(s == 'd' || s == 'D' || s == 'f' || s == 'F') {
            advance_byte();
            return;
        }

        if(s == 'u' || s == 'U' || s == 'i' || s == 'I') {
            if(is_at_end() || std::isdigit(C_UC(peek_byte(1))) == 0) {
                // Bare u/U/i/I — not consumed (FR-015)
                return;
            }

            // Longest-match first to avoid partial consumption (32 before 3, etc.)
            if(try_scan_width({'3', '2'})) { return; }
            if(try_scan_width({'1', '6'})) { return; }
            if(try_scan_width({'8'})) { return; }
            // Invalid width (e.g. 64, 80, 999) — do NOT consume anything
        }
    }

    // NOLINTBEGIN(readability-function-cognitive-complexity)
    Token Lexer::scan_numeric_literal(const SourceLocation &start) {
        const auto text_start = m_pos;

        // ── G1: Numeric part (mandatory) ────────────────────────────────────
        // Branch A: starts with digit (e.g., 42, 3., 3.14)
        // Branch B: starts with dot followed by digit (e.g., .5, .14) - handled by next_token()
        if(std::isdigit(C_UC(peek_byte())) != 0) {
            // Consume integer digits
            while(!is_at_end() && std::isdigit(C_UC(peek_byte())) != 0) { advance_byte(); }

            // Consume optional trailing dot (FR-003: trailing dot IS included)
            if(!is_at_end() && peek_byte() == '.') {
                advance_byte();
                // Consume fractional digits (optional)
                while(!is_at_end() && std::isdigit(C_UC(peek_byte())) != 0) { advance_byte(); }
            }
        } else if(peek_byte() == '.' && !is_at_end() && std::isdigit(C_UC(peek_byte(1))) != 0) {
            // Branch B: leading dot followed by digits
            advance_byte();  // consume '.'
            while(!is_at_end() && std::isdigit(C_UC(peek_byte())) != 0) { advance_byte(); }
        }

        // ── G2: Optional exponent ───────────────────────────────────────────
        try_scan_exponent();

        // ── G3: Optional type suffix ────────────────────────────────────────
        try_scan_type_suffix();

        return make_token(TokenKind::Numeric, m_source.substr(text_start, m_pos - text_start), start);
    }
    // NOLINTEND(readability-function-cognitive-complexity)

    constexpr bool Lexer::is_binary_digit(const char c) noexcept { return c == '0' || c == '1'; }

    constexpr bool Lexer::is_octal_digit(const char c) noexcept { return c >= '0' && c <= '7'; }

    bool Lexer::is_hex_digit(const char c) noexcept { return std::isxdigit(C_UC(c)) != 0; }

    // =========================================================================
    // Hash-prefixed numeric scanner  (#b, #o, #x)
    // =========================================================================
    template <typename IsDigit>
    Token Lexer::scan_based_literal(const std::size_t text_start, const SourceLocation &start, const TokenKind kind, IsDigit is_digit) {
        if(is_at_end() || !is_digit(peek_byte())) { return error_token(m_source.substr(text_start, m_pos - text_start), start); }
        while(!is_at_end() && (is_digit(peek_byte()) || peek_byte() == '_')) { advance_byte(); }
        if(!is_at_end() && (peek_byte() == 'u' || peek_byte() == 'U') && (std::isalnum(C_UC(peek_byte(1))) == 0)) { advance_byte(); }
        return make_token(kind, m_source.substr(text_start, m_pos - text_start), start);
    }

    // NOLINTBEGIN(readability-function-cognitive-complexity)
    Token Lexer::scan_hash_numeric(const SourceLocation &start) {
        const auto text_start = m_pos;
        advance_byte();  // consume '#'

        if(is_at_end()) { return error_token(m_source.substr(text_start, m_pos - text_start), start); }

        const char tag = peek_byte();
        advance_byte();  // consume tag

        switch(tag) {
        case 'b':
            return scan_based_literal(text_start, start, TokenKind::Binary, is_binary_digit);
        case 'o':
            return scan_based_literal(text_start, start, TokenKind::Octal, is_octal_digit);
        case 'x':
            return scan_based_literal(text_start, start, TokenKind::Hexadecimal, is_hex_digit);
        default:
            return error_token(m_source.substr(text_start, m_pos - text_start), start);
        }
    }
    // NOLINTEND(readability-function-cognitive-complexity)

    // =========================================================================
    // String / char literal scanners
    // =========================================================================

    void Lexer::skip_escape() {
        if(is_at_end()) { return; }
        // Unicode escapes consume additional hex digits
        if(const char c = advance_byte(); c == 'u') {
            for(int i = 0; i < 4 && !is_at_end() && (std::isxdigit(C_UC(peek_byte())) != 0); ++i) { advance_byte(); }
        } else if(c == 'U') {
            for(int i = 0; i < 8 && !is_at_end() && (std::isxdigit(C_UC(peek_byte())) != 0); ++i) { advance_byte(); }
        }
        // All other escapes (\\, \n, \t, \r, \", \', \0) fully consumed above.
    }

    Token Lexer::scan_string_literal(const SourceLocation &start) {
        const auto text_start = m_pos;
        advance_byte();  // opening '"'
        bool has_malformed = false;

        while(!is_at_end()) {
            const char c = peek_byte();
            if(c == '"') {
                advance_byte();  // closing '"'
                break;
            }
            if(c == '\\') {
                advance_byte();  // '\'
                skip_escape();
                continue;
            }
            if(c == '\n' || c == '\r') {
                // Unterminated single-line string — stop and let the parser reject.
                break;
            }
            // For non-ASCII bytes, validate the UTF-8 sequence (FR-021)
            if(C_UC(c) > 0x7F) {
                advance_with_utf8_check(has_malformed);
            } else {
                advance_byte();
            }
        }

        const auto text = m_source.substr(text_start, m_pos - text_start);
        if(has_malformed) { return error_token(text, start); }
        return make_token(TokenKind::StringLiteral, text, start);
    }

    Token Lexer::scan_char_literal(const SourceLocation &start) {
        const auto text_start = m_pos;
        advance_byte();  // opening '\''
        bool has_malformed = false;

        if(!is_at_end()) {
            if(peek_byte() == '\\') {
                advance_byte();  // '\'
                skip_escape();
            } else {
                // For non-ASCII bytes, validate the UTF-8 sequence (FR-021)
                const char c = peek_byte();
                if(C_UC(c) > 0x7F) {
                    advance_with_utf8_check(has_malformed);
                } else {
                    advance_byte();
                }
            }
        }

        if(!is_at_end() && peek_byte() == '\'') { advance_byte(); }  // closing '\''

        const auto text = m_source.substr(text_start, m_pos - text_start);
        if(has_malformed) { return error_token(text, start); }
        return make_token(TokenKind::CharLiteral, text, start);
    }

    // =========================================================================
    // Operator / punctuation scanner
    // =========================================================================

    // NOLINTBEGIN(readability-function-cognitive-complexity)
    Token Lexer::scan_operator_or_punctuation(const SourceLocation &start) {
        const auto text_start = m_pos;
        const char c0 = advance_byte();
        const char c1 = peek_byte();

        switch(c0) {
        case '+':
            if(auto t = try_two_char_token(c1, '=', TokenKind::PlusEqual, text_start, start)) { return *t; }
            if(auto t = try_two_char_token(c1, '+', TokenKind::PlusPlus, text_start, start)) { return *t; }
            return make_token(TokenKind::Plus, current_text(text_start), start);

        case '-':
            if(auto t = try_two_char_token(c1, '=', TokenKind::MinusEqual, text_start, start)) { return *t; }
            if(auto t = try_two_char_token(c1, '-', TokenKind::MinusMinus, text_start, start)) { return *t; }
            return make_token(TokenKind::Minus, current_text(text_start), start);

        case '=':
            if(auto t = try_two_char_token(c1, '=', TokenKind::EqualEqual, text_start, start)) { return *t; }
            return make_token(TokenKind::Equal, current_text(text_start), start);

        case '!':
            if(auto t = try_two_char_token(c1, '=', TokenKind::NotEqual, text_start, start)) { return *t; }
            return make_token(TokenKind::Not, current_text(text_start), start);

        case '<':
            if(auto t = try_two_char_token(c1, '=', TokenKind::LessEqual, text_start, start)) { return *t; }
            if(auto t = try_two_char_token(c1, '<', TokenKind::ShiftLeft, text_start, start)) { return *t; }
            return make_token(TokenKind::Less, current_text(text_start), start);

        case '>':
            if(auto t = try_two_char_token(c1, '=', TokenKind::GreaterEqual, text_start, start)) { return *t; }
            if(auto t = try_two_char_token(c1, '>', TokenKind::ShiftRight, text_start, start)) { return *t; }
            return make_token(TokenKind::Greater, current_text(text_start), start);

        case '|':
            if(auto t = try_two_char_token(c1, '|', TokenKind::OrOr, text_start, start)) { return *t; }
            return make_token(TokenKind::Or, current_text(text_start), start);

        case '&':
            if(auto t = try_two_char_token(c1, '&', TokenKind::AndAnd, text_start, start)) { return *t; }
            return make_token(TokenKind::And, current_text(text_start), start);

        case '%':
            if(auto t = try_two_char_token(c1, '=', TokenKind::PercentEqual, text_start, start)) { return *t; }
            return make_token(TokenKind::Percent, current_text(text_start), start);

        case '^':
            if(auto t = try_two_char_token(c1, '=', TokenKind::XorEqual, text_start, start)) { return *t; }
            return make_token(TokenKind::Xor, current_text(text_start), start);

        case '*':
            return make_token(TokenKind::Star, current_text(text_start), start);
        case '/':
            return make_token(TokenKind::Slash, current_text(text_start), start);
        case ':':
            return make_token(TokenKind::Colon, current_text(text_start), start);
        case ',':
            return make_token(TokenKind::Comma, current_text(text_start), start);
        case '.':
            return make_token(TokenKind::Dot, current_text(text_start), start);
        case ';':
            return make_token(TokenKind::Semicolon, current_text(text_start), start);
        case '(':
            return make_token(TokenKind::OpenParen, current_text(text_start), start);
        case ')':
            return make_token(TokenKind::CloseParen, current_text(text_start), start);
        case '[':
            return make_token(TokenKind::OpenBracket, current_text(text_start), start);
        case ']':
            return make_token(TokenKind::CloseBracket, current_text(text_start), start);
        case '{':
            return make_token(TokenKind::OpenBrace, current_text(text_start), start);
        case '}':
            return make_token(TokenKind::CloseBrace, current_text(text_start), start);

        default:
            // Gracefully consume unknown UTF-8 sequences (first byte already advanced).
            if(C_UC(c0) > 0x7F) {
                const auto seq = unicode::decode_utf8(m_source, text_start);
                for(std::size_t i = 1; i < seq.byte_length && !is_at_end(); ++i) { advance_byte(); }
            }
            return error_token(current_text(text_start), start);
        }
    }
    // NOLINTEND(readability-function-cognitive-complexity)

    // =========================================================================
    // Keyword / type classification
    // =========================================================================

    TokenKind Lexer::classify_word(const std::string_view text) noexcept {
        using namespace std::string_view_literals;

        static const std::unordered_map<std::string_view, TokenKind> kTable{
            // Keywords
            {"fun"sv, TokenKind::KeywordFun},
            {"if"sv, TokenKind::KeywordIf},
            {"else"sv, TokenKind::KeywordElse},
            {"return"sv, TokenKind::KeywordReturn},
            {"while"sv, TokenKind::KeywordWhile},
            {"for"sv, TokenKind::KeywordFor},
            {"main"sv, TokenKind::KeywordMain},
            {"var"sv, TokenKind::KeywordVar},
            {"const"sv, TokenKind::KeywordConst},
            {"nullptr"sv, TokenKind::KeywordNullptr},
            {"break"sv, TokenKind::KeywordBreak},
            {"continue"sv, TokenKind::KeywordContinue},
            {"bool"sv, TokenKind::KeywordBool},
            // Primitive types
            {"i8"sv, TokenKind::TypeI8},
            {"i16"sv, TokenKind::TypeI16},
            {"i32"sv, TokenKind::TypeI32},
            {"i64"sv, TokenKind::TypeI64},
            {"u8"sv, TokenKind::TypeU8},
            {"u16"sv, TokenKind::TypeU16},
            {"u32"sv, TokenKind::TypeU32},
            {"u64"sv, TokenKind::TypeU64},
            {"f32"sv, TokenKind::TypeF32},
            {"f64"sv, TokenKind::TypeF64},
            {"char"sv, TokenKind::TypeChar},
            {"string"sv, TokenKind::TypeString},
        };

        const auto it = kTable.find(text);
        return (it != kTable.end()) ? it->second : TokenKind::IdentifierAscii;
    }

}  // namespace jsv
// NOLINTEND(*-include-cleaner,*-identifier-length,*-avoid-magic-numbers,*-magic-numbers)
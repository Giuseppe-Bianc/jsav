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
            if(c == ' ' || c == '\t' || c == '\r' || c == '\v' || c == '\f') {
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
            const auto first = C_UC(peek_byte());

            if(first < 0x80) {
                // ASCII fast path
                if((std::isalnum(first) != 0) || first == '_') {
                    advance_byte();
                } else {
                    break;
                }
            } else {
                // Non-ASCII: decode and check XID_Continue
                const auto cp = peek_codepoint();
                if(unicode::is_id_continue(cp)) {
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

    // NOLINTBEGIN(readability-function-cognitive-complexity)
    Token Lexer::scan_numeric_literal(const SourceLocation &start) {
        const auto text_start = m_pos;

        // ── Integer part (underscore separators allowed) ──────────────────────
        while(!is_at_end() && (std::isdigit(C_UC(peek_byte())) != 0 || peek_byte() == '_')) { advance_byte(); }

        // ── Optional fractional part ──────────────────────────────────────────
        // Only consumed when '.' is IMMEDIATELY followed by a decimal digit.
        // "123." is intentionally split into Numeric("123") + Dot(".") so that
        // trailing-dot method calls (e.g. 123.toString()) parse correctly,
        // matching the behaviour of Rust, Kotlin, and Swift.
        if(!is_at_end() && peek_byte() == '.' && (std::isdigit(C_UC(peek_byte(1))) != 0)) {
            advance_byte();  // '.'
            while(!is_at_end() && (std::isdigit(C_UC(peek_byte())) != 0 || peek_byte() == '_')) { advance_byte(); }
        }

        // ── Optional exponent: e/E, optional sign, digits ─────────────────────
        if(!is_at_end() && (peek_byte() == 'e' || peek_byte() == 'E')) {
            advance_byte();  // 'e' / 'E'
            if(!is_at_end() && (peek_byte() == '+' || peek_byte() == '-')) {
                advance_byte();  // sign
            }
            while(!is_at_end() && (std::isdigit(C_UC(peek_byte())) != 0)) { advance_byte(); }
        }

        // ── Optional type suffix ──────────────────────────────────────────────
        // Recognised patterns (must immediately follow the number):
        //   i8  i16  i32  i64   →  'i' + one-or-more digits
        //   u8  u16  u32  u64   →  'u' + one-or-more digits
        //   f32 f64             →  'f' + one-or-more digits
        //   u   U               →  bare unsigned marker, NOT followed by alnum
        //
        // Only consume the suffix letter when it is either:
        //   (a) 'u'/'U' standing alone (next char is not alnum), or
        //   (b) 'i'/'u'/'f' immediately followed by one or more digits.
        //
        // This prevents "42identifier" from incorrectly eating "identifier":
        //   "42 myVar"  → Numeric("42"), Identifier("myVar")  ✓
        //   "42myVar"   → Numeric("42"), Identifier("myVar")  ✓
        if(!is_at_end()) {
            const char s = peek_byte();
            const char s1 = peek_byte(1);

            const bool bare_unsigned = (s == 'u' || s == 'U') && (std::isalnum(C_UC(s1)) == 0);

            const bool typed_suffix = (s == 'i' || s == 'u' || s == 'f') && (std::isdigit(C_UC(s1)) != 0);

            if(bare_unsigned || typed_suffix) {
                advance_byte();  // suffix letter
                while(!is_at_end() && (std::isdigit(C_UC(peek_byte())) != 0)) { advance_byte(); }
            }
        }

        return make_token(TokenKind::Numeric, m_source.substr(text_start, m_pos - text_start), start);
    }
    // NOLINTEND(readability-function-cognitive-complexity)

    // =========================================================================
    // Hash-prefixed numeric scanner  (#b, #o, #x)
    // =========================================================================

    // NOLINTBEGIN(readability-function-cognitive-complexity)
    Token Lexer::scan_hash_numeric(const SourceLocation &start) {
        const auto text_start = m_pos;
        advance_byte();  // consume '#'

        if(is_at_end()) { return error_token(m_source.substr(text_start, m_pos - text_start), start); }

        const char tag = peek_byte();

        // ── Binary: #b[01_]+ [uU]? ───────────────────────────────────────────
        if(tag == 'b') {
            advance_byte();  // 'b'
            if(is_at_end() || (peek_byte() != '0' && peek_byte() != '1')) {
                // No valid binary digit immediately after prefix → Error.
                // Recovery is local: cursor sits on the bad char so the next
                // next_token() call sees it fresh.
                return error_token(m_source.substr(text_start, m_pos - text_start), start);
            }
            while(!is_at_end() && (peek_byte() == '0' || peek_byte() == '1' || peek_byte() == '_')) { advance_byte(); }
            // Bare unsigned suffix: 'u'/'U' not followed by alnum.
            if(!is_at_end() && (peek_byte() == 'u' || peek_byte() == 'U') && (std::isalnum(C_UC(peek_byte(1))) == 0)) { advance_byte(); }
            return make_token(TokenKind::Binary, m_source.substr(text_start, m_pos - text_start), start);
        }

        // ── Octal: #o[0-7_]+ [uU]? ───────────────────────────────────────────
        if(tag == 'o') {
            advance_byte();  // 'o'
            if(is_at_end() || peek_byte() < '0' || peek_byte() > '7') {
                return error_token(m_source.substr(text_start, m_pos - text_start), start);
            }
            while(!is_at_end() && ((peek_byte() >= '0' && peek_byte() <= '7') || peek_byte() == '_')) { advance_byte(); }
            if(!is_at_end() && (peek_byte() == 'u' || peek_byte() == 'U') && (std::isalnum(C_UC(peek_byte(1))) == 0)) { advance_byte(); }
            return make_token(TokenKind::Octal, m_source.substr(text_start, m_pos - text_start), start);
        }

        // ── Hexadecimal: #x[0-9a-fA-F_]+ [uU]? ─────────────────────────────
        if(tag == 'x') {
            advance_byte();  // 'x'
            if(is_at_end() || (std::isxdigit(C_UC(peek_byte())) == 0)) {
                return error_token(m_source.substr(text_start, m_pos - text_start), start);
            }
            while(!is_at_end() && ((std::isxdigit(C_UC(peek_byte())) != 0) || peek_byte() == '_')) { advance_byte(); }
            // 'u'/'U' are NOT valid hex digits so they are unambiguously a suffix
            // here — they could not have been consumed by the digit loop above.
            if(!is_at_end() && (peek_byte() == 'u' || peek_byte() == 'U') && (std::isalnum(C_UC(peek_byte(1))) == 0)) { advance_byte(); }
            return make_token(TokenKind::Hexadecimal, m_source.substr(text_start, m_pos - text_start), start);
        }

        // '#' followed by an unrecognised tag byte → emit Error for '#' + tag.
        advance_byte();  // consume the unknown byte so we always make forward progress
        return error_token(m_source.substr(text_start, m_pos - text_start), start);
    }
    // NOLINTEND(readability-function-cognitive-complexity)

    // =========================================================================
    // String / char literal scanners
    // =========================================================================

    void Lexer::skip_escape() {
        if(is_at_end()) { return; }
        const char c = advance_byte();  // consume the character after '\'

        // Unicode escapes consume additional hex digits
        if(c == 'u') {
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
        const char c1 = peek_byte();  // lookahead, not yet consumed

        // Consume c1 and return a two-char token if c1 == expected.
        auto two = [&](const char expected, const TokenKind kind) -> std::optional<Token> {
            if(c1 == expected) {
                advance_byte();
                return make_token(kind, m_source.substr(text_start, m_pos - text_start), start);
            }
            return std::nullopt;
        };

        switch(c0) {
        case '+':
            if(auto t = two('=', TokenKind::PlusEqual)) { return *t; }
            if(auto t = two('+', TokenKind::PlusPlus)) { return *t; }
            return make_token(TokenKind::Plus, m_source.substr(text_start, m_pos - text_start), start);

        case '-':
            if(auto t = two('=', TokenKind::MinusEqual)) { return *t; }
            if(auto t = two('-', TokenKind::MinusMinus)) { return *t; }
            return make_token(TokenKind::Minus, m_source.substr(text_start, m_pos - text_start), start);

        case '=':
            if(auto t = two('=', TokenKind::EqualEqual)) { return *t; }
            return make_token(TokenKind::Equal, m_source.substr(text_start, m_pos - text_start), start);

        case '!':
            if(auto t = two('=', TokenKind::NotEqual)) { return *t; }
            return make_token(TokenKind::Not, m_source.substr(text_start, m_pos - text_start), start);

        case '<':
            if(auto t = two('=', TokenKind::LessEqual)) { return *t; }
            if(auto t = two('<', TokenKind::ShiftLeft)) { return *t; }
            return make_token(TokenKind::Less, m_source.substr(text_start, m_pos - text_start), start);

        case '>':
            if(auto t = two('=', TokenKind::GreaterEqual)) { return *t; }
            if(auto t = two('>', TokenKind::ShiftRight)) { return *t; }
            return make_token(TokenKind::Greater, m_source.substr(text_start, m_pos - text_start), start);

        case '|':
            if(auto t = two('|', TokenKind::OrOr)) { return *t; }
            return make_token(TokenKind::Or, m_source.substr(text_start, m_pos - text_start), start);

        case '&':
            if(auto t = two('&', TokenKind::AndAnd)) { return *t; }
            return make_token(TokenKind::And, m_source.substr(text_start, m_pos - text_start), start);

        case '%':
            if(auto t = two('=', TokenKind::PercentEqual)) { return *t; }
            return make_token(TokenKind::Percent, m_source.substr(text_start, m_pos - text_start), start);

        case '^':
            if(auto t = two('=', TokenKind::XorEqual)) { return *t; }
            return make_token(TokenKind::Xor, m_source.substr(text_start, m_pos - text_start), start);

        case '*':
            return make_token(TokenKind::Star, m_source.substr(text_start, m_pos - text_start), start);
        case '/':
            return make_token(TokenKind::Slash, m_source.substr(text_start, m_pos - text_start), start);
        case ':':
            return make_token(TokenKind::Colon, m_source.substr(text_start, m_pos - text_start), start);
        case ',':
            return make_token(TokenKind::Comma, m_source.substr(text_start, m_pos - text_start), start);
        case '.':
            return make_token(TokenKind::Dot, m_source.substr(text_start, m_pos - text_start), start);
        case ';':
            return make_token(TokenKind::Semicolon, m_source.substr(text_start, m_pos - text_start), start);
        case '(':
            return make_token(TokenKind::OpenParen, m_source.substr(text_start, m_pos - text_start), start);
        case ')':
            return make_token(TokenKind::CloseParen, m_source.substr(text_start, m_pos - text_start), start);
        case '[':
            return make_token(TokenKind::OpenBracket, m_source.substr(text_start, m_pos - text_start), start);
        case ']':
            return make_token(TokenKind::CloseBracket, m_source.substr(text_start, m_pos - text_start), start);
        case '{':
            return make_token(TokenKind::OpenBrace, m_source.substr(text_start, m_pos - text_start), start);
        case '}':
            return make_token(TokenKind::CloseBrace, m_source.substr(text_start, m_pos - text_start), start);

        default:
            // Gracefully consume unknown UTF-8 sequences (first byte already advanced).
            if(C_UC(c0) > 0x7F) {
                const auto seq = unicode::decode_utf8(m_source, text_start);
                for(std::size_t i = 1; i < seq.byte_length && !is_at_end(); ++i) { advance_byte(); }
            }
            return error_token(m_source.substr(text_start, m_pos - text_start), start);
        }
    }
    // NOLINTEND(readability-function-cognitive-complexity)

    // =========================================================================
    // Unicode XID classification
    //
    // Covers the scripts most commonly found in source code identifiers.
    // For full conformance, generate lookup tables from:
    //   https://www.unicode.org/Public/UCD/latest/ucd/DerivedCoreProperties.txt
    // =========================================================================

    // =========================================================================
    // Keyword / type classification
    // =========================================================================

    TokenKind Lexer::classify_word(const std::string_view text) noexcept {
        using namespace std::string_view_literals;

        // Keywords
        if(text == "fun"sv) { return TokenKind::KeywordFun; }
        if(text == "if"sv) { return TokenKind::KeywordIf; }
        if(text == "else"sv) { return TokenKind::KeywordElse; }
        if(text == "return"sv) { return TokenKind::KeywordReturn; }
        if(text == "while"sv) { return TokenKind::KeywordWhile; }
        if(text == "for"sv) { return TokenKind::KeywordFor; }
        if(text == "main"sv) { return TokenKind::KeywordMain; }
        if(text == "var"sv) { return TokenKind::KeywordVar; }
        if(text == "const"sv) { return TokenKind::KeywordConst; }
        if(text == "nullptr"sv) { return TokenKind::KeywordNullptr; }
        if(text == "break"sv) { return TokenKind::KeywordBreak; }
        if(text == "continue"sv) { return TokenKind::KeywordContinue; }
        if(text == "bool"sv) { return TokenKind::KeywordBool; }

        // Primitive types
        if(text == "i8"sv) { return TokenKind::TypeI8; }
        if(text == "i16"sv) { return TokenKind::TypeI16; }
        if(text == "i32"sv) { return TokenKind::TypeI32; }
        if(text == "i64"sv) { return TokenKind::TypeI64; }
        if(text == "u8"sv) { return TokenKind::TypeU8; }
        if(text == "u16"sv) { return TokenKind::TypeU16; }
        if(text == "u32"sv) { return TokenKind::TypeU32; }
        if(text == "u64"sv) { return TokenKind::TypeU64; }
        if(text == "f32"sv) { return TokenKind::TypeF32; }
        if(text == "f64"sv) { return TokenKind::TypeF64; }
        if(text == "char"sv) { return TokenKind::TypeChar; }
        if(text == "string"sv) { return TokenKind::TypeString; }

        return TokenKind::IdentifierAscii;
    }

}  // namespace jsv
// NOLINTEND(*-include-cleaner,*-identifier-length,*-avoid-magic-numbers,*-magic-numbers)
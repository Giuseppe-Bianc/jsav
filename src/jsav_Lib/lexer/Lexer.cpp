/*
 * Created by gbian on 28/02/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#include "jsav/lexer/Lexer.hpp"

namespace jsv {
    Lexer::Lexer(std::string_view source, std::string file_path)
      : m_source{source}, m_file_path{MAKE_SHARED(const std::string, vnd_move(file_path))} {}

    std::vector<Token> Lexer::tokenize() {
        std::vector<Token> tokens;
        tokens.reserve(m_source.size() / 4);  // rough estimate
        while(true) {
            auto tok = next_token();
            const bool done = (tok.getKind() == TokenKind::Eof);
            // tokens.push_back(std::move(tok));
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

        // ── Non-ASCII: try Unicode XID_Start ────────────────────────────
        if(first > 0x7F) {
            const auto cp = peek_codepoint();
            if(is_xid_start(cp)) { return scan_identifier_or_keyword(start, true); }
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
    std::size_t Lexer::utf8_byte_len(const unsigned char first_byte) noexcept {
        if((first_byte & 0x80U) == 0x00U) { return 1; }  // 0xxxxxxx
        if((first_byte & 0xE0U) == 0xC0U) { return 2; }  // 110xxxxx
        if((first_byte & 0xF0U) == 0xE0U) { return 3; }  // 1110xxxx
        if((first_byte & 0xF8U) == 0xF0U) { return 4; }  // 11110xxx
        return 1;                                        // invalid continuation — treat as 1
    }
    std::uint32_t Lexer::peek_codepoint() const noexcept {
        if(is_at_end()) { return 0; }

        const auto first = C_UC(m_source[m_pos]);
        const auto len = utf8_byte_len(first);

        if(len == 1) { return first; }
        if(m_pos + len > m_source.size()) { return first; }  // truncated sequence

        // Strip leading marker bits: len=2 → mask 0x1F, len=3 → 0x0F, len=4 → 0x07
        std::uint32_t cp = first & (0xFFU >> (len + 1U));
        for(std::size_t i = 1; i < len; ++i) { cp = (cp << 6U) | (C_UC(m_source[m_pos + i]) & 0x3FU); }
        return cp;
    }
    std::uint32_t Lexer::advance_codepoint() noexcept {
        const auto cp = peek_codepoint();
        const auto len = utf8_byte_len(C_UC(m_source[m_pos]));

        if(cp == '\n') {
            m_pos += len;
            ++m_line;
            m_column = 1;
        } else {
            m_pos += len;
            m_column += len;  // byte-based column counter
        }
        return cp;
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

    void Lexer::skip_whitespace_and_comments() {
        while(!is_at_end()) {
            const char c = peek_byte();

            // Plain whitespace
            if(c == ' ' || c == '\t' || c == '\r') {
                advance_byte();
                continue;
            }
            if(c == '\n') {
                advance_codepoint();  // handles line/column reset
                continue;
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
                if(is_xid_continue(cp)) {
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

    // =========================================================================
    // Hash-prefixed numeric scanner  (#b, #o, #x)
    // =========================================================================

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
            advance_codepoint();
        }

        return make_token(TokenKind::StringLiteral, m_source.substr(text_start, m_pos - text_start), start);
    }

    Token Lexer::scan_char_literal(const SourceLocation &start) {
        const auto text_start = m_pos;
        advance_byte();  // opening '\''

        if(!is_at_end()) {
            if(peek_byte() == '\\') {
                advance_byte();  // '\'
                skip_escape();
            } else {
                advance_codepoint();  // one Unicode scalar value
            }
        }

        if(!is_at_end() && peek_byte() == '\'') { advance_byte(); }  // closing '\''

        return make_token(TokenKind::CharLiteral, m_source.substr(text_start, m_pos - text_start), start);
    }

    // =========================================================================
    // Operator / punctuation scanner
    // =========================================================================

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
                const auto remaining = utf8_byte_len(C_UC(c0));
                for(std::size_t i = 1; i < remaining && !is_at_end(); ++i) { advance_byte(); }
            }
            return error_token(m_source.substr(text_start, m_pos - text_start), start);
        }
    }

    // =========================================================================
    // Unicode XID classification
    //
    // Covers the scripts most commonly found in source code identifiers.
    // For full conformance, generate lookup tables from:
    //   https://www.unicode.org/Public/UCD/latest/ucd/DerivedCoreProperties.txt
    // =========================================================================

    bool Lexer::is_xid_start(const std::uint32_t cp) noexcept {
        if(cp < 0x80) { return (std::isalpha(static_cast<int>(cp)) != 0) || cp == '_'; }

        // Latin-1 Supplement & Latin Extended
        if(cp >= 0x00C0 && cp <= 0x00D6) { return true; }
        if(cp >= 0x00D8 && cp <= 0x00F6) { return true; }
        if(cp >= 0x00F8 && cp <= 0x01F5) { return true; }
        if(cp >= 0x01FA && cp <= 0x0217) { return true; }
        if(cp >= 0x0250 && cp <= 0x02A8) { return true; }

        // Greek
        if(cp >= 0x0370 && cp <= 0x0373) { return true; }
        if(cp >= 0x0376 && cp <= 0x0377) { return true; }
        if(cp >= 0x037B && cp <= 0x037D) { return true; }
        if(cp == 0x037F) { return true; }
        if(cp == 0x0386) { return true; }
        if(cp >= 0x0388 && cp <= 0x038A) { return true; }
        if(cp == 0x038C) { return true; }
        if(cp >= 0x038E && cp <= 0x03A1) { return true; }
        if(cp >= 0x03A3 && cp <= 0x03F5) { return true; }

        // Cyrillic
        if(cp >= 0x0400 && cp <= 0x0481) { return true; }
        if(cp >= 0x048A && cp <= 0x052F) { return true; }

        // Armenian
        if(cp >= 0x0531 && cp <= 0x0556) { return true; }
        if(cp >= 0x0561 && cp <= 0x0587) { return true; }

        // Hebrew
        if(cp >= 0x05D0 && cp <= 0x05EA) { return true; }

        // Arabic
        if(cp >= 0x0620 && cp <= 0x064A) { return true; }
        if(cp >= 0x0671 && cp <= 0x06B7) { return true; }

        // Devanagari
        if(cp >= 0x0905 && cp <= 0x0939) { return true; }
        if(cp == 0x093D) { return true; }

        // Thai
        if(cp >= 0x0E01 && cp <= 0x0E2E) { return true; }

        // Hiragana / Katakana
        if(cp >= 0x3041 && cp <= 0x3094) { return true; }
        if(cp >= 0x30A1 && cp <= 0x30FA) { return true; }

        // CJK Unified Ideographs
        if(cp >= 0x4E00 && cp <= 0x9FFF) { return true; }
        if(cp >= 0x3400 && cp <= 0x4DBF) { return true; }    // Extension A
        if(cp >= 0x20000 && cp <= 0x2A6DF) { return true; }  // Extension B

        // Korean Hangul
        if(cp >= 0xAC00 && cp <= 0xD7A3) { return true; }
        if(cp >= 0x1100 && cp <= 0x1159) { return true; }

        // Mathematical Alphanumeric Symbols
        if(cp >= 0x1D400 && cp <= 0x1D7CB) { return true; }

        return false;
    }

    bool Lexer::is_xid_continue(const std::uint32_t cp) noexcept {
        if(cp < 0x80) { return (std::isalnum(static_cast<int>(cp)) != 0) || cp == '_'; }

        // Combining Diacritical Marks — essential for XID_Continue
        if(cp >= 0x0300 && cp <= 0x036F) { return true; }

        // Devanagari matras / vowel signs
        if(cp >= 0x093E && cp <= 0x094C) { return true; }
        if(cp >= 0x0951 && cp <= 0x0954) { return true; }

        // Arabic digit forms
        if(cp >= 0x0660 && cp <= 0x0669) { return true; }  // Arabic-Indic
        if(cp >= 0x06F0 && cp <= 0x06F9) { return true; }  // Extended Arabic-Indic

        // Devanagari digits
        if(cp >= 0x0966 && cp <= 0x096F) { return true; }

        // Thai digits
        if(cp >= 0x0E50 && cp <= 0x0E59) { return true; }

        // Enclosed Alphanumerics
        if(cp >= 0x24B6 && cp <= 0x24E9) { return true; }

        // Anything that is XID_Start also continues
        return is_xid_start(cp);
    }

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
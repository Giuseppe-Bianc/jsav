/*
 * Created by gbian on 28/02/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

#include "../headers.hpp"
#include "Token.hpp"

namespace jsv {
    [[nodiscard]] static constexpr bool is_ascii_horizontal_space(const char c) noexcept {
        return c == ' ' || c == '\t' || c == '\r' || c == '\v' || c == '\f';
    }
    /// UTF-8 aware lexer that produces a flat stream of `Token`s from source text.
    ///
    /// # Design notes
    /// - Source is held as a non-owning `std::string_view`; callers must keep the
    ///   underlying buffer alive for the lifetime of any produced tokens (since
    ///   `Token::m_text` is also a view into the same buffer).
    /// - Line/column tracking is 1-indexed, column is byte-based (matching
    ///   `SourceLocation` documentation).
    /// - UTF-8 multi-byte sequences are decoded for identifier classification
    ///   (Unicode XID); all other scanning is byte-oriented for performance.
    ///
    /// # Numeric literal syntax
    /// | Kind        | Prefix | Example            |
    /// |-------------|--------|--------------------|
    /// | Decimal     | —      | `42`, `3.14f32`    |
    /// | Binary      | `#b`   | `#b1010u`          |
    /// | Octal       | `#o`   | `#o755`            |
    /// | Hexadecimal | `#x`   | `#xDEAD_BEEFu`     |
    ///
    /// # Trailing-dot rule
    /// `123.` produces `Numeric("123.")` — the trailing dot IS included in the
    /// numeric token. Leading-dot floats (`.456`) produce `Numeric(".456")`.
    /// The jsav language does not support method-call syntax on numeric literals.
    ///
    /// # Comment syntax
    /// - Line comments:  `// …`
    /// - Block comments: `/* … */` (non-nested)
    class Lexer {
    public:
        /// @param source    Complete source text to lex.
        /// @param file_path Path used in diagnostics / span data.
        explicit Lexer(std::string_view source, std::string file_path);

        /// Lex all tokens including the terminating `Eof`.
        [[nodiscard]] std::vector<Token> tokenize();

        /// Produce the next single token from the stream.
        /// After `Eof` is returned, subsequent calls keep returning `Eof`.
        [[nodiscard]] Token next_token();

    private:
        // ── Source state ──────────────────────────────────────────────────
        std::string_view m_source;  ///< Non-owning view of the full input.
        std::size_t m_pos = 0;      ///< Current byte offset (0-indexed).
        std::size_t m_line = 1;     ///< Current line (1-indexed).
        std::size_t m_column = 1;   ///< Current column, byte-based (1-indexed).
        std::shared_ptr<const std::string> m_file_path;

        // ── Navigation ────────────────────────────────────────────────────
        [[nodiscard]] bool is_at_end() const noexcept;

        /// Peek the raw byte at `m_pos + offset` without consuming. Returns '\0' at EOF.
        [[nodiscard]] char peek_byte(std::size_t offset = 0) const noexcept;

        /// Consume one raw byte, incrementing column. Does NOT handle newlines.
        char advance_byte() noexcept;

        // ── UTF-8 helpers ─────────────────────────────────────────────────

        /// Decode the codepoint at `m_pos` without consuming.
        [[nodiscard]] char32_t peek_codepoint() const noexcept;

        /// Decode and consume one UTF-8 codepoint, updating line/column correctly.
        char32_t advance_codepoint() noexcept;

        /// Advance m_pos and m_column by one UTF-8 sequence, marking has_malformed if invalid.
        /// Used in string/char literal scanning to handle non-ASCII bytes.
        void advance_with_utf8_check(bool &has_malformed) noexcept;

        // ── Location / token construction ─────────────────────────────────
        [[nodiscard]] SourceLocation current_location() const noexcept;
        [[nodiscard]] SourceSpan make_span(const SourceLocation &start) const;
        [[nodiscard]] Token make_token(TokenKind kind, std::string_view text, const SourceLocation &start) const;
        [[nodiscard]] Token error_token(std::string_view text, const SourceLocation &start) const;

        /// Return the source slice [text_start, m_pos) as a string_view.
        /// Extracted from the `text` lambda in scan_operator_or_punctuation.
        [[nodiscard]] std::string_view current_text(std::size_t text_start) const noexcept;

        // ── Whitespace / comments ─────────────────────────────────────────
        void skip_whitespace_and_comments();

        /// Handle non-ASCII Unicode whitespace at current position.
        /// Returns true if whitespace was consumed, false if it was not whitespace.
        [[nodiscard]] bool skip_unicode_whitespace() noexcept;

        /// Consume a block comment starting after the opening `/*`.
        void skip_block_comment();

        // ── Scanners ──────────────────────────────────────────────────────
        Token scan_identifier_or_keyword(const SourceLocation &start, bool seen_unicode);
        Token scan_numeric_literal(const SourceLocation &start);
        template <typename IsDigit>
        Token scan_based_literal(const std::size_t text_start, const SourceLocation &start, const TokenKind kind, IsDigit is_digit);
        Token scan_hash_numeric(const SourceLocation &start);
        Token scan_string_literal(const SourceLocation &start);
        Token scan_char_literal(const SourceLocation &start);
        Token scan_operator_or_punctuation(const SourceLocation &start);

        /// Advance past a single escape sequence (after the leading backslash).
        void skip_escape();

        // ── Numeric literal helpers ───────────────────────────────────────
        /// Attempt to consume an exponent group [eE][+-]?\d+.
        /// Uses save/restore: if the exponent is incomplete, restores position
        /// and returns without consuming anything.
        void try_scan_exponent();

        /// Attempt to consume a type suffix (d/D, f/F, u/U[width], i/I<width>).
        /// Returns without consuming if no valid suffix is found at current position.
        void try_scan_type_suffix();

        /// Attempt to match and consume a specific integer width suffix starting at
        /// offset 1 from the current position. The `digits` list describes the expected
        /// digit characters of the width (e.g., {'3','2'} for "32"). Consumes the prefix
        /// byte plus all width digits if they match and are not followed by another digit.
        /// Returns true on success, false if the pattern does not match.
        /// Extracted from the `try_width` lambda in try_scan_type_suffix.
        [[nodiscard]] bool try_scan_width(std::initializer_list<char> digits);

        /// If `c1 == expected`, consumes it, builds a two-character token of `kind`,
        /// and returns it. Otherwise returns std::nullopt without consuming.
        /// Extracted from the `two` lambda in scan_operator_or_punctuation.
        [[nodiscard]] std::optional<Token> try_two_char_token(char c1, char expected, TokenKind kind, std::size_t text_start,
                                                              const SourceLocation &start);

        // ── Digit classification (formerly stateless lambdas) ─────────────
        /// Returns true iff `c` is a valid binary digit (0 or 1).
        [[nodiscard]] static constexpr bool is_binary_digit(char c) noexcept;

        /// Returns true iff `c` is a valid octal digit (0–7).
        [[nodiscard]] static constexpr bool is_octal_digit(char c) noexcept;

        /// Returns true iff `c` is a valid hexadecimal digit (0–9, a–f, A–F).
        [[nodiscard]] static bool is_hex_digit(char c) noexcept;

        // ── Keyword / type classification ─────────────────────────────────
        /// Map a lexed word to its `TokenKind` (keyword, type, or identifier).
        [[nodiscard]] static TokenKind classify_word(std::string_view text) noexcept;
    };

}  // namespace jsv
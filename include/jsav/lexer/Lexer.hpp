/*
 * Created by gbian on 28/02/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

#include "../headers.hpp"
#include "Token.hpp"

namespace jsv {

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
    /// `123.` produces `Numeric("123")` + `Dot(".")` — the dot is NOT part of
    /// the float literal.  This allows method-call syntax like `123.toString()`
    /// without ambiguity.  Leading-dot floats (`.456`) are also split.
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
        /// Return the byte length of the UTF-8 sequence starting with `first_byte`.
        [[nodiscard]] static std::size_t utf8_byte_len(unsigned char first_byte) noexcept;

        /// Decode the codepoint at `m_pos` without consuming.
        [[nodiscard]] std::uint32_t peek_codepoint() const noexcept;

        /// Decode and consume one UTF-8 codepoint, updating line/column correctly.
        std::uint32_t advance_codepoint() noexcept;

        // ── Location / token construction ─────────────────────────────────
        [[nodiscard]] SourceLocation current_location() const noexcept;
        [[nodiscard]] SourceSpan make_span(const SourceLocation &start) const;
        [[nodiscard]] Token make_token(TokenKind kind, std::string_view text, const SourceLocation &start) const;
        [[nodiscard]] Token error_token(std::string_view text, const SourceLocation &start) const;

        // ── Whitespace / comments ─────────────────────────────────────────
        void skip_whitespace_and_comments();

        // ── Scanners ──────────────────────────────────────────────────────
        Token scan_identifier_or_keyword(const SourceLocation &start, bool seen_unicode);
        Token scan_numeric_literal(const SourceLocation &start);
        Token scan_hash_numeric(const SourceLocation &start);
        Token scan_string_literal(const SourceLocation &start);
        Token scan_char_literal(const SourceLocation &start);
        Token scan_operator_or_punctuation(const SourceLocation &start);

        /// Advance past a single escape sequence (after the leading backslash).
        void skip_escape();

        // ── Unicode XID classification ────────────────────────────────────
        /// True if `cp` may start an identifier (Unicode XID_Start ∪ {'_'}).
        [[nodiscard]] static bool is_xid_start(std::uint32_t cp) noexcept;

        /// True if `cp` may continue an identifier (Unicode XID_Continue).
        [[nodiscard]] static bool is_xid_continue(std::uint32_t cp) noexcept;

        // ── Keyword / type classification ─────────────────────────────────
        /// Map a lexed word to its `TokenKind` (keyword, type, or identifier).
        [[nodiscard]] static TokenKind classify_word(std::string_view text) noexcept;
    };

}  // namespace jsv
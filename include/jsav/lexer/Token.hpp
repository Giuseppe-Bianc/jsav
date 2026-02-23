/*
 * Created by gbian on 23/02/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

#include "../headers.hpp"
#include "SourceSpan.hpp"

namespace jsv {

    enum class TokenKind : std::uint8_t {
        // ── Operatori a due caratteri (longest-match first) ──
        PlusEqual,     // +=
        MinusEqual,    // -=
        EqualEqual,    // ==
        NotEqual,      // !=
        LessEqual,     // <=
        GreaterEqual,  // >=
        PlusPlus,      // ++
        MinusMinus,    // --
        OrOr,          // ||
        AndAnd,        // &&
        ShiftLeft,     // <<
        ShiftRight,    // >>
        PercentEqual,  // %=
        XorEqual,      // ^=

        // ── Operatori a singolo carattere ──
        Plus,     // +
        Minus,    // -
        Star,     // *
        Slash,    // /
        Less,     // <
        Greater,  // >
        Not,      // !
        Xor,      // ^
        Percent,  // %
        Or,       // |
        And,      // &
        Equal,    // =
        Colon,    // :
        Comma,    // ,
        Dot,      // .

        // ── Keyword ──
        KeywordFun,
        KeywordIf,
        KeywordElse,
        KeywordReturn,
        KeywordWhile,
        KeywordFor,
        KeywordMain,
        KeywordVar,
        KeywordConst,
        KeywordNullptr,
        KeywordBreak,
        KeywordContinue,
        KeywordBool,  // porta il valore bool nel TokenValue

        // ── Identificatori ──
        IdentifierAscii,    // [a-zA-Z_][a-zA-Z0-9_]*
        IdentifierUnicode,  // Unicode XID (fallback)

        // ── Letterali numerici ──
        Numeric,      // decimale/float/scientifico + suffisso
        Binary,       // #b[01]+[uU]?
        Octal,        // #o[0-7]+[uU]?
        Hexadecimal,  // #x[0-9a-fA-F]+[uU]?

        // ── Letterali stringa / carattere ──
        StringLiteral,  // "..."  (senza virgolette)
        CharLiteral,    // '.'   (senza virgolette)

        // ── Parentesi ──
        OpenParen,
        CloseParen,  // ( )
        OpenBracket,
        CloseBracket,  // [ ]
        OpenBrace,
        CloseBrace,  // { }

        // ── Tipi primitivi ──
        TypeI8,
        TypeI16,
        TypeI32,
        TypeI64,
        TypeU8,
        TypeU16,
        TypeU32,
        TypeU64,
        TypeF32,
        TypeF64,
        TypeChar,
        TypeString,
        TypeBool,

        // ── Varie ──
        Semicolon,
        Eof,
        Error  // token non riconosciuto
    };

    /// Convert a TokenKind to its string representation.
    /// Used by operator<<, std::formatter and fmt::formatter.
    [[nodiscard]] constexpr std::string_view to_string(TokenKind kind) noexcept {
        switch(kind) {
            // Operatori a due caratteri
        case TokenKind::PlusEqual: return "+=";
        case TokenKind::MinusEqual: return "-=";
        case TokenKind::EqualEqual: return "==";
        case TokenKind::NotEqual: return "!=";
        case TokenKind::LessEqual: return "<=";
        case TokenKind::GreaterEqual: return ">=";
        case TokenKind::PlusPlus: return "++";
        case TokenKind::MinusMinus: return "--";
        case TokenKind::OrOr: return "||";
        case TokenKind::AndAnd: return "&&";
        case TokenKind::ShiftLeft: return "<<";
        case TokenKind::ShiftRight: return ">>";
        case TokenKind::PercentEqual: return "%=";
        case TokenKind::XorEqual: return "^=";

            // Operatori a singolo carattere
        case TokenKind::Plus: return "+";
        case TokenKind::Minus: return "-";
        case TokenKind::Star: return "*";
        case TokenKind::Slash: return "/";
        case TokenKind::Less: return "<";
        case TokenKind::Greater: return ">";
        case TokenKind::Not: return "!";
        case TokenKind::Xor: return "^";
        case TokenKind::Percent: return "%";
        case TokenKind::Or: return "|";
        case TokenKind::And: return "&";
        case TokenKind::Equal: return "=";
        case TokenKind::Colon: return ":";
        case TokenKind::Comma: return ",";
        case TokenKind::Dot: return ".";

            // Keyword
        case TokenKind::KeywordFun: return "fun";
        case TokenKind::KeywordIf: return "if";
        case TokenKind::KeywordElse: return "else";
        case TokenKind::KeywordReturn: return "return";
        case TokenKind::KeywordWhile: return "while";
        case TokenKind::KeywordFor: return "for";
        case TokenKind::KeywordMain: return "main";
        case TokenKind::KeywordVar: return "var";
        case TokenKind::KeywordConst: return "const";
        case TokenKind::KeywordNullptr: return "nullptr";
        case TokenKind::KeywordBreak: return "break";
        case TokenKind::KeywordContinue: return "continue";
        case TokenKind::KeywordBool: return "bool";

            // Identificatori
        case TokenKind::IdentifierAscii: return "identifier";
        case TokenKind::IdentifierUnicode: return "identifier";

            // Letterali numerici
        case TokenKind::Numeric: return "numeric";
        case TokenKind::Binary: return "binary";
        case TokenKind::Octal: return "octal";
        case TokenKind::Hexadecimal: return "hex";

            // Letterali stringa / carattere
        case TokenKind::StringLiteral: return "string";
        case TokenKind::CharLiteral: return "char";

            // Parentesi
        case TokenKind::OpenParen: return "(";
        case TokenKind::CloseParen: return ")";
        case TokenKind::OpenBracket: return "[";
        case TokenKind::CloseBracket: return "]";
        case TokenKind::OpenBrace: return "{";
        case TokenKind::CloseBrace: return "}";

            // Tipi primitivi
        case TokenKind::TypeI8: return "i8";
        case TokenKind::TypeI16: return "i16";
        case TokenKind::TypeI32: return "i32";
        case TokenKind::TypeI64: return "i64";
        case TokenKind::TypeU8: return "u8";
        case TokenKind::TypeU16: return "u16";
        case TokenKind::TypeU32: return "u32";
        case TokenKind::TypeU64: return "u64";
        case TokenKind::TypeF32: return "f32";
        case TokenKind::TypeF64: return "f64";
        case TokenKind::TypeChar: return "char";
        case TokenKind::TypeString: return "string";
        case TokenKind::TypeBool: return "bool";

            // Varie
        case TokenKind::Semicolon: return ";";
        case TokenKind::Eof: return "eof";
        case TokenKind::Error: return "error";

        default: return "unknown";
        }
    }

    class Token {
    public:
        // Costruttore primario
        Token(const TokenKind kind, std::string_view text, const SourceSpan &span) : m_kind(kind), m_text(text), m_span(span) {}

        Token(const Token &other) noexcept = default;
        Token &operator=(const Token &other) noexcept = default;
        Token(Token &&other) noexcept = default;
        Token &operator=(Token &&other) noexcept = default;
        [[nodiscard]] auto operator<=>(const Token &other) const noexcept = default;

        [[nodiscard]] TokenKind getKind() const { return m_kind; }
        [[nodiscard]] std::string_view getText() const { return m_text; }
        [[nodiscard]] const SourceSpan &getSpan() const { return m_span; }

        /// Format: `TokenKind(text)`
        /// Shared by operator<<, std::formatter and fmt::formatter.
        [[nodiscard]] std::string to_string() const;

        friend std::ostream &operator<<(std::ostream &os, const Token &token);

    private:
        TokenKind m_kind;
        std::string_view m_text;  // testo originale del token (senza modifiche)
        SourceSpan m_span;        // posizione del token nel codice sorgente
    };

}  // namespace jsv

// -------------------------------------------------------------------------
// std::formatter  (C++23 <format>)
// -------------------------------------------------------------------------
namespace std {
    template <> struct formatter<jsv::Token> : formatter<string> {
        template <typename FormatContext>
        auto format(const jsv::Token &token, FormatContext &ctx) const {
            return formatter<string>::format(token.to_string(), ctx);
        }
    };
}  // namespace std

// -------------------------------------------------------------------------
// fmt::formatter  (fmtlib)
// -------------------------------------------------------------------------
template <> struct fmt::formatter<jsv::Token> : fmt::formatter<std::string> {
    template <typename FormatContext>
    auto format(const jsv::Token &token, FormatContext &ctx) const {
        return fmt::formatter<std::string>::format(token.to_string(), ctx);
    }
};

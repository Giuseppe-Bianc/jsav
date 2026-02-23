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
    [[nodiscard]] constexpr std::string_view tokenKindToString(TokenKind kind) noexcept {
        switch(kind) {
            // Operatori a due caratteri
        case TokenKind::PlusEqual:
            return "PLUS_EQUAL";
        case TokenKind::MinusEqual:
            return "MINUS_EQUAL";
        case TokenKind::EqualEqual:
            return "EQUAL_EQUAL";
        case TokenKind::NotEqual:
            return "NOT_EQUAL";
        case TokenKind::LessEqual:
            return "LESS_EQUAL";
        case TokenKind::GreaterEqual:
            return "GREATER_EQUAL";
        case TokenKind::PlusPlus:
            return "PLUS_PLUS";
        case TokenKind::MinusMinus:
            return "MINUS_MINUS";
        case TokenKind::OrOr:
            return "OR_OR";
        case TokenKind::AndAnd:
            return "AND_AND";
        case TokenKind::ShiftLeft:
            return "SHIFT_LEFT";
        case TokenKind::ShiftRight:
            return "SHIFT_RIGHT";
        case TokenKind::PercentEqual:
            return "PERCENT_EQUAL";
        case TokenKind::XorEqual:
            return "XOR_EQUAL";

            // Operatori a singolo carattere
        case TokenKind::Plus:
            return "PLUS";
        case TokenKind::Minus:
            return "MINUS";
        case TokenKind::Star:
            return "STAR";
        case TokenKind::Slash:
            return "SLASH";
        case TokenKind::Less:
            return "LESS";
        case TokenKind::Greater:
            return "GREATER";
        case TokenKind::Not:
            return "NOT";
        case TokenKind::Xor:
            return "XOR";
        case TokenKind::Percent:
            return "PERCENT";
        case TokenKind::Or:
            return "OR";
        case TokenKind::And:
            return "AND";
        case TokenKind::Equal:
            return "EQUAL";
        case TokenKind::Colon:
            return "COLON";
        case TokenKind::Comma:
            return "COMMA";
        case TokenKind::Dot:
            return "DOT";

            // Keyword
        case TokenKind::KeywordFun:
            return "FUN";
        case TokenKind::KeywordIf:
            return "IF";
        case TokenKind::KeywordElse:
            return "ELSE";
        case TokenKind::KeywordReturn:
            return "RETURN";
        case TokenKind::KeywordWhile:
            return "WHILE";
        case TokenKind::KeywordFor:
            return "FOR";
        case TokenKind::KeywordMain:
            return "MAIN";
        case TokenKind::KeywordVar:
            return "VAR";
        case TokenKind::KeywordConst:
            return "CONST";
        case TokenKind::KeywordNullptr:
            return "NULLPTR";
        case TokenKind::KeywordBreak:
            return "BREAK";
        case TokenKind::KeywordContinue:
            return "CONTINUE";
        case TokenKind::KeywordBool:
            return "BOOL";

            // Identificatori
        case TokenKind::IdentifierAscii:
            return "IDENTIFIER";
        case TokenKind::IdentifierUnicode:
            return "IDENTIFIER";

            // Letterali numerici
        case TokenKind::Numeric:
            return "NUMERIC";
        case TokenKind::Binary:
            return "BINARY";
        case TokenKind::Octal:
            return "OCTAL";
        case TokenKind::Hexadecimal:
            return "HEX";

            // Letterali stringa / carattere
        case TokenKind::StringLiteral:
            return "STRING";
        case TokenKind::CharLiteral:
            return "CHAR";

            // Parentesi
        case TokenKind::OpenParen:
            return "OPEN_PAREN";
        case TokenKind::CloseParen:
            return "CLOSE_PAREN";
        case TokenKind::OpenBracket:
            return "OPEN_BRACKET";
        case TokenKind::CloseBracket:
            return "CLOSE_BRACKET";
        case TokenKind::OpenBrace:
            return "OPEN_BRACE";
        case TokenKind::CloseBrace:
            return "CLOSE_BRACE";

            // Tipi primitivi
        case TokenKind::TypeI8:
            return "I8";
        case TokenKind::TypeI16:
            return "I16";
        case TokenKind::TypeI32:
            return "I32";
        case TokenKind::TypeI64:
            return "I64";
        case TokenKind::TypeU8:
            return "U8";
        case TokenKind::TypeU16:
            return "U16";
        case TokenKind::TypeU32:
            return "U32";
        case TokenKind::TypeU64:
            return "U64";
        case TokenKind::TypeF32:
            return "F32";
        case TokenKind::TypeF64:
            return "F64";
        case TokenKind::TypeChar:
            return "CHAR";
        case TokenKind::TypeString:
            return "STRING";
        case TokenKind::TypeBool:
            return "BOOL";

            // Varie
        case TokenKind::Semicolon:
            return "SEMICOLON";
        case TokenKind::Eof:
            return "EOF";
        case TokenKind::Error:
            return "ERROR";

        default:
            return "UNKNOWN";
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
        template <typename FormatContext> auto format(const jsv::Token &token, FormatContext &ctx) const {
            return formatter<string>::format(token.to_string(), ctx);
        }
    };
}  // namespace std

// -------------------------------------------------------------------------
// fmt::formatter  (fmtlib)
// -------------------------------------------------------------------------
template <> struct fmt::formatter<jsv::Token> : fmt::formatter<std::string> {
    template <typename FormatContext> auto format(const jsv::Token &token, FormatContext &ctx) const {
        return fmt::formatter<std::string>::format(token.to_string(), ctx);
    }
};

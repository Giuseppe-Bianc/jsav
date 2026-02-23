/*
 * Created by gbian on 23/02/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

#include "../headers.hpp"

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
    class Token {};

}  // namespace jsv

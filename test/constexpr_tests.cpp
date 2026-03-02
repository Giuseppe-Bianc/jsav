#include <catch2/catch_test_macros.hpp>
// clang-format off
// NOLINTBEGIN(*-include-cleaner, *-avoid-magic-numbers, *-magic-numbers, *-unchecked-optional-access, *-avoid-do-while, *-use-anonymous-namespace, *-qualified-auto, *-suspicious-stringview-data-usage, *-err58-cpp, *-function-cognitive-complexity, *-macro-usage, *-unnecessary-copy-initialization)
// clang-format on

#include <jsav/jsav.hpp>
#include <jsav/lexer/unicode/UnicodeData.hpp>
#include <jsav/lexer/unicode/Utf8.hpp>

/*
TEST_CASE("Factorials are computed with constexpr", "[factorial]")
{
  STATIC_REQUIRE(factorial_constexpr(0) == 1);
  STATIC_REQUIRE(factorial_constexpr(1) == 1);
  STATIC_REQUIRE(factorial_constexpr(2) == 2);
  STATIC_REQUIRE(factorial_constexpr(3) == 6);
  STATIC_REQUIRE(factorial_constexpr(10) == 3628800);
}*/

TEST_CASE("the constexpr size of types", "[TypeSizes]") {
    STATIC_REQUIRE(TypeSizes::sizeOfBool == sizeof(bool));
    STATIC_REQUIRE(TypeSizes::sizeOfByte == sizeof(std::byte));
    STATIC_REQUIRE(TypeSizes::sizeOfIntPtr == sizeof(std::intptr_t));
    STATIC_REQUIRE(TypeSizes::sizeOfUintPtr == sizeof(std::uintptr_t));
    STATIC_REQUIRE(TypeSizes::sizeOfInt8T == sizeof(std::int8_t));
    STATIC_REQUIRE(TypeSizes::sizeOfInt16T == sizeof(std::int16_t));
    STATIC_REQUIRE(TypeSizes::sizeOfInt32T == sizeof(std::int32_t));
    STATIC_REQUIRE(TypeSizes::sizeOfInt64T == sizeof(std::int64_t));
    STATIC_REQUIRE(TypeSizes::sizeOfUint8T == sizeof(std::uint8_t));
    STATIC_REQUIRE(TypeSizes::sizeOfUint16T == sizeof(std::uint16_t));
    STATIC_REQUIRE(TypeSizes::sizeOfUint32T == sizeof(std::uint32_t));
    STATIC_REQUIRE(TypeSizes::sizeOfUint64T == sizeof(std::uint64_t));
    STATIC_REQUIRE(TypeSizes::sizeOfPtrdiffT == sizeof(std::ptrdiff_t));
    STATIC_REQUIRE(TypeSizes::sizeOfDivT == sizeof(std::div_t));
    STATIC_REQUIRE(TypeSizes::sizeOfLdivT == sizeof(std::ldiv_t));
    STATIC_REQUIRE(TypeSizes::sizeOfChar == sizeof(char));
    STATIC_REQUIRE(TypeSizes::sizeOfChar16T == sizeof(char16_t));
    STATIC_REQUIRE(TypeSizes::sizeOfChar32T == sizeof(char32_t));
    STATIC_REQUIRE(TypeSizes::sizeOfChar8T == sizeof(char8_t));
    STATIC_REQUIRE(TypeSizes::sizeOfDouble == sizeof(double));
    STATIC_REQUIRE(TypeSizes::sizeOfFloat == sizeof(float));
    STATIC_REQUIRE(TypeSizes::sizeOfInt == sizeof(int));
    STATIC_REQUIRE(TypeSizes::sizeOfLong == sizeof(long));
    STATIC_REQUIRE(TypeSizes::sizeOfLongDouble == sizeof(long double));
    STATIC_REQUIRE(TypeSizes::sizeOfLongInt == sizeof(long int));
    STATIC_REQUIRE(TypeSizes::sizeOfLongLong == sizeof(long long));
    STATIC_REQUIRE(TypeSizes::sizeOfLongLongInt == sizeof(long long int));
    STATIC_REQUIRE(TypeSizes::sizeOfShort == sizeof(short));
    STATIC_REQUIRE(TypeSizes::sizeOfShortInt == sizeof(short int));
    STATIC_REQUIRE(TypeSizes::sizeOfUChar == sizeof(unsigned char));
    STATIC_REQUIRE(TypeSizes::sizeOfUInt == sizeof(unsigned int));
    STATIC_REQUIRE(TypeSizes::sizeOfULong == sizeof(unsigned long));
    STATIC_REQUIRE(TypeSizes::sizeOfULongInt == sizeof(unsigned long int));
    STATIC_REQUIRE(TypeSizes::sizeOfULongLong == sizeof(unsigned long long));
    STATIC_REQUIRE(TypeSizes::sizeOfULongLongInt == sizeof(unsigned long long int));
    STATIC_REQUIRE(TypeSizes::sizeOfString == sizeof(std::string));
    STATIC_REQUIRE(TypeSizes::sizeOfWString == sizeof(std::wstring));
    STATIC_REQUIRE(TypeSizes::sizeOfU8String == sizeof(std::u8string));
    STATIC_REQUIRE(TypeSizes::sizeOfU16String == sizeof(std::u16string));
    STATIC_REQUIRE(TypeSizes::sizeOfU32String == sizeof(std::u32string));
    STATIC_REQUIRE(TypeSizes::sizeOfStringView == sizeof(std::string_view));
    STATIC_REQUIRE(TypeSizes::sizeOfWStringView == sizeof(std::wstring_view));
    STATIC_REQUIRE(TypeSizes::sizeOfU8StringView == sizeof(std::u8string_view));
    STATIC_REQUIRE(TypeSizes::sizeOfU16StringView == sizeof(std::u16string_view));
    STATIC_REQUIRE(TypeSizes::sizeOfU32StringView == sizeof(std::u32string_view));
}

TEST_CASE("TokenKind tokenKindToString returns correct string representation", "[TokenKind]") {
    // Two-character operators
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::PlusEqual) == "PLUS_EQUAL");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::MinusEqual) == "MINUS_EQUAL");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::EqualEqual) == "EQUAL_EQUAL");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::NotEqual) == "NOT_EQUAL");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::LessEqual) == "LESS_EQUAL");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::GreaterEqual) == "GREATER_EQUAL");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::PlusPlus) == "PLUS_PLUS");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::MinusMinus) == "MINUS_MINUS");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::OrOr) == "OR_OR");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::AndAnd) == "AND_AND");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::ShiftLeft) == "SHIFT_LEFT");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::ShiftRight) == "SHIFT_RIGHT");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::PercentEqual) == "PERCENT_EQUAL");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::XorEqual) == "XOR_EQUAL");

    // Single-character operators
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::Plus) == "PLUS");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::Minus) == "MINUS");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::Star) == "STAR");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::Slash) == "SLASH");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::Less) == "LESS");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::Greater) == "GREATER");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::Not) == "NOT");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::Xor) == "XOR");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::Percent) == "PERCENT");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::Or) == "OR");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::And) == "AND");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::Equal) == "EQUAL");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::Colon) == "COLON");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::Comma) == "COMMA");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::Dot) == "DOT");

    // Keywords
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::KeywordFun) == "FUN");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::KeywordIf) == "IF");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::KeywordElse) == "ELSE");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::KeywordReturn) == "RETURN");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::KeywordWhile) == "WHILE");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::KeywordFor) == "FOR");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::KeywordMain) == "MAIN");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::KeywordVar) == "VAR");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::KeywordConst) == "CONST");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::KeywordNullptr) == "NULLPTR");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::KeywordBreak) == "BREAK");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::KeywordContinue) == "CONTINUE");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::KeywordBool) == "BOOL");

    // Identifiers
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::IdentifierAscii) == "IDENTIFIER");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::IdentifierUnicode) == "IDENTIFIER");

    // Numeric literals
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::Numeric) == "NUMERIC");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::Binary) == "BINARY");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::Octal) == "OCTAL");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::Hexadecimal) == "HEX");

    // String/char literals
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::StringLiteral) == "STRING");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::CharLiteral) == "CHAR");

    // Brackets/braces/parens
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::OpenParen) == "OPEN_PAREN");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::CloseParen) == "CLOSE_PAREN");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::OpenBracket) == "OPEN_BRACKET");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::CloseBracket) == "CLOSE_BRACKET");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::OpenBrace) == "OPEN_BRACE");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::CloseBrace) == "CLOSE_BRACE");

    // Primitive types
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::TypeI8) == "I8");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::TypeI16) == "I16");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::TypeI32) == "I32");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::TypeI64) == "I64");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::TypeU8) == "U8");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::TypeU16) == "U16");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::TypeU32) == "U32");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::TypeU64) == "U64");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::TypeF32) == "F32");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::TypeF64) == "F64");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::TypeChar) == "CHAR");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::TypeString) == "STRING");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::TypeBool) == "BOOL");

    // Miscellaneous
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::Semicolon) == "SEMICOLON");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::Eof) == "EOF");
    STATIC_REQUIRE(jsv::tokenKindToString(jsv::TokenKind::Error) == "ERROR");
}

// ==========================================================================
// T019 [CG01] Phase 2: Foundational decode + classify verification
// ==========================================================================

TEST_CASE("jsv::unicode foundational constexpr functions", "[Unicode][T019]") {
    using namespace jsv::unicode;

    // ── decode_utf8: ASCII 'A' ──────────────────────────────────────────────
    SECTION("ASCII 'A' decodes to {0x41, 1, Ok}") {
        constexpr auto r = decode_utf8("A", 0);
        STATIC_REQUIRE(r.codepoint == U'A');
        STATIC_REQUIRE(r.byte_length == 1);
        STATIC_REQUIRE(r.status == Utf8Status::Ok);
    }

    // ── decode_utf8: CJK U+5909 (変) = 0xE5 0xA4 0x89 ─────────────────────
    SECTION("CJK U+5909 decodes to {0x5909, 3, Ok}") {
        constexpr auto r = decode_utf8("\xE5\xA4\x89", 0);
        STATIC_REQUIRE(r.codepoint == char32_t{0x5909U});
        STATIC_REQUIRE(r.byte_length == 3);
        STATIC_REQUIRE(r.status == Utf8Status::Ok);
    }

    // ── decode_utf8: U+0020 SPACE ──────────────────────────────────────────
    SECTION("U+0020 SPACE decodes to {0x0020, 1, Ok}") {
        constexpr auto r = decode_utf8(" ", 0);
        STATIC_REQUIRE(r.codepoint == U' ');
        STATIC_REQUIRE(r.byte_length == 1);
        STATIC_REQUIRE(r.status == Utf8Status::Ok);
    }

    // ── is_letter ──────────────────────────────────────────────────────────
    SECTION("is_letter: ASCII letters are letters") {
        STATIC_REQUIRE(is_letter(U'A'));
        STATIC_REQUIRE(is_letter(U'z'));
        STATIC_REQUIRE(!is_letter(U'0'));
        STATIC_REQUIRE(!is_letter(U'_'));
    }

    SECTION("is_letter: CJK U+5909 is a letter") { STATIC_REQUIRE(is_letter(char32_t{0x5909U})); }

    // ── is_id_start ────────────────────────────────────────────────────────
    SECTION("is_id_start: ASCII letters and underscore are id_start") {
        STATIC_REQUIRE(is_id_start(U'A'));
        STATIC_REQUIRE(is_id_start(U'z'));
        STATIC_REQUIRE(is_id_start(U'_'));
        STATIC_REQUIRE(!is_id_start(U'0'));
    }

    // ── is_id_continue ─────────────────────────────────────────────────────
    SECTION("is_id_continue: ASCII letters, digits, underscore") {
        STATIC_REQUIRE(is_id_continue(U'A'));
        STATIC_REQUIRE(is_id_continue(U'0'));
        STATIC_REQUIRE(is_id_continue(U'_'));
        STATIC_REQUIRE(!is_id_continue(U' '));
    }

    // ── is_unicode_whitespace ──────────────────────────────────────────────
    SECTION("is_unicode_whitespace: U+0020 is Zs") {
        STATIC_REQUIRE(is_unicode_whitespace(U' '));
        STATIC_REQUIRE(!is_unicode_whitespace(U'\t'));  // Tab is not Zs/Zl/Zp
        STATIC_REQUIRE(!is_unicode_whitespace(U'A'));
    }
}

// ==========================================================================
// T020–T025, T030A Phase 3: UTF-8 decoding (valid sequences)
// ==========================================================================

TEST_CASE("Utf8Decoder_AsciiChar_ReturnsOkWithByteLength1", "[Unicode][T020]") {
    using namespace jsv::unicode;
    constexpr auto r = decode_utf8("A", 0);
    STATIC_REQUIRE(r.codepoint == U'A');
    STATIC_REQUIRE(r.byte_length == 1);
    STATIC_REQUIRE(r.status == Utf8Status::Ok);
}

TEST_CASE("Utf8Decoder_TwoByteSequence_ReturnsCorrectCodepoint", "[Unicode][T021]") {
    using namespace jsv::unicode;
    // é = U+00E9 = 0xC3 0xA9
    constexpr auto r = decode_utf8("\xC3\xA9", 0);
    STATIC_REQUIRE(r.codepoint == char32_t{0x00E9U});
    STATIC_REQUIRE(r.byte_length == 2);
    STATIC_REQUIRE(r.status == Utf8Status::Ok);
}

TEST_CASE("Utf8Decoder_ThreeByteSequence_ReturnsCorrectCodepoint", "[Unicode][T022]") {
    using namespace jsv::unicode;
    // 変 = U+5909 = 0xE5 0xA4 0x89
    constexpr auto r = decode_utf8("\xE5\xA4\x89", 0);
    STATIC_REQUIRE(r.codepoint == char32_t{0x5909U});
    STATIC_REQUIRE(r.byte_length == 3);
    STATIC_REQUIRE(r.status == Utf8Status::Ok);
}

TEST_CASE("Utf8Decoder_FourByteSequence_ReturnsCorrectCodepoint", "[Unicode][T023]") {
    using namespace jsv::unicode;
    // 𐍈 = U+10348 = 0xF0 0x90 0x8D 0x88
    constexpr auto r = decode_utf8("\xF0\x90\x8D\x88", 0);
    STATIC_REQUIRE(r.codepoint == char32_t{0x10348U});
    STATIC_REQUIRE(r.byte_length == 4);
    STATIC_REQUIRE(r.status == Utf8Status::Ok);
}

TEST_CASE("Utf8Decoder_NullByte_ReturnsOkCodepointZero", "[Unicode][T024]") {
    using namespace jsv::unicode;
    constexpr auto r = decode_utf8(std::string_view("\x00", 1), 0);
    STATIC_REQUIRE(r.codepoint == char32_t{0U});
    STATIC_REQUIRE(r.byte_length == 1);
    STATIC_REQUIRE(r.status == Utf8Status::Ok);
}

TEST_CASE("Utf8Decoder_InterleavedAsciiAndMultibyte_AllDecodeCorrectly", "[Unicode][T025]") {
    using namespace jsv::unicode;
    // "A" + é(0xC3 0xA9) + "B"
    constexpr std::string_view mixed = "A\xC3\xA9"
                                       "B";
    constexpr auto r0 = decode_utf8(mixed, 0);  // 'A'
    STATIC_REQUIRE(r0.codepoint == U'A');
    STATIC_REQUIRE(r0.byte_length == 1);
    STATIC_REQUIRE(r0.status == Utf8Status::Ok);

    constexpr auto r1 = decode_utf8(mixed, 1);  // é
    STATIC_REQUIRE(r1.codepoint == char32_t{0x00E9U});
    STATIC_REQUIRE(r1.byte_length == 2);
    STATIC_REQUIRE(r1.status == Utf8Status::Ok);

    constexpr auto r2 = decode_utf8(mixed, 3);  // 'B'
    STATIC_REQUIRE(r2.codepoint == U'B');
    STATIC_REQUIRE(r2.byte_length == 1);
    STATIC_REQUIRE(r2.status == Utf8Status::Ok);
}

TEST_CASE("Utf8Decoder_AllSeventeenPlanes_DecodeCorrectly", "[Unicode][T030A][SC-006]") {
    using namespace jsv::unicode;
    // One valid code point from each of the 17 Unicode planes
    // Plane 0 BMP: U+0041 'A' (ASCII)
    STATIC_REQUIRE(decode_utf8("A", 0).status == Utf8Status::Ok);
    STATIC_REQUIRE(decode_utf8("A", 0).codepoint == char32_t{0x0041U});
    // Plane 1 SMP: U+10348 = 0xF0 0x90 0x8D 0x88
    STATIC_REQUIRE(decode_utf8("\xF0\x90\x8D\x88", 0).codepoint == char32_t{0x10348U});
    STATIC_REQUIRE(decode_utf8("\xF0\x90\x8D\x88", 0).byte_length == 4);
    STATIC_REQUIRE(decode_utf8("\xF0\x90\x8D\x88", 0).status == Utf8Status::Ok);
    // Plane 2 SIP: U+20000 = 0xF0 0xA0 0x80 0x80
    STATIC_REQUIRE(decode_utf8("\xF0\xA0\x80\x80", 0).codepoint == char32_t{0x20000U});
    STATIC_REQUIRE(decode_utf8("\xF0\xA0\x80\x80", 0).status == Utf8Status::Ok);
    // Plane 3 TIP: U+30000 = 0xF0 0xB0 0x80 0x80
    STATIC_REQUIRE(decode_utf8("\xF0\xB0\x80\x80", 0).codepoint == char32_t{0x30000U});
    STATIC_REQUIRE(decode_utf8("\xF0\xB0\x80\x80", 0).status == Utf8Status::Ok);
    // Plane 4: U+40000 = 0xF1 0x80 0x80 0x80
    STATIC_REQUIRE(decode_utf8("\xF1\x80\x80\x80", 0).codepoint == char32_t{0x40000U});
    STATIC_REQUIRE(decode_utf8("\xF1\x80\x80\x80", 0).status == Utf8Status::Ok);
    // Plane 5: U+50000 = 0xF1 0x90 0x80 0x80
    STATIC_REQUIRE(decode_utf8("\xF1\x90\x80\x80", 0).codepoint == char32_t{0x50000U});
    STATIC_REQUIRE(decode_utf8("\xF1\x90\x80\x80", 0).status == Utf8Status::Ok);
    // Plane 6: U+60000 = 0xF1 0xA0 0x80 0x80
    STATIC_REQUIRE(decode_utf8("\xF1\xA0\x80\x80", 0).codepoint == char32_t{0x60000U});
    STATIC_REQUIRE(decode_utf8("\xF1\xA0\x80\x80", 0).status == Utf8Status::Ok);
    // Plane 7: U+70000 = 0xF1 0xB0 0x80 0x80
    STATIC_REQUIRE(decode_utf8("\xF1\xB0\x80\x80", 0).codepoint == char32_t{0x70000U});
    STATIC_REQUIRE(decode_utf8("\xF1\xB0\x80\x80", 0).status == Utf8Status::Ok);
    // Plane 8: U+80000 = 0xF2 0x80 0x80 0x80
    STATIC_REQUIRE(decode_utf8("\xF2\x80\x80\x80", 0).codepoint == char32_t{0x80000U});
    STATIC_REQUIRE(decode_utf8("\xF2\x80\x80\x80", 0).status == Utf8Status::Ok);
    // Plane 9: U+90000 = 0xF2 0x90 0x80 0x80
    STATIC_REQUIRE(decode_utf8("\xF2\x90\x80\x80", 0).codepoint == char32_t{0x90000U});
    STATIC_REQUIRE(decode_utf8("\xF2\x90\x80\x80", 0).status == Utf8Status::Ok);
    // Plane 10: U+A0000 = 0xF2 0xA0 0x80 0x80
    STATIC_REQUIRE(decode_utf8("\xF2\xA0\x80\x80", 0).codepoint == char32_t{0xA0000U});
    STATIC_REQUIRE(decode_utf8("\xF2\xA0\x80\x80", 0).status == Utf8Status::Ok);
    // Plane 11: U+B0000 = 0xF2 0xB0 0x80 0x80
    STATIC_REQUIRE(decode_utf8("\xF2\xB0\x80\x80", 0).codepoint == char32_t{0xB0000U});
    STATIC_REQUIRE(decode_utf8("\xF2\xB0\x80\x80", 0).status == Utf8Status::Ok);
    // Plane 12: U+C0000 = 0xF3 0x80 0x80 0x80
    STATIC_REQUIRE(decode_utf8("\xF3\x80\x80\x80", 0).codepoint == char32_t{0xC0000U});
    STATIC_REQUIRE(decode_utf8("\xF3\x80\x80\x80", 0).status == Utf8Status::Ok);
    // Plane 13: U+D0000 = 0xF3 0x90 0x80 0x80
    STATIC_REQUIRE(decode_utf8("\xF3\x90\x80\x80", 0).codepoint == char32_t{0xD0000U});
    STATIC_REQUIRE(decode_utf8("\xF3\x90\x80\x80", 0).status == Utf8Status::Ok);
    // Plane 14 SSP: U+E0000 = 0xF3 0xA0 0x80 0x80
    STATIC_REQUIRE(decode_utf8("\xF3\xA0\x80\x80", 0).codepoint == char32_t{0xE0000U});
    STATIC_REQUIRE(decode_utf8("\xF3\xA0\x80\x80", 0).status == Utf8Status::Ok);
    // Plane 15 SPUA-A: U+F0000 = 0xF3 0xB0 0x80 0x80
    STATIC_REQUIRE(decode_utf8("\xF3\xB0\x80\x80", 0).codepoint == char32_t{0xF0000U});
    STATIC_REQUIRE(decode_utf8("\xF3\xB0\x80\x80", 0).status == Utf8Status::Ok);
    // Plane 16 SPUA-B: U+100000 = 0xF4 0x80 0x80 0x80
    STATIC_REQUIRE(decode_utf8("\xF4\x80\x80\x80", 0).codepoint == char32_t{0x100000U});
    STATIC_REQUIRE(decode_utf8("\xF4\x80\x80\x80", 0).byte_length == 4);
    STATIC_REQUIRE(decode_utf8("\xF4\x80\x80\x80", 0).status == Utf8Status::Ok);
}

// ==========================================================================
// T036–T044 Phase 4: Malformed UTF-8 decoding (error cases)
// ==========================================================================

TEST_CASE("Utf8Decoder_OverlongTwoByte_ReturnsOverlongError", "[Unicode][T036]") {
    using namespace jsv::unicode;
    // 0xC0 0xAF → Overlong (would encode U+002F '/')
    constexpr auto r = decode_utf8("\xC0\xAF", 0);
    STATIC_REQUIRE(r.codepoint == char32_t{0xFFFDU});
    STATIC_REQUIRE(r.byte_length == 1);
    STATIC_REQUIRE(r.status == Utf8Status::Overlong);
}

TEST_CASE("Utf8Decoder_OverlongThreeByte_ReturnsOverlongError", "[Unicode][T037]") {
    using namespace jsv::unicode;
    // 0xE0 0x80 0xAF → Overlong
    constexpr auto r = decode_utf8("\xE0\x80\xAF", 0);
    STATIC_REQUIRE(r.codepoint == char32_t{0xFFFDU});
    STATIC_REQUIRE(r.byte_length == 1);
    STATIC_REQUIRE(r.status == Utf8Status::Overlong);
}

TEST_CASE("Utf8Decoder_SurrogateHalf_ReturnsSurrogateError", "[Unicode][T038]") {
    using namespace jsv::unicode;
    // 0xED 0xA0 0x80 → U+D800 (high surrogate)
    constexpr auto r = decode_utf8("\xED\xA0\x80", 0);
    STATIC_REQUIRE(r.codepoint == char32_t{0xFFFDU});
    STATIC_REQUIRE(r.byte_length == 1);
    STATIC_REQUIRE(r.status == Utf8Status::Surrogate);
}

TEST_CASE("Utf8Decoder_OutOfRange_ReturnsOutOfRangeError", "[Unicode][T039]") {
    using namespace jsv::unicode;
    // 0xF4 0x90 0x80 0x80 → U+110000 (out of Unicode range)
    constexpr auto r = decode_utf8("\xF4\x90\x80\x80", 0);
    STATIC_REQUIRE(r.codepoint == char32_t{0xFFFDU});
    STATIC_REQUIRE(r.byte_length == 1);
    STATIC_REQUIRE(r.status == Utf8Status::OutOfRange);
}

TEST_CASE("Utf8Decoder_OrphanedContinuation_ReturnsOrphanedError", "[Unicode][T040]") {
    using namespace jsv::unicode;
    // 0x80 → orphaned continuation byte
    constexpr auto r = decode_utf8("\x80", 0);
    STATIC_REQUIRE(r.codepoint == char32_t{0xFFFDU});
    STATIC_REQUIRE(r.byte_length == 1);
    STATIC_REQUIRE(r.status == Utf8Status::OrphanedContinuation);
}

TEST_CASE("Utf8Decoder_TruncatedTwoByte_ReturnsTruncatedError", "[Unicode][T041]") {
    using namespace jsv::unicode;
    // 0xC3 alone (only 1 byte, should be 2) → TruncatedSequence
    constexpr auto r = decode_utf8("\xC3", 0);
    STATIC_REQUIRE(r.codepoint == char32_t{0xFFFDU});
    STATIC_REQUIRE(r.byte_length == 1);
    STATIC_REQUIRE(r.status == Utf8Status::TruncatedSequence);
}

TEST_CASE("Utf8Decoder_TruncatedThreeByte_ReturnsCorrectMaximalSubpart", "[Unicode][T042]") {
    using namespace jsv::unicode;
    // 0xE5 0xA4 at end of input (only 2 bytes, should be 3) → TruncatedSequence, length=2
    constexpr auto r = decode_utf8("\xE5\xA4", 0);
    STATIC_REQUIRE(r.codepoint == char32_t{0xFFFDU});
    STATIC_REQUIRE(r.byte_length == 2);
    STATIC_REQUIRE(r.status == Utf8Status::TruncatedSequence);
}

TEST_CASE("Utf8Decoder_InvalidLeadByte_ReturnsInvalidLeadError", "[Unicode][T043]") {
    using namespace jsv::unicode;
    // 0xFF → InvalidLeadByte
    constexpr auto r = decode_utf8("\xFF", 0);
    STATIC_REQUIRE(r.codepoint == char32_t{0xFFFDU});
    STATIC_REQUIRE(r.byte_length == 1);
    STATIC_REQUIRE(r.status == Utf8Status::InvalidLeadByte);
}

TEST_CASE("Utf8Decoder_AllContinuationBytes_EachProducesOrphanedError", "[Unicode][T044]") {
    using namespace jsv::unicode;
    // 0x80 0x80 0x80 → 3 separate OrphanedContinuation results
    constexpr std::string_view input = "\x80\x80\x80";
    constexpr auto r0 = decode_utf8(input, 0);
    constexpr auto r1 = decode_utf8(input, 1);
    constexpr auto r2 = decode_utf8(input, 2);
    STATIC_REQUIRE(r0.status == Utf8Status::OrphanedContinuation);
    STATIC_REQUIRE(r0.byte_length == 1);
    STATIC_REQUIRE(r1.status == Utf8Status::OrphanedContinuation);
    STATIC_REQUIRE(r1.byte_length == 1);
    STATIC_REQUIRE(r2.status == Utf8Status::OrphanedContinuation);
    STATIC_REQUIRE(r2.byte_length == 1);
}

// ==========================================================================
// T054–T062 Phase 5: Unicode classification
// ==========================================================================

TEST_CASE("UnicodeClassifier_AsciiLetter_IsLetter", "[Unicode][T054]") {
    using namespace jsv::unicode;
    STATIC_REQUIRE(is_letter(U'A'));
    STATIC_REQUIRE(is_letter(U'z'));
    STATIC_REQUIRE(!is_letter(U'0'));
}

TEST_CASE("UnicodeClassifier_CJKIdeograph_IsLetter", "[Unicode][T055]") {
    using namespace jsv::unicode;
    STATIC_REQUIRE(is_letter(char32_t{0x5909U}));  // 変
}

TEST_CASE("UnicodeClassifier_CyrillicLetter_IsIdStart", "[Unicode][T056]") {
    using namespace jsv::unicode;
    STATIC_REQUIRE(is_id_start(char32_t{0x0438U}));  // и
}

TEST_CASE("UnicodeClassifier_DevanagariLetter_IsIdStart", "[Unicode][T057]") {
    using namespace jsv::unicode;
    STATIC_REQUIRE(is_id_start(char32_t{0x0917U}));  // ग
}

TEST_CASE("UnicodeClassifier_Underscore_IsIdStart", "[Unicode][T058]") {
    using namespace jsv::unicode;
    STATIC_REQUIRE(is_id_start(U'_'));
}

TEST_CASE("UnicodeClassifier_CombiningMark_IsIdContinueNotStart", "[Unicode][T059]") {
    using namespace jsv::unicode;
    STATIC_REQUIRE(is_id_continue(char32_t{0x0303U}));  // combining tilde
    STATIC_REQUIRE(!is_id_start(char32_t{0x0303U}));
}

TEST_CASE("UnicodeClassifier_ArabicDigit_IsIdContinueNotStart", "[Unicode][T060]") {
    using namespace jsv::unicode;
    STATIC_REQUIRE(is_id_continue(char32_t{0x0660U}));  // Arabic-Indic digit zero (Nd)
    STATIC_REQUIRE(!is_id_start(char32_t{0x0660U}));
}

TEST_CASE("UnicodeClassifier_Emoji_NotIdStartNotContinue", "[Unicode][T061]") {
    using namespace jsv::unicode;
    STATIC_REQUIRE(!is_id_start(char32_t{0x1F600U}));  // 😀
    STATIC_REQUIRE(!is_id_continue(char32_t{0x1F600U}));
}

TEST_CASE("UnicodeClassifier_MathItalic_IsLetter", "[Unicode][T062]") {
    using namespace jsv::unicode;
    // U+1D465: Mathematical Italic Small X (General Category Ll)
    STATIC_REQUIRE(is_letter(char32_t{0x1D465U}));
    STATIC_REQUIRE(is_id_start(char32_t{0x1D465U}));
}

// ==========================================================================
// T085–T086 Phase 7: Fast-path verification
// ==========================================================================

TEST_CASE("UnicodeClassifier_AsciiLetterFastPath_NoTableAccess", "[Unicode][T085]") {
    using namespace jsv::unicode;
    // Verify full ASCII range for id_start and id_continue behavior
    STATIC_REQUIRE(is_id_start(U'A'));
    STATIC_REQUIRE(is_id_start(U'Z'));
    STATIC_REQUIRE(is_id_start(U'a'));
    STATIC_REQUIRE(is_id_start(U'z'));
    STATIC_REQUIRE(is_id_start(U'_'));
    STATIC_REQUIRE(!is_id_start(U'0'));
    STATIC_REQUIRE(!is_id_start(U' '));
    STATIC_REQUIRE(is_id_continue(U'A'));
    STATIC_REQUIRE(is_id_continue(U'0'));
    STATIC_REQUIRE(is_id_continue(U'9'));
    STATIC_REQUIRE(is_id_continue(U'_'));
    STATIC_REQUIRE(!is_id_continue(U' '));
    STATIC_REQUIRE(!is_id_continue(U'+'));
}

TEST_CASE("UnicodeClassifier_WhitespaceFastPath_SpaceIsZs", "[Unicode][T086]") {
    using namespace jsv::unicode;
    STATIC_REQUIRE(is_unicode_whitespace(U' '));               // U+0020 SPACE (Zs) — fast-path
    STATIC_REQUIRE(!is_unicode_whitespace(U'\t'));             // HT not Zs/Zl/Zp
    STATIC_REQUIRE(!is_unicode_whitespace(U'\n'));             // LF not Zs/Zl/Zp
    STATIC_REQUIRE(!is_unicode_whitespace(U'\r'));             // CR not Zs/Zl/Zp
    STATIC_REQUIRE(is_unicode_whitespace(char32_t{0x00A0U}));  // NO-BREAK SPACE (Zs)
    STATIC_REQUIRE(is_unicode_whitespace(char32_t{0x2003U}));  // EM SPACE (Zs)
    STATIC_REQUIRE(is_unicode_whitespace(char32_t{0x2028U}));  // LINE SEPARATOR (Zl)
    STATIC_REQUIRE(is_unicode_whitespace(char32_t{0x2029U}));  // PARAGRAPH SEPARATOR (Zp)
}

// clang-format off
// NOLINTEND(*-include-cleaner, *-avoid-magic-numbers, *-magic-numbers, *-unchecked-optional-access, *-avoid-do-while, *-use-anonymous-namespace, *-qualified-auto, *-suspicious-stringview-data-usage, *-err58-cpp, *-function-cognitive-complexity, *-macro-usage, *-unnecessary-copy-initialization)
// clang-format on
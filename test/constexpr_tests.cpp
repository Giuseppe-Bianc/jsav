#include <catch2/catch_test_macros.hpp>
// clang-format off
// NOLINTBEGIN(*-include-cleaner, *-avoid-magic-numbers, *-magic-numbers, *-unchecked-optional-access, *-avoid-do-while, *-use-anonymous-namespace, *-qualified-auto, *-suspicious-stringview-data-usage, *-err58-cpp, *-function-cognitive-complexity, *-macro-usage, *-unnecessary-copy-initialization)
// clang-format on

#include <jsav/jsav.hpp>

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

// clang-format off
// NOLINTEND(*-include-cleaner, *-avoid-magic-numbers, *-magic-numbers, *-unchecked-optional-access, *-avoid-do-while, *-use-anonymous-namespace, *-qualified-auto, *-suspicious-stringview-data-usage, *-err58-cpp, *-function-cognitive-complexity, *-macro-usage, *-unnecessary-copy-initialization)
// clang-format on

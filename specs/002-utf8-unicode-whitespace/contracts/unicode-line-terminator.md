# Contract: Unicode Line Terminator Classification

**Module**: `jsv::unicode` (in `UnicodeData.hpp`)
**Date**: 2026-03-02

## Function Signature

```cpp
[[nodiscard]] constexpr bool is_unicode_line_terminator(char32_t cp) noexcept;
```

## Semantics

Returns `true` if and only if `cp` is one of the three Unicode line terminator code points
that are handled in the non-ASCII whitespace path:

| Code Point | Name | Category |
|------------|------|----------|
| U+0085 | NEXT LINE (NEL) | Cc |
| U+2028 | LINE SEPARATOR | Zl |
| U+2029 | PARAGRAPH SEPARATOR | Zp |

Returns `false` for all other code points, including:

- U+000A (LF) — handled in ASCII path, not by this function
- U+000D (CR) — not a line terminator per project design
- U+000B (VT), U+000C (FF) — whitespace but not line terminators

## Constraints

- `constexpr`: must be evaluable at compile time
- `noexcept`: must not throw
- `[[nodiscard]]`: caller must use return value
- Stateless: no side effects, no dependencies beyond the input parameter

## Usage Context

Called by `Lexer::skip_unicode_whitespace()` to determine whether a decoded non-ASCII
whitespace code point should trigger a line increment + column reset (line terminator)
or simply advance the column (non-line-terminating whitespace).

## Relationship to is_unicode_whitespace

`is_unicode_whitespace()` covers Zs/Zl/Zp categories. `is_unicode_line_terminator()` covers
a different set: U+0085 (Cc, not in Zs/Zl/Zp) plus U+2028 (Zl) and U+2029 (Zp). The two
functions overlap on U+2028 and U+2029 but serve different purposes:

- `is_unicode_whitespace(cp)` → "should the lexer skip this as whitespace?"
- `is_unicode_line_terminator(cp)` → "should the lexer also increment the line counter?"

## Test Contract

The truth table must be verified by `STATIC_REQUIRE` in `constexpr_tests.cpp`:

```cpp
STATIC_REQUIRE(is_unicode_line_terminator(U'\u0085'));   // NEL
STATIC_REQUIRE(is_unicode_line_terminator(U'\u2028'));   // LINE SEPARATOR
STATIC_REQUIRE(is_unicode_line_terminator(U'\u2029'));   // PARAGRAPH SEPARATOR
STATIC_REQUIRE_FALSE(is_unicode_line_terminator(U'\n')); // LF — ASCII path
STATIC_REQUIRE_FALSE(is_unicode_line_terminator(U'\r')); // CR — not a line terminator
STATIC_REQUIRE_FALSE(is_unicode_line_terminator(U' '));  // SPACE
```

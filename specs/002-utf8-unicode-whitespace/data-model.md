# Data Model: Full UTF-8 Unicode Whitespace Support in Lexer

**Feature Branch**: `002-utf8-unicode-whitespace`
**Date**: 2026-03-02

## Entities

### WhitespaceCodePoint (conceptual — no new struct)

The 26 `\p{White_Space}` code points, classified by handling path and behavior.

| Code Point | Name | UTF-8 Bytes | Path | Line Terminator | Column Delta |
|------------|------|-------------|------|-----------------|-------------|
| U+0009 | HT (tab) | 1 (0x09) | ASCII switch | No | +1 |
| U+000A | LF | 1 (0x0A) | ASCII switch | Yes | reset to 1 |
| U+000B | VT | 1 (0x0B) | ASCII switch (**NEW**) | No | +1 |
| U+000C | FF | 1 (0x0C) | ASCII switch (**NEW**) | No | +1 |
| U+000D | CR | 1 (0x0D) | ASCII switch | No (plain ws) | +1 |
| U+0020 | SPACE | 1 (0x20) | ASCII switch | No | +1 |
| U+0085 | NEL | 2 (0xC2 0x85) | Non-ASCII (**NEW**) | Yes | reset to 1 |
| U+00A0 | NBSP | 2 (0xC2 0xA0) | Non-ASCII (existing) | No | +2 |
| U+1680 | OGHAM SPACE | 3 (0xE1 0x9A 0x80) | Non-ASCII (existing) | No | +3 |
| U+2000 | EN QUAD | 3 (0xE2 0x80 0x80) | Non-ASCII (existing) | No | +3 |
| U+2001 | EM QUAD | 3 (0xE2 0x80 0x81) | Non-ASCII (existing) | No | +3 |
| U+2002 | EN SPACE | 3 (0xE2 0x80 0x82) | Non-ASCII (existing) | No | +3 |
| U+2003 | EM SPACE | 3 (0xE2 0x80 0x83) | Non-ASCII (existing) | No | +3 |
| U+2004 | THREE-PER-EM SPACE | 3 (0xE2 0x80 0x84) | Non-ASCII (existing) | No | +3 |
| U+2005 | FOUR-PER-EM SPACE | 3 (0xE2 0x80 0x85) | Non-ASCII (existing) | No | +3 |
| U+2006 | SIX-PER-EM SPACE | 3 (0xE2 0x80 0x86) | Non-ASCII (existing) | No | +3 |
| U+2007 | FIGURE SPACE | 3 (0xE2 0x80 0x87) | Non-ASCII (existing) | No | +3 |
| U+2008 | PUNCTUATION SPACE | 3 (0xE2 0x80 0x88) | Non-ASCII (existing) | No | +3 |
| U+2009 | THIN SPACE | 3 (0xE2 0x80 0x89) | Non-ASCII (existing) | No | +3 |
| U+200A | HAIR SPACE | 3 (0xE2 0x80 0x8A) | Non-ASCII (existing) | No | +3 |
| U+2028 | LINE SEPARATOR | 3 (0xE2 0x80 0xA8) | Non-ASCII (existing) | Yes | reset to 1 |
| U+2029 | PARAGRAPH SEPARATOR | 3 (0xE2 0x80 0xA9) | Non-ASCII (existing) | Yes | reset to 1 |
| U+202F | NARROW NBSP | 3 (0xE2 0x80 0xAF) | Non-ASCII (existing) | No | +3 |
| U+205F | MEDIUM MATH SPACE | 3 (0xE2 0x81 0x9F) | Non-ASCII (existing) | No | +3 |
| U+3000 | IDEOGRAPHIC SPACE | 3 (0xE3 0x80 0x80) | Non-ASCII (existing) | No | +3 |

### Line Terminator Set

A subset of whitespace code points that trigger line increment + column reset.

| Code Point | Name | Handled In |
|------------|------|-----------|
| U+000A | LF | ASCII switch (existing) |
| U+0085 | NEL | `skip_unicode_whitespace()` (**NEW**) |
| U+2028 | LINE SEPARATOR | `skip_unicode_whitespace()` (existing) |
| U+2029 | PARAGRAPH SEPARATOR | `skip_unicode_whitespace()` (existing) |

**Not a line terminator**: U+000D (CR) — consumed as plain whitespace. CR+LF → single line
increment via LF.

### New Function: `is_unicode_line_terminator(char32_t cp)`

**Location**: `include/jsav/lexer/unicode/UnicodeData.hpp` (auto-generated)
**Added via**: `scripts/generate_unicode_tables.py` → `generate_header()` template
**Namespace**: `jsv::unicode`
**Signature**: `[[nodiscard]] constexpr bool is_unicode_line_terminator(char32_t cp) noexcept`

```text
Returns true iff cp ∈ {U+0085, U+2028, U+2029}
```

**Validation rules**: None — pure classification. Input is any `char32_t`.

**State transitions**: N/A — stateless function.

## Modified Functions

### `Lexer::skip_whitespace_and_comments()`

**Current behavior**: Handles space, tab, CR, LF, `//` comments, `/* */` comments, non-ASCII
via `skip_unicode_whitespace()`.

**New behavior**: Also handles VT (0x0B) and FF (0x0C) in the ASCII fast-path. Structured as:

```text
while(!at_end):
  c = peek_byte()
  
  if c < 0x80:                    ← ASCII fast-path (Tier 1)
    switch(c):
      ' ', '\t', '\r':  advance_byte(); continue   ← existing
      '\v', '\f':        advance_byte(); continue   ← NEW (FR-002)
      '\n':              advance_codepoint(); continue  ← existing (line terminator)
      '/':               check // or /* → skip comment  ← existing
      default:           break out of loop              ← existing
  else:                           ← Non-ASCII path (Tier 2)
    skip_unicode_whitespace()     ← existing + NEL addition
```

### `Lexer::skip_unicode_whitespace()`

**Current behavior**: Decodes UTF-8 → checks `is_unicode_whitespace()` → handles U+2028/U+2029
as line terminators.

**New behavior**: After decode, checks for NEL (U+0085) FIRST (before `is_unicode_whitespace()`
which returns false for NEL). If NEL: increment line, reset column, return true.

```text
decode_utf8 → res
if res.status != Ok → return false
if res.codepoint == U+0085:
  m_pos += res.byte_length (2)
  ++m_line
  m_column = 1
  return true
if is_unicode_whitespace(res.codepoint):
  if res.codepoint == U+2028 || res.codepoint == U+2029:
    line terminator handling (existing)
  else:
    advance by byte_length (existing)
  return true
return false
```

## Relationships

```text
skip_whitespace_and_comments()
  ├── [ASCII] direct byte handling (space, tab, CR, VT, FF, LF)
  ├── [ASCII] skip_block_comment() / line comment skip
  └── [non-ASCII] skip_unicode_whitespace()
        ├── unicode::decode_utf8() [from Utf8.hpp]
        ├── NEL check (U+0085) [NEW inline]
        ├── unicode::is_unicode_whitespace() [from UnicodeData.hpp, unchanged]
        └── unicode::is_unicode_line_terminator() [NEW in UnicodeData.hpp via generator script]
```

Note: `is_unicode_line_terminator()` is used by `skip_unicode_whitespace()` to replace the
inline `cp == U'\u2028' || cp == U'\u2029'` check, making the line terminator set a single
source of truth. It also enables constexpr testing of the truth table.

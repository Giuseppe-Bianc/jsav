# Data Model: Lexer Full UTF-8 Support

**Branch**: `001-lexer-utf8-support` | **Date**: 2026-03-01

## Entities

### Utf8Status (enum class : std::uint8_t)

Status code returned by the UTF-8 decoder indicating the validity of a decoded sequence.

| Enumerator | Value | Description |
|------------|-------|-------------|
| `Ok` | 0 | Valid UTF-8 sequence decoded successfully |
| `Overlong` | 1 | Overlong encoding detected (e.g., U+002F encoded as 0xC0 0xAF) |
| `Surrogate` | 2 | Decoded code point falls in surrogate range U+D800–U+DFFF |
| `OutOfRange` | 3 | Decoded code point exceeds U+10FFFF |
| `OrphanedContinuation` | 4 | Continuation byte (0x80–0xBF) without preceding lead byte |
| `TruncatedSequence` | 5 | Lead byte not followed by expected number of continuation bytes |
| `InvalidLeadByte` | 6 | Byte 0xF5–0xFF which cannot start any valid UTF-8 sequence |

**Validation rules**: Must be `std::uint8_t`-backed for compact storage. Must be constexpr-compatible.

### Utf8DecodeResult (struct, POD)

Result of decoding a single UTF-8 sequence starting at a given byte offset.

| Field | Type | Description |
|-------|------|-------------|
| `codepoint` | `char32_t` | Decoded code point (U+FFFD if error) |
| `byte_length` | `std::uint8_t` | Number of bytes consumed (maximal subpart on error) |
| `status` | `Utf8Status` | Decode outcome |

**Layout**: 8 bytes (4 + 1 + 1 + 2 padding). Passed by value.

**Validation rules**:

- `byte_length` is always in range [1, 4].
- When `status == Ok`: `codepoint` is a valid Unicode scalar value (U+0000–U+D7FF
  or U+E000–U+10FFFF), `byte_length` matches the minimal encoding length.
- When `status != Ok`: `codepoint` is U+FFFD (replacement character),
  `byte_length` is the maximal subpart length per Unicode Standard D93b.

**Constexpr**: Aggregate-initializable, usable in `constexpr` context.

### CodepointRange (struct, POD)

A contiguous range of Unicode code points used in lookup tables.

| Field | Type | Description |
|-------|------|-------------|
| `first` | `char32_t` | First code point in range (inclusive) |
| `last` | `char32_t` | Last code point in range (inclusive) |

**Layout**: 8 bytes (4 + 4).

**Validation rules**:

- `first <= last`.
- Ranges within an array are sorted by `first` and non-overlapping:
  `ranges[i].last < ranges[i+1].first`.

**Constexpr**: Aggregate-initializable, used as element type in
`constexpr std::array<CodepointRange, N>`.

### Unicode Classification Tables (constexpr arrays)

Three static sorted arrays of `CodepointRange`, defined in the generated header
`include/jsav/lexer/unicode/UnicodeData.hpp`.

| Array | Categories | Estimated size | Purpose |
|-------|-----------|----------------|---------|
| `id_start_ranges` | L (Lu, Ll, Lt, Lm, Lo) | ~750 ranges, ~6 KB | Identifier start classification |
| `id_continue_ranges` | L + M (Mn, Mc, Me) + N (Nd, Nl, No) | ~850 ranges, ~6.8 KB | Identifier continue classification |
| `whitespace_ranges` | Zs, Zl, Zp | ~18 ranges, ~144 B | Unicode whitespace recognition |

**Generation**: Produced by `scripts/generate_unicode_tables.py` from UnicodeData.txt
(Unicode 16.0.0). Committed to repository as C++ source.

**Invariants**:

- Arrays are sorted by `first`.
- No overlapping ranges within an array.
- `static_assert` guards verify range count within expected bounds.

## Existing Entity Modifications

### Token (unchanged)

No changes to `Token` or `TokenKind`. The existing `TokenKind::Error` and
`TokenKind::IdentifierUnicode` already cover the new use cases:

- Malformed UTF-8 → `TokenKind::Error` with diagnostic text
- Unicode identifiers → `TokenKind::IdentifierUnicode` (already exists)
- Unexpected Unicode characters → `TokenKind::Error`

### Lexer (modified interface)

| Change | Before | After |
|--------|--------|-------|
| `peek_codepoint()` return | `std::uint32_t` | `Utf8DecodeResult` (or internal wrapper using it) |
| `advance_codepoint()` return | `std::uint32_t` | `Utf8DecodeResult` (or internal wrapper using it) |
| `is_xid_start()` | Static method, hardcoded ranges | Delegates to `jsv::unicode::is_id_start()` |
| `is_xid_continue()` | Static method, hardcoded ranges | Delegates to `jsv::unicode::is_id_continue()` |
| `skip_whitespace_and_comments()` | ASCII-only whitespace | Also checks `jsv::unicode::is_unicode_whitespace()` for non-ASCII bytes |
| `tokenize()` | No BOM handling | Skips UTF-8 BOM (0xEF 0xBB 0xBF) at start of input |

### SourceLocation (unchanged)

Column tracking remains byte-based, consistent with current approach and spec
assumptions. No changes needed.

## Relationships

```text
┌──────────────────────┐
│  Lexer               │
│  ─────               │
│  m_source: sv        │──reads──▶ Source buffer (std::string, owned externally)
│  m_pos, m_line, m_col│
├──────────────────────┤
│  tokenize()          │──produces──▶ std::vector<Token>
│  next_token()        │
│  peek_codepoint()    │──calls──▶ jsv::unicode::decode_utf8()
│  advance_codepoint() │──calls──▶ jsv::unicode::decode_utf8()
│  scan_identifier_*() │──calls──▶ jsv::unicode::is_id_start()
│                      │──calls──▶ jsv::unicode::is_id_continue()
│  skip_whitespace_*() │──calls──▶ jsv::unicode::is_unicode_whitespace()
└──────────────────────┘

┌──────────────────────────────┐
│  jsv::unicode (new module)   │
│  ──────────────────────      │
│  decode_utf8()               │──uses──▶ Utf8DecodeResult, Utf8Status
│  is_letter()                 │──uses──▶ id_start_ranges (constexpr array)
│  is_id_start()               │──uses──▶ is_letter() + underscore check
│  is_id_continue()            │──uses──▶ id_continue_ranges (constexpr array)
│  is_unicode_whitespace()     │──uses──▶ whitespace_ranges (constexpr array)
└──────────────────────────────┘

┌──────────────────────────────────────────┐
│  CodepointRange / Lookup Tables          │
│  (generated by generate_unicode_tables.py│
│   from UnicodeData.txt Unicode 16.0.0)   │
└──────────────────────────────────────────┘
```

## State Transitions

### UTF-8 Decoder State Machine

The decoder is stateless (pure function). Each call to `decode_utf8(input, offset)`
processes bytes starting at `offset` and returns immediately. No persistent state.

### Lexer Token Production (updated flow)

```text
next_token():
  1. skip_whitespace_and_comments()
     ├── ASCII whitespace (0x09, 0x0A, 0x0D, 0x20) → consume, loop
     ├── Non-ASCII byte → decode_utf8()
     │   ├── Valid + is_unicode_whitespace() → consume, loop
     │   └── Valid + not whitespace → break (not WS)
     │   └── Invalid → break (will be handled as error token)
     └── '//' or '/*' → consume comment, loop
  
  2. if at_end → Eof token
  
  3. Dispatch by first byte:
     ├── ASCII digit → scan_numeric_literal()
     ├── '#' → scan_hash_numeric()
     ├── '"' → scan_string_literal()
     ├── '\'' → scan_char_literal()
     ├── ASCII alpha or '_' → scan_identifier_or_keyword()
     ├── Non-ASCII byte → decode_utf8()
     │   ├── Valid + is_id_start() → scan_identifier_or_keyword(seen_unicode=true)
     │   ├── Valid + not recognized → error_token("unexpected Unicode character U+XXXX")
     │   └── Invalid (status != Ok) → error_token("malformed UTF-8 sequence")
     └── ASCII operator/punctuation → scan_operator_or_punctuation()
```

### String Literal UTF-8 Validation (new behavior)

```text
scan_string_literal():
  For each byte inside quotes:
    ├── ASCII byte → process normally (escape sequences, etc.)
    └── Non-ASCII byte → decode_utf8()
        ├── Valid → advance past sequence, continue
        └── Invalid → entire literal becomes error_token
```

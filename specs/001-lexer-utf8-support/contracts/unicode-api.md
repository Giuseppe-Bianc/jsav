# Contract: jsv::unicode Public API

**Module**: `include/jsav/lexer/unicode/`
**Consumer**: `jsv::Lexer` (internal), future `jsv::Parser` and semantic analysis

## Header: Utf8.hpp

### Types

```cpp
namespace jsv::unicode {

    enum class Utf8Status : std::uint8_t {
        Ok,                    // Valid sequence
        Overlong,              // Overlong encoding
        Surrogate,             // Surrogate code point (U+D800–U+DFFF)
        OutOfRange,            // Code point > U+10FFFF
        OrphanedContinuation,  // Continuation byte without lead byte
        TruncatedSequence,     // Lead byte with missing continuation bytes
        InvalidLeadByte,       // Byte 0xF5–0xFF
    };

    struct Utf8DecodeResult {
        char32_t     codepoint;    // U+FFFD on error
        std::uint8_t byte_length;  // Bytes consumed (1–4)
        Utf8Status   status;       // Error classification
    };

}  // namespace jsv::unicode
```

### Functions

```cpp
namespace jsv::unicode {

    /// Decode one UTF-8 sequence starting at input[offset].
    /// Returns decoded codepoint, bytes consumed, and validation status.
    /// On error, codepoint is U+FFFD and byte_length is the maximal subpart length.
    /// Precondition: offset < input.size()
    [[nodiscard]] constexpr Utf8DecodeResult
    decode_utf8(std::string_view input, std::size_t offset) noexcept;

    /// Validate that input[offset..] starts with a well-formed UTF-8 sequence.
    /// Equivalent to: decode_utf8(input, offset).status == Utf8Status::Ok
    [[nodiscard]] constexpr bool
    is_valid_utf8_at(std::string_view input, std::size_t offset) noexcept;

}  // namespace jsv::unicode
```

### Behavioral Contract

| Condition | Behavior |
|-----------|----------|
| `input[offset]` is ASCII (0x00–0x7F) | Returns `{codepoint, 1, Ok}` |
| Valid 2-byte sequence at offset | Returns `{codepoint, 2, Ok}` |
| Valid 3-byte sequence at offset | Returns `{codepoint, 3, Ok}` |
| Valid 4-byte sequence at offset | Returns `{codepoint, 4, Ok}` |
| Lead byte 0xC0 or 0xC1 | Returns `{U+FFFD, 1, Overlong}` |
| Overlong 3-byte (e.g., E0 80–9F xx) | Returns `{U+FFFD, 1, Overlong}` |
| Overlong 4-byte (e.g., F0 80–8F xx xx) | Returns `{U+FFFD, 1, Overlong}` |
| Surrogate (ED A0–BF xx) | Returns `{U+FFFD, 1, Surrogate}` |
| Out-of-range (F4 90+ xx xx) | Returns `{U+FFFD, 1, OutOfRange}` |
| Lead byte 0xF5–0xFF | Returns `{U+FFFD, 1, InvalidLeadByte}` |
| Orphaned continuation (0x80–0xBF) | Returns `{U+FFFD, 1, OrphanedContinuation}` |
| Truncated (lead + insufficient continuation) | Returns `{U+FFFD, maximal_subpart_len, TruncatedSequence}` |

## Header: UnicodeData.hpp

### Types

```cpp
namespace jsv::unicode {

    struct CodepointRange {
        char32_t first;  // Inclusive start
        char32_t last;   // Inclusive end
    };

}  // namespace jsv::unicode
```

### Functions

```cpp
namespace jsv::unicode {

    /// True if cp has Unicode General Category L (Letter).
    [[nodiscard]] constexpr bool is_letter(char32_t cp) noexcept;

    /// True if cp may start an identifier: \p{Letter} or '_' (U+005F).
    [[nodiscard]] constexpr bool is_id_start(char32_t cp) noexcept;

    /// True if cp may continue an identifier:
    /// \p{Letter} | \p{Mark} | \p{Number} | '_' | ASCII digits.
    [[nodiscard]] constexpr bool is_id_continue(char32_t cp) noexcept;

    /// True if cp is Unicode whitespace: General Category Zs, Zl, or Zp.
    [[nodiscard]] constexpr bool is_unicode_whitespace(char32_t cp) noexcept;

}  // namespace jsv::unicode
```

### Behavioral Contract

| Function | ASCII fast-path | Non-ASCII behavior |
|----------|----------------|--------------------|
| `is_letter(cp)` | `true` if `cp` in `[A-Za-z]` | Binary search in `id_start_ranges` |
| `is_id_start(cp)` | `true` if `cp` in `[A-Za-z_]` | `cp == U'_' \|\| is_letter(cp)` |
| `is_id_continue(cp)` | `true` if `cp` in `[A-Za-z0-9_]` | Binary search in `id_continue_ranges` |
| `is_unicode_whitespace(cp)` | `false` (ASCII WS not in Zs/Zl/Zp) | Binary search in `whitespace_ranges` |

**Note**: ASCII whitespace (U+0020 SPACE) is General Category Zs and IS included in
`whitespace_ranges`. The ASCII fast-path for `is_unicode_whitespace` should return
`true` for U+0020. Other ASCII whitespace (tab, LF, CR) are NOT Zs/Zl/Zp.

## Lexer Contract Changes

### New behavior: BOM handling

```text
tokenize():
  If source starts with bytes EF BB BF:
    Skip 3 bytes (advance m_pos, m_column)
    No token emitted
```

### New behavior: Unicode whitespace

```text
skip_whitespace_and_comments():
  For non-ASCII bytes:
    decode_utf8() → if valid and is_unicode_whitespace():
      consume and continue skipping
```

### New behavior: Malformed UTF-8 error recovery

```text
next_token():
  For non-ASCII bytes where decode gives status != Ok:
    Emit error_token covering the maximal subpart bytes
    Diagnostic: "malformed UTF-8 sequence at byte offset {offset}"
```

### New behavior: String/char literal UTF-8 validation

```text
scan_string_literal() / scan_char_literal():
  For non-ASCII bytes inside literal:
    decode_utf8() → if status != Ok:
      Entire literal becomes error_token
      Diagnostic: "malformed UTF-8 in string literal"
```

### Preserved behavior: ASCII backward compatibility

All ASCII-only input produces identical tokens to the pre-change lexer. This is
verified by running the existing test suite after changes (FR-017).

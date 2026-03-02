# Contract: Lexer Whitespace Handling

**Module**: `jsv::Lexer` (in `Lexer.cpp`)
**Date**: 2026-03-02

## Modified Behavior: skip_whitespace_and_comments

### Pre-condition

- `m_pos` is within bounds of `m_source` (or at end)
- `m_line` and `m_column` reflect the correct position

### Post-condition

- `m_pos` is advanced past all contiguous whitespace and comments
- `m_line` and `m_column` are updated correctly for all consumed characters:
  - ASCII space, tab, CR, VT, FF: column += 1
  - LF: line += 1, column = 1
  - NEL (U+0085): line += 1, column = 1
  - U+2028, U+2029: line += 1, column = 1
  - Other Unicode whitespace (Zs): column += UTF-8 byte length (2 or 3)
- The next byte at `m_pos` (if not at end) is NOT whitespace and NOT the start of a comment

### Invariants

- Never reads past `m_source.size()`
- Never enters infinite loop (each iteration either advances `m_pos` or breaks)
- Invalid UTF-8 bytes are not consumed — function returns, letting `next_token()` handle them

### Two-Tier Classification

```text
Tier 1 (ASCII, bit 7 = 0):
  switch(byte):
    0x20 (SPACE), 0x09 (HT), 0x0D (CR)  → advance 1 byte, column += 1
    0x0B (VT), 0x0C (FF)                  → advance 1 byte, column += 1  [NEW]
    0x0A (LF)                              → advance, line += 1, column = 1
    0x2F ('/')                             → check for // or /* comment
    anything else                          → break (not whitespace)

Tier 2 (non-ASCII, bit 7 = 1):
  decode_utf8 → (codepoint, byte_length, status)
  if status != Ok → break (not whitespace, don't consume)
  if codepoint == U+0085 (NEL) → line += 1, column = 1, advance  [NEW]
  if is_unicode_whitespace(codepoint):
    if is_unicode_line_terminator(codepoint) → line += 1, column = 1, advance
    else → column += byte_length, advance
  else → break (not whitespace)
```

## Modified Behavior: skip_unicode_whitespace

### Pre-condition

- `m_pos` points to a byte with bit 7 set (lead byte of potential multi-byte sequence)

### Post-condition

- If the byte sequence is valid UTF-8 AND the codepoint is Unicode whitespace (Zs/Zl/Zp) or
  NEL (U+0085): position and counters updated, returns `true`
- Otherwise: no state change, returns `false`

### NEL Special Case

U+0085 (NEL) is in category Cc, not Zs/Zl/Zp. `is_unicode_whitespace()` returns `false` for
it. The function checks for U+0085 explicitly BEFORE calling `is_unicode_whitespace()`.

## Error Handling

| Invalid Input | Behavior |
|---------------|----------|
| Lone continuation byte (0x80–0xBF) | `decode_utf8` returns OrphanedContinuation → not consumed |
| Truncated 2-byte at EOF | `decode_utf8` returns TruncatedSequence → not consumed |
| Truncated 3-byte at EOF | `decode_utf8` returns TruncatedSequence → not consumed |
| Overlong encoding of SPACE | `decode_utf8` returns Overlong → not consumed |
| 0xFE / 0xFF bytes | `decode_utf8` returns InvalidLeadByte → not consumed |
| Surrogate (0xED 0xA0–0xBF ...) | `decode_utf8` returns Surrogate → not consumed |
| Valid non-whitespace multi-byte | `is_unicode_whitespace` returns false → not consumed |

# Research Report: UTF-8 Unicode Lexer Implementation

**Feature**: UTF-8 Unicode Lexer Support  
**Branch**: `001-utf8-unicode-lexer`  
**Date**: 2026-02-28  
**Status**: Complete

---

## Research Task 1: UTF-8 Decoding in C++23 with constexpr

### Decision

Implement hand-written `constexpr` UTF-8 decoder as a state machine that validates and decodes 1-4 byte sequences at compile-time where possible, returning `std::optional<char32_t>` for valid code points or error indication for malformed sequences.

### Rationale

- **No runtime dependencies**: External Unicode libraries (ICU, utfcpp) add build complexity and runtime overhead
- **Compile-time verification**: `constexpr` allows UTF-8 validation logic to be tested at compile-time with known inputs
- **Full control**: Hand-written decoder can be optimized for compiler use cases (error recovery, precise position tracking)
- **C++23 capabilities**: `constexpr` supports loops, conditionals, and `std::optional` in constant expressions
- **Standards compliance**: RFC 3629 UTF-8 is well-specified and stable

### Alternatives Considered

| Alternative | Pros | Cons | Rejected Because |
|-------------|------|------|------------------|
| **ICU (International Components for Unicode)** | Full Unicode support, battle-tested | Large dependency (~10MB), runtime overhead, C API | Overkill for lexer; violates "zero additional dependencies" constraint |
| **utfcpp (UTF-8 C++ library)** | Header-only, lightweight | Runtime-only, no constexpr support, external dependency | Still an external dependency; hand-written solution is simpler |
| **std::u8string_view iteration** | Standard library | No validation, C++20 feature with limited constexpr | Doesn't validate UTF-8; doesn't handle error recovery |
| **Lookup table decoder** | Fast runtime performance | Large tables (~4KB), cache pressure | State machine is more compact and equally fast for lexer throughput |

### Implementation Approach

```cpp
constexpr std::optional<char32_t> decodeUtf8(std::string_view input, std::size_t& pos) noexcept {
    if (pos >= input.size()) return std::nullopt;
    
    auto byte = static_cast<unsigned char>(input[pos]);
    
    if (byte < 0x80) {
        // 1-byte sequence (ASCII)
        return char32_t{byte};
    } else if ((byte & 0xE0) == 0xC0) {
        // 2-byte sequence
        // Validate continuation bytes, check for overlong encoding
    } else if ((byte & 0xF0) == 0xE0) {
        // 3-byte sequence
        // Validate continuation bytes, check for surrogates (U+D800-U+DFFF)
    } else if ((byte & 0xF8) == 0xF0) {
        // 4-byte sequence
        // Validate continuation bytes, check max code point (U+10FFFF)
    } else {
        // Invalid start byte
        return std::nullopt;
    }
}
```

### Best Practices

- Validate continuation bytes (must be `0x80`-`0xBF`)
- Reject overlong encodings (e.g., `0xC0 0x80` for NUL)
- Reject surrogate halves (U+D800-U+DFFF)
- Reject code points > U+10FFFF
- Track byte position for error reporting

---

## Research Task 2: Unicode XID_Start/XID_Continue Classification

### Decision

Implement hand-written `constexpr` XID classification tables covering 10 scripts (Latin, Greek, Cyrillic, Arabic, Hebrew, Devanagari, Thai, CJK, Hangul, Bengali) using compressed range tables stored as `std::array` of code point ranges, with binary search for O(log n) lookup.

### Rationale

- **Zero runtime dependencies**: No need for Unicode data files or external libraries
- **Compile-time tables**: Range data can be `constexpr`, enabling compile-time validation
- **Targeted coverage**: 10 scripts cover the requirement without bloating tables with unused characters
- **Memory efficiency**: Range tables (~200 ranges) are more compact than bitmaps for sparse character sets
- **Deterministic performance**: Binary search provides predictable O(log n) lookup time

### Alternatives Considered

| Alternative | Pros | Cons | Rejected Because |
|-------------|------|------|------------------|
| **Full Unicode XID tables (UCD)** | Complete coverage | ~200KB data, requires parsing UnicodeData.txt | Overkill; violates "zero dependencies" constraint |
| **Bitmap-based classification** | O(1) lookup | 1MB+ for full BMP bitmap | Memory inefficient for sparse character sets |
| **Perfect hash function** | Fast, compact | Complex generation, requires external tooling | Adds build complexity; binary search is simpler |
| **std::iswalpha() with locale** | Standard library | Locale-dependent, no XID semantics, runtime-only | Not XID-compliant; not constexpr |

### Implementation Approach

```cpp
struct CodePointRange {
    char32_t start;
    char32_t end;
};

constexpr std::array<CodePointRange, N> xidStartRanges = {{
    // Latin (Basic + Extended)
    {U'A', U'Z'}, {U'a', U'z'}, {U'À', U'Ö'}, {U'Ø', U'ö'}, {U'ø', U'ÿ'},
    // Greek
    {U'Α', U'Ω'}, {U'α', U'ω'}, {U'ϐ', U'ϵ'},
    // Cyrillic
    {U'А', U'Я'}, {U'а', U'я'}, {U'Ё', U'Ё'}, {U'ё', U'ё'},
    // Arabic
    {U'ا', U'ي'},
    // Hebrew
    {U'א', U'ת'},
    // Devanagari
    {U'अ', U'ह'}, {U'क', U'ह'},
    // Bengali
    {U'অ', U'হ'},
    // Thai
    {U'ก', U'ฮ'},
    // CJK Unified Ideographs
    {U'\u4E00', U'\u9FFF'},
    // Hangul Syllables
    {U'\uAC00', U'\uD7AF'},
    // ... (underscore, other letters)
}};

constexpr bool isXidStart(char32_t cp) noexcept {
    return std::binary_search(xidStartRanges.begin(), xidStartRanges.end(), cp,
        [](const CodePointRange& range, char32_t val) {
            if (val < range.start) return true;
            if (val > range.end) return false;
            return false; // val is in range
        });
}
```

### Coverage Analysis

| Script | XID_Start Characters | XID_Continue Characters |
|--------|---------------------|------------------------|
| Latin | 52 (A-Z, a-z) + extended | 62 + digits 0-9 |
| Greek | 48 (capital + small) | + digits (Ͱ-ͳ, Ͷ-Ϳ) |
| Cyrillic | 66 (basic) + extended | + digits (Є, Ѕ, І, Ї) |
| Arabic | 42 (basic) | + digits (٠-٩) |
| Hebrew | 22 (basic) | + digits |
| Devanagari | 48 (अ-ह, क-ह) | + digits (०-९) |
| Bengali | 48 (অ-হ) | + digits (০-৯) |
| Thai | 44 (ก-ฮ) | + digits (๐-๙) |
| CJK | 20,992 (U+4E00-U+9FFF) | N/A (no digits) |
| Hangul | 11,172 syllables (U+AC00-U+D7AF) | + digits (๐-๙) |

### Best Practices

- Include combining diacritical marks (U+0300-U+036F) in XID_Continue
- Include script-specific digits in XID_Continue
- Exclude surrogate halves from all tables
- Use `char32_t` for code point representation
- Document exact Unicode version (15.0+) for reproducibility

---

## Research Task 3: Unicode Escape Sequence Handling (\uXXXX, \UXXXXXXXX)

### Decision

Implement escape sequence decoder as a `constexpr` function that validates hex digits, decodes to `char32_t`, validates Unicode scalar value (not surrogate, ≤ U+10FFFF), and returns `std::variant<char32_t, EscapeError>` for error reporting.

### Rationale

- **Standards compliance**: `\uXXXX` and `\UXXXXXXXX` are standard in C/C++ string literals
- **Compile-time validation**: Escape decoding can be `constexpr` for string literals
- **Precise error reporting**: Need to distinguish between invalid hex digits, surrogates, out-of-range
- **UTF-8 output**: Escapes decode to code points, which must be re-encoded to UTF-8 for token text

### Alternatives Considered

| Alternative | Pros | Cons | Rejected Because |
|-------------|------|------|------------------|
| **Raw string literals only** | Simpler lexer | No escape support, limits expressiveness | Users expect escape sequences for special characters |
| **Runtime-only decoding** | Simpler implementation | Can't validate at compile-time, slower | C++23 constexpr enables compile-time validation |
| **Accept any \uXXXX** | Simpler validation | Allows invalid surrogates, non-standard | Must reject surrogates per Unicode standard |

### Implementation Approach

```cpp
enum class EscapeError : uint8_t {
    InvalidHexDigit,
    SurrogateHalf,
    CodePointTooLarge,
    UnexpectedEnd
};

constexpr std::variant<char32_t, EscapeError> decodeUnicodeEscape(
    std::string_view input, 
    std::size_t& pos,
    int hexDigits  // 4 for \u, 8 for \U
) noexcept {
    char32_t codePoint = 0;
    
    for (int i = 0; i < hexDigits; ++i) {
        if (pos + i >= input.size()) {
            return EscapeError::UnexpectedEnd;
        }
        
        char c = input[pos + i];
        int digitValue = 0;
        
        if (c >= '0' && c <= '9') digitValue = c - '0';
        else if (c >= 'a' && c <= 'f') digitValue = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') digitValue = c - 'A' + 10;
        else return EscapeError::InvalidHexDigit;
        
        codePoint = (codePoint << 4) | digitValue;
    }
    
    // Validate Unicode scalar value
    if (codePoint >= 0xD800 && codePoint <= 0xDFFF) {
        return EscapeError::SurrogateHalf;
    }
    if (codePoint > 0x10FFFF) {
        return EscapeError::CodePointTooLarge;
    }
    
    pos += hexDigits;
    return codePoint;
}
```

### Best Practices

- Validate exactly 4 hex digits for `\u`, exactly 8 for `\U`
- Accept both uppercase and lowercase hex digits (a-f, A-F)
- Reject surrogate halves (U+D800-U+DFFF) with specific error
- Reject code points > U+10FFFF
- Report exact position of invalid hex digit for error messages

---

## Research Task 4: Error Token Insertion Pattern

### Decision

Implement error recovery via error token insertion: when lexer encounters invalid input, create `Token{TokenType::Error, errorKind, sourceSlice, position}` and continue lexing from next valid token boundary. Error token preserves source slice for diagnostic reporting.

### Rationale

- **Parser compatibility**: Parser receives well-formed token stream with explicit error markers
- **Error recovery**: Lexer can continue after errors, enabling multiple error reports per file
- **Source fidelity**: Original bytes preserved in error token for accurate error messages
- **Position tracking**: Byte offset in error token enables precise "caret diagnostics"

### Alternatives Considered

| Alternative | Pros | Cons | Rejected Because |
|-------------|------|------|------------------|
| **Halt on first error** | Simple implementation | Single error per file, poor UX | Users expect multiple error reports |
| **Skip to next line** | Simple recovery | Loses valid tokens on error line | Too aggressive; loses information |
| **Exception throwing** | Standard C++ error handling | Performance overhead, not constexpr | Exceptions incompatible with constexpr |
| **Error token insertion** | Preserves stream structure, multiple errors | Slightly more complex | Best balance of recovery and fidelity |

### Implementation Approach

```cpp
enum class LexicalError : uint8_t {
    InvalidUtf8StartByte,
    IncompleteUtf8Sequence,
    OverlongUtf8Encoding,
    SurrogateHalf,
    InvalidEscapeSequence,
    InvalidUnicodeEscape,
    UnterminatedString,
    UnterminatedCharacter,
    UnterminatedBlockComment
};

struct Token {
    TokenType type;
    std::optional<LexicalError> error;
    std::string_view text;  // Non-owning view into source buffer
    std::size_t byteOffset;
    std::size_t line;
    std::size_t column;
};

class Lexer {
public:
    std::vector<Token> tokenize(std::string_view source) {
        std::vector<Token> tokens;
        
        while (pos < source.size()) {
            skipWhitespaceAndComments();
            
            if (pos >= source.size()) break;
            
            auto token = nextToken(source);
            if (token.isError()) {
                // Insert error token and continue
                tokens.push_back(token);
                advancePastError();  // Move to next valid token boundary
            } else {
                tokens.push_back(token);
            }
        }
        
        return tokens;
    }
};
```

### Best Practices

- Error token includes `LexicalError` enum for categorization
- Error token includes source slice for diagnostics
- Lexer advances past error (at least 1 byte) to avoid infinite loops
- Error recovery is deterministic (same input → same token stream)
- Error tokens are materialized in vector alongside valid tokens

---

## Research Task 5: Zero-Copy Token Storage with std::string_view

### Decision

Use `std::string_view` for token text, storing non-owning views into the original source buffer. Lexer takes ownership of source buffer (as `std::string` or `std::string_view`), and all token views reference this buffer. Token vector owns the `Token` structs but not the underlying character data.

### Rationale

- **Performance**: Avoids allocating memory for each token's text
- **Memory efficiency**: Single source buffer vs. N allocations for N tokens
- **C++23 compatibility**: `std::string_view` is constexpr-friendly
- **Clear ownership**: Source buffer lifetime exceeds token vector lifetime
- **Zero-copy parsing**: No string copying during lexing

### Alternatives Considered

| Alternative | Pros | Cons | Rejected Because |
|-------------|------|------|------------------|
| **std::string per token** | Simple ownership | N allocations, poor performance | Unnecessary overhead for lexer |
| **String interning** | Deduplicates repeated identifiers | Complexity, runtime overhead | Not needed for compiler lexer |
| **Rope data structure** | Efficient for edits | Complexity, overkill | Lexer doesn't need rope semantics |
| **std::string_view** | Zero-copy, constexpr, simple | Requires careful lifetime management | Best choice with clear ownership model |

### Implementation Approach

```cpp
class Lexer {
private:
    std::string_view source;  // Non-owning view (or std::string if owning)
    std::size_t pos = 0;
    
public:
    // Option A: Non-owning (caller must keep source alive)
    explicit Lexer(std::string_view source) : source{source} {}
    
    // Option B: Owning (lexer owns source buffer)
    explicit Lexer(std::string source) 
        : sourceBuffer{std::move(source)}, source{sourceBuffer} {}
    
    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        while (pos < source.size()) {
            // Token text is std::string_view into source
            auto tokenText = source.substr(startPos, pos - startPos);
            tokens.push_back(Token{type, tokenText, startPos, line, column});
        }
        return tokens;
    }
    
private:
    std::string sourceBuffer;  // Only used in Option B
};

struct Token {
    TokenType type;
    std::string_view text;  // View into Lexer's source buffer
    std::size_t byteOffset;
    std::size_t line;
    std::size_t column;
};
```

### Lifetime Management

- **Owning model** (recommended): Lexer takes `std::string` by value, stores internally
- **Non-owning model**: Lexer takes `std::string_view`, caller must ensure source outlives tokens
- **Token vector**: Owns `Token` structs, but `text` views reference lexer's source
- **Rule**: Token vector must not outlive the Lexer that created it (or source buffer)

### Best Practices

- Document ownership semantics clearly in API
- Use `[[nodiscard]]` on tokenize() to prevent accidental discarding
- Consider move semantics for token vector to avoid copies
- Ensure source buffer is not modified while tokens reference it
- Use `const std::string_view` to prevent accidental mutation

---

## Research Task 6: Line Ending Handling (LF, CRLF, CR)

### Decision

Implement line ending detection as a `constexpr` function that recognizes LF (`\n`), CRLF (`\r\n`), and CR (`\r`) as line terminators. CRLF is treated as a single line ending (advance by 2 bytes, increment line count by 1). CR and LF are each treated as single line endings (advance by 1 byte, increment line count by 1).

### Rationale

- **Cross-platform compatibility**: Handles Unix (LF), Windows (CRLF), legacy Mac (CR)
- **Accurate line counting**: Essential for error reporting ("error on line 42")
- **Simple implementation**: State machine with 3 states for line ending detection
- **Standards compliance**: Matches C/C++ source file handling conventions

### Implementation Approach

```cpp
constexpr void advanceLine(std::string_view source, std::size_t& pos, 
                           std::size_t& line, std::size_t& column) noexcept {
    if (pos >= source.size()) return;
    
    unsigned char c = static_cast<unsigned char>(source[pos]);
    
    if (c == 0x0D) {  // CR
        if (pos + 1 < source.size() && 
            static_cast<unsigned char>(source[pos + 1]) == 0x0A) {
            // CRLF: skip both bytes
            pos += 2;
        } else {
            // CR only: skip 1 byte
            pos += 1;
        }
        line += 1;
        column = 0;
    } else if (c == 0x0A) {  // LF
        pos += 1;
        line += 1;
        column = 0;
    }
}
```

### Best Practices

- Handle CRLF atomically (don't count as 2 line endings)
- Reset column to 0 after each line ending
- Handle incomplete CRLF at end of file (CR without LF)
- Track line numbers as 0-based or 1-based consistently
- Consider column tracking for error reporting ("column 5")

---

## Consolidated Findings Summary

### Technology Choices

| Decision Area | Choice | Rationale |
|---------------|--------|-----------|
| **UTF-8 Decoding** | Hand-written constexpr state machine | Zero dependencies, compile-time validation, full control |
| **XID Classification** | Compressed range tables with binary search | Memory efficient, constexpr-friendly, targeted coverage |
| **Escape Sequences** | constexpr decoder with validation | Standards compliance, precise error reporting |
| **Error Recovery** | Error token insertion | Preserves token stream, enables multiple error reports |
| **Token Storage** | std::string_view (zero-copy) | Performance, memory efficiency, C++23 compatibility |
| **Line Endings** | Multi-style detection (LF/CRLF/CR) | Cross-platform compatibility, accurate line counting |

### Unicode Version

- **XID Properties**: Unicode 15.0 or later
- **UTF-8 Encoding**: RFC 3629 (only 1-4 byte sequences, excluding surrogates)
- **Scripts Covered**: Latin, Greek, Cyrillic, Arabic, Hebrew, Devanagari, Bengali, Thai, CJK, Hangul

### Performance Targets

- **Slowdown**: ≤10% vs ASCII-only input (measured as ratio of lexing times)
- **Memory**: Single source buffer + token vector (no per-token allocations)
- **Complexity**: O(n) lexing where n = source bytes; O(log m) XID lookup where m = number of ranges

### Testing Strategy

- **Compile-time tests**: UTF-8 decoding, escape sequence validation, XID classification
- **Runtime tests**: Full lexer with Unicode identifiers, error recovery, line counting
- **Coverage**: ≥95% branch coverage (verified via gcovr)
- **Conformance**: Unicode 15.0 Conformance Test Suite for UTF-8 validation

---

## References

- **RFC 3629**: UTF-8, a transformation format of ISO 10646 - https://www.rfc-editor.org/rfc/rfc3629
- **Unicode Standard Annex #31**: Identifier and Pattern Syntax - https://www.unicode.org/reports/tr31/
- **Unicode 15.0 Core Specification**: https://www.unicode.org/versions/Unicode15.0.0/
- **C++23 Standard**: ISO/IEC 14882:2023 (constexpr, std::string_view, std::optional)
- **Unicode Conformance Test Suite**: https://www.unicode.org/Public/15.0.0/ucd/

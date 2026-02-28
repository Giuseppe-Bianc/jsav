# Lexer Interface Contract

**Feature**: UTF-8 Unicode Lexer Support  
**Branch**: `001-utf8-unicode-lexer`  
**Date**: 2026-02-28  
**Version**: 1.0

---

## Overview

This document defines the public interface contract for the UTF-8 Unicode lexer component. The lexer is a core library component that transforms UTF-8 encoded source text into a materialized vector of tokens for parser consumption.

---

## Interface: Lexer

### Namespace

```cpp
namespace jsav::lexer {
    // All lexer entities reside in this namespace
}
```

### Class: Lexer

**Purpose**: Tokenize UTF-8 encoded source text into a vector of tokens.

**Header**: `#include <jsav/lexer/Lexer.hpp>`

#### Constructors

```cpp
// Owning constructor (recommended)
// Takes ownership of source buffer
explicit Lexer(std::string source);

// Non-owning constructor (advanced)
// Source must outlive Lexer and produced tokens
explicit Lexer(std::string_view source);
```

**Preconditions**:
- For owning constructor: none
- For non-owning constructor: `source.data()` must remain valid for lifetime of Lexer and all produced tokens

**Postconditions**:
- Lexer is ready for tokenization
- Internal state initialized (pos=0, line=1, column=0)

#### Public Methods

##### tokenize()

```cpp
[[nodiscard]] std::vector<Token> tokenize();
```

**Purpose**: Tokenize entire source and return materialized token vector.

**Returns**: `std::vector<Token>` containing all tokens from source

**Postconditions**:
- Vector contains at least one token (EOF token)
- Last token is `TokenType::EndOfFile`
- All tokens have valid `SourceLocation` with monotonically increasing `byteOffset`
- Error tokens inserted for all lexical errors encountered

**Exception Guarantee**: `noexcept(false)` - may throw `std::bad_alloc` on allocation failure

**Complexity**:
- Time: O(n) where n = source size in bytes
- Space: O(n + t) where t = number of tokens

**Example**:
```cpp
jsav::lexer::Lexer lexer{"int x = 42;"};
auto tokens = lexer.tokenize();
// tokens[0]: TokenType::Keyword ("int")
// tokens[1]: TokenType::IdentifierAscii ("x")
// tokens[2]: TokenType::Assign ("=")
// tokens[3]: TokenType::IntegerLiteral ("42")
// tokens[4]: TokenType::Semicolon (";")
// tokens[5]: TokenType::EndOfFile
```

---

##### tokenizeInPlace()

```cpp
void tokenizeInPlace();
```

**Purpose**: Tokenize source and store tokens internally for streaming access.

**Postconditions**:
- `tokens()` method returns reference to internal token vector
- Same guarantees as `tokenize()`

**Usage**: Preferred when lexer lifetime exceeds parser lifetime

---

##### tokens()

```cpp
[[nodiscard]] const std::vector<Token>& tokens() const noexcept;
```

**Purpose**: Access materialized tokens (after tokenization).

**Returns**: `const std::vector<Token>&` reference to token vector

**Preconditions**:
- `tokenize()` or `tokenizeInPlace()` has been called

**Returns**: Reference to internal token vector

**Complexity**: O(1)

---

##### locationFromOffset()

```cpp
[[nodiscard]] SourceLocation locationFromOffset(std::size_t byteOffset) const noexcept;
```

**Purpose**: Convert byte offset to SourceLocation (line, column).

**Parameters**:
- `byteOffset`: 0-based byte offset from start of source

**Returns**: `SourceLocation` with line and column corresponding to offset

**Preconditions**:
- `byteOffset <= source.size()`

**Postconditions**:
- `result.line` is 1-based line number containing offset
- `result.column` is 0-based column within line
- `result.byteOffset == byteOffset`

**Complexity**: O(n) in worst case (scans from last known position)

---

### Class: Token

**Purpose**: Represents a single lexical unit.

**Header**: `#include <jsav/lexer/Token.hpp>`

#### Constructors

```cpp
// Internal use only - constructed by Lexer
constexpr Token(TokenType type, std::string_view text, SourceLocation loc) noexcept;
constexpr Token(TokenType type, LexicalError error, std::string_view text, SourceLocation loc) noexcept;
```

#### Public Methods

##### type()

```cpp
[[nodiscard]] constexpr TokenType type() const noexcept;
```

**Returns**: Token type enumeration value

##### error()

```cpp
[[nodiscard]] constexpr std::optional<LexicalError> error() const noexcept;
```

**Returns**: `std::nullopt` for valid tokens, `LexicalError` value for error tokens

##### text()

```cpp
[[nodiscard]] constexpr std::string_view text() const noexcept;
```

**Returns**: Non-owning view of token text in source buffer

**Lifetime**: Valid while source buffer is alive

##### location()

```cpp
[[nodiscard]] constexpr SourceLocation location() const noexcept;
```

**Returns**: Source location (byte offset, line, column) of token start

##### isError()

```cpp
[[nodiscard]] constexpr bool isError() const noexcept;
```

**Returns**: `true` if token is an error token

##### isIdentifier()

```cpp
[[nodiscard]] constexpr bool isIdentifier() const noexcept;
```

**Returns**: `true` if token is `IdentifierAscii` or `IdentifierUnicode`

##### isLiteral()

```cpp
[[nodiscard]] constexpr bool isLiteral() const noexcept;
```

**Returns**: `true` if token is any literal type

---

### Enum: TokenType

**Purpose**: Strongly-typed enumeration of token kinds.

**Header**: `#include <jsav/lexer/Token.hpp>`

```cpp
enum class TokenType : uint8_t {
    // Keywords
    Keyword,
    
    // Identifiers
    IdentifierAscii,
    IdentifierUnicode,
    
    // Literals
    IntegerLiteral,
    FloatLiteral,
    StringLiteral,
    CharacterLiteral,
    
    // Operators (representative sample)
    Plus, Minus, Star, Slash,
    Equal, NotEqual, Less, Greater,
    Assign,
    
    // Delimiters
    LeftParen, RightParen,
    LeftBrace, RightBrace,
    Comma, Semicolon,
    
    // Comments
    LineComment,
    BlockComment,
    
    // Whitespace
    Whitespace,
    Newline,
    
    // Special
    EndOfFile,
    Error
};
```

**Design Notes**:
- Underlying type is `uint8_t` for compact storage
- Scoped enum (`enum class`) for type safety
- Exhaustive matching enforced by compiler

---

### Enum: LexicalError

**Purpose**: Enumeration of lexical error types.

**Header**: `#include <jsav/lexer/Token.hpp>`

```cpp
enum class LexicalError : uint8_t {
    // UTF-8 encoding errors
    InvalidUtf8StartByte,
    IncompleteUtf8Sequence,
    OverlongUtf8Encoding,
    SurrogateHalf,
    
    // Escape sequence errors
    InvalidEscapeSequence,
    InvalidHexDigit,
    SurrogateInEscape,
    CodePointTooLarge,
    UnexpectedEndInEscape,
    
    // Literal errors
    UnterminatedString,
    UnterminatedCharacter,
    EmptyCharacterLiteral,
    
    // Comment errors
    UnterminatedBlockComment,
    
    // Identifier errors
    InvalidIdentifierStart,
    InvalidIdentifierContinue
};
```

---

### Struct: SourceLocation

**Purpose**: Source position tracking.

**Header**: `#include <jsav/lexer/Token.hpp>`

```cpp
struct SourceLocation {
    std::size_t byteOffset;  // 0-based byte offset
    std::size_t line;        // 1-based line number
    std::size_t column;      // 0-based column
};
```

**Invariants**:
- `byteOffset` is within source bounds
- `line >= 1`
- `column >= 0`

---

## Error Handling

### Error Token Insertion

When the lexer encounters invalid input:

1. **Insert Error Token**: Create `Token{TokenType::Error, errorKind, sourceSlice, location}`
2. **Continue Lexing**: Advance past error (at least 1 byte)
3. **Preserve Stream**: Error token maintains token stream structure

**Example**:
```cpp
// Source: "let x = \uGGGG;"  (invalid hex in escape)
// Tokens:
// [0] IdentifierAscii ("let")
// [1] IdentifierAscii ("x")
// [2] Assign ("=")
// [3] Error (LexicalError::InvalidHexDigit)  ← error token
// [4] Semicolon (";")
// [5] EndOfFile
```

### Error Recovery Strategies

| Error Type | Recovery Strategy |
|------------|-------------------|
| Invalid UTF-8 start byte | Skip 1 byte, resume lexing |
| Incomplete UTF-8 sequence | Insert error token for partial sequence, resume at EOF |
| Overlong UTF-8 encoding | Insert error token, skip entire overlong sequence |
| Surrogate half | Insert error token, skip surrogate bytes |
| Invalid escape | Insert error token, skip to end of literal |
| Unterminated string | Insert error token at EOF, treat rest of file as string content |
| Invalid identifier start | Insert error token, skip 1 byte |

---

## Usage Examples

### Basic Usage

```cpp
#include <jsav/lexer/Lexer.hpp>
#include <jsav/lexer/Token.hpp>
#include <fmt/format.h>

int main() {
    std::string source = R"(
        let α = 42;  // Greek identifier
        let имя = "Привет";  // Cyrillic identifier with string
        let 变量 = '\u0041';  // CJK identifier with escape
    )";
    
    jsav::lexer::Lexer lexer{std::move(source)};
    auto tokens = lexer.tokenize();
    
    for (const auto& token : tokens) {
        if (token.isError()) {
            fmt::print("Error at line {}: {}\n", 
                      token.location().line,
                      token.text());
        } else {
            fmt::print("Token: {} at line {}, column {}\n",
                      token.type(),
                      token.location().line,
                      token.location().column);
        }
    }
}
```

### Error Handling

```cpp
#include <jsav/lexer/Lexer.hpp>
#include <jsav/lexer/Token.hpp>

void processTokens(const std::vector<jsav::lexer::Token>& tokens) {
    for (const auto& token : tokens) {
        if (token.isError()) {
            // Handle error
            auto error = token.error().value();
            switch (error) {
                case jsav::lexer::LexicalError::InvalidUtf8StartByte:
                    // Report invalid UTF-8
                    break;
                case jsav::lexer::LexicalError::InvalidHexDigit:
                    // Report invalid escape
                    break;
                // ... handle other errors
            }
        } else {
            // Process valid token
            processValidToken(token);
        }
    }
}
```

### Random Access

```cpp
#include <jsav/lexer/Lexer.hpp>

class Parser {
private:
    std::vector<jsav::lexer::Token> tokens_;
    std::size_t pos_ = 0;
    
public:
    explicit Parser(std::vector<jsav::lexer::Token> tokens)
        : tokens_{std::move(tokens)} {}
    
    const jsav::lexer::Token& current() const {
        return tokens_[pos_];
    }
    
    const jsav::lexer::Token& peek(std::size_t offset) const {
        return tokens_[pos_ + offset];  // O(1) random access
    }
    
    void advance() {
        ++pos_;
    }
};
```

---

## Performance Characteristics

### Time Complexity

| Operation | Complexity | Notes |
|-----------|------------|-------|
| `tokenize()` | O(n) | n = source bytes |
| `tokens()` | O(1) | Returns reference |
| `locationFromOffset()` | O(n) worst case | May scan from last known position |
| Token access by index | O(1) | Random access via vector |

### Space Complexity

| Component | Space | Notes |
|-----------|-------|-------|
| Source buffer | O(n) | Owned by Lexer |
| Token vector | O(t) | t = number of tokens |
| Per-token overhead | 32-48 bytes | TokenType, string_view, SourceLocation |
| Total | O(n + t) | Linear in input size |

### Performance Targets

- **Throughput**: ≥100k lines/sec for ASCII input
- **UTF-8 Slowdown**: ≤10% slowdown vs ASCII-only input
- **Memory**: ≤2x source size for complete tokenization

---

## Compatibility

### C++ Version

- **Minimum**: C++23
- **Features Used**: `constexpr`, `std::string_view`, `std::optional`, `std::variant`, structured bindings

### Platform Support

- **Windows**: MSVC 2022+ (17.8+)
- **Linux**: GCC 13+, Clang 16+
- **macOS**: Apple Clang 15+, Clang 16+

### Unicode Version

- **XID Properties**: Unicode 15.0+
- **UTF-8 Encoding**: RFC 3629
- **Scripts Supported**: Latin, Greek, Cyrillic, Arabic, Hebrew, Devanagari, Bengali, Thai, CJK, Hangul

---

## Testing Requirements

### Unit Tests

All public methods must have unit tests in `test/tests.cpp`:

- [ ] `Lexer::tokenize()` with ASCII input
- [ ] `Lexer::tokenize()` with Unicode identifiers
- [ ] `Lexer::tokenize()` with escape sequences
- [ ] `Lexer::tokenize()` with invalid UTF-8
- [ ] `Lexer::locationFromOffset()` with various offsets
- [ ] Error token insertion for all `LexicalError` types

### Compile-Time Tests

All `constexpr` functionality must have tests in `test/constexpr_tests.cpp`:

- [ ] `Utf8Decoder::decode()` with known inputs
- [ ] `XidClassifier::isXidStart()` for various scripts
- [ ] `EscapeDecoder::decodeUnicode()` with valid/invalid escapes
- [ ] `Token` construction and accessors

### Integration Tests

- [ ] Full file tokenization with mixed scripts
- [ ] Error recovery across multiple errors
- [ ] Line ending handling (LF, CRLF, CR)
- [ ] Large file performance (≥10k lines)

### Coverage Requirements

- **Branch Coverage**: ≥95%
- **Line Coverage**: ≥95%
- **Function Coverage**: 100%

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | 2026-02-28 | Initial contract definition |

---

## References

- Feature Specification: `/specs/001-utf8-unicode-lexer/spec.md`
- Data Model: `/specs/001-utf8-unicode-lexer/data-model.md`
- Research Report: `/specs/001-utf8-unicode-lexer/research.md`

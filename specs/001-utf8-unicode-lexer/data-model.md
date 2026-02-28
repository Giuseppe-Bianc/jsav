# Data Model: UTF-8 Unicode Lexer

**Feature**: UTF-8 Unicode Lexer Support  
**Branch**: `001-utf8-unicode-lexer`  
**Date**: 2026-02-28  
**Version**: 1.0

---

## Overview

This document defines the core data entities for the UTF-8 Unicode lexer component. The lexer produces a materialized vector of tokens from UTF-8 encoded source input, with support for Unicode identifiers, escape sequences, and comprehensive error reporting.

---

## Entity: TokenType

**Description**: Strongly-typed scoped enumeration representing the kind of lexical unit recognized by the lexer.

**Fields**:
```cpp
enum class TokenType : uint8_t {
    // Keywords
    Keyword,
    
    // Identifiers
    IdentifierAscii,      // [a-zA-Z_][a-zA-Z0-9_]*
    IdentifierUnicode,    // [\p{Letter}\p{Mark}_][\p{Letter}\p{Mark}\p{Number}_]*
    
    // Literals
    IntegerLiteral,
    FloatLiteral,
    StringLiteral,
    CharacterLiteral,
    
    // Operators
    Plus, Minus, Star, Slash, Percent,
    Equal, NotEqual, Less, Greater, LessEqual, GreaterEqual,
    And, Or, Not, Xor,
    Assign, PlusAssign, MinusAssign, StarAssign, SlashAssign,
    
    // Delimiters
    LeftParen, RightParen,
    LeftBrace, RightBrace,
    LeftBracket, RightBracket,
    Comma, Semicolon, Colon, Dot,
    
    // Comments
    LineComment,
    BlockComment,
    
    // Whitespace
    Whitespace,
    Newline,
    
    // Special
    EndOfFile,
    Error               // Lexical error token
};
```

**Validation Rules**:

- All enum values are contiguous for efficient switch statements
- `TokenType::Error` is used for all lexical errors (invalid UTF-8, malformed escapes, etc.)
- `[[nodiscard]]` applied to functions returning `TokenType`

**Relationships**:

- Each `Token` contains exactly one `TokenType` value
- Used in parser switch statements for token dispatch

---

## Entity: LexicalError

**Description**: Enumeration of lexical error types for precise error reporting and recovery.

**Fields**:
```cpp

enum class LexicalError : uint8_t {
    // UTF-8 encoding errors
    InvalidUtf8StartByte,      // Byte 0xC0-0xC1, 0xF5-0xFF
    IncompleteUtf8Sequence,    // Truncated multi-byte sequence at EOF
    OverlongUtf8Encoding,      // Non-shortest UTF-8 form
    SurrogateHalf,             // U+D800-U+DFFF in UTF-8
    
    // Escape sequence errors
    InvalidEscapeSequence,     // Unknown escape (e.g., \z)
    InvalidHexDigit,           // Non-hex in \uXXXX or \UXXXXXXXX
    SurrogateInEscape,         // \uDC00 (surrogate half)
    CodePointTooLarge,         // \UFFFFFFFF (> U+10FFFF)
    UnexpectedEndInEscape,     // \uABC (too few hex digits)
    
    // Literal errors
    UnterminatedString,        // Missing closing "
    UnterminatedCharacter,     // Missing closing '
    EmptyCharacterLiteral,     // ''
    
    // Comment errors
    UnterminatedBlockComment,  // /* without */
    
    // Identifier errors
    InvalidIdentifierStart,    // Digit or invalid character at start
    InvalidIdentifierContinue  // Invalid character in identifier
};
```

**Validation Rules**:

- Each error type maps to a specific recovery strategy
- Error codes are dense for efficient array indexing

**Relationships**:

- Referenced by `Token::error` field for error tokens
- Used in diagnostic message generation

---

## Entity: SourceLocation

**Description**: Source position tracking using byte-based offsets with line/column information.

**Fields**:
```cpp
struct SourceLocation {
    std::size_t byteOffset;   // 0-based offset from start of source
    std::size_t line;         // 0-based or 1-based line number (TBD)
    std::size_t column;       // 0-based column within line
};
```

**Validation Rules**:
- `byteOffset` must be within source buffer bounds
- `line` and `column` are consistent with byte offset
- Line endings (LF, CRLF, CR) increment line, reset column

**Relationships**:
- Each `Token` contains a `SourceLocation` for its start position
- Used in diagnostic reporting ("error at line 42, column 5")

**Design Decisions**:
- **Line numbering**: 1-based (matches editor line numbers)
- **Column numbering**: 0-based (byte offset within line)
- **Byte offset**: Primary positioning; line/column are derived

---

## Entity: Token

**Description**: A lexical unit produced by the lexer, containing type, text, and position information.

**Fields**:
```cpp
class Token {
private:
    TokenType type_;
    std::optional<LexicalError> error_;
    std::string_view text_;      // Non-owning view into source buffer
    SourceLocation location_;
    
public:
    // Constructors
    constexpr Token(TokenType type, std::string_view text, SourceLocation loc) noexcept;
    constexpr Token(TokenType type, LexicalError error, std::string_view text, SourceLocation loc) noexcept;
    
    // Accessors
    [[nodiscard]] constexpr TokenType type() const noexcept { return type_; }
    [[nodiscard]] constexpr std::optional<LexicalError> error() const noexcept { return error_; }
    [[nodiscard]] constexpr std::string_view text() const noexcept { return text_; }
    [[nodiscard]] constexpr SourceLocation location() const noexcept { return location_; }
    
    // Predicates
    [[nodiscard]] constexpr bool isError() const noexcept { return error_.has_value(); }
    [[nodiscard]] constexpr bool isKeyword() const noexcept { return type_ == TokenType::Keyword; }
    [[nodiscard]] constexpr bool isIdentifier() const noexcept { 
        return type_ == TokenType::IdentifierAscii || type_ == TokenType::IdentifierUnicode; 
    }
    [[nodiscard]] constexpr bool isLiteral() const noexcept {
        return type_ == TokenType::IntegerLiteral || type_ == TokenType::FloatLiteral ||
               type_ == TokenType::StringLiteral || type_ == TokenType::CharacterLiteral;
    }
    
    // Comparison
    [[nodiscard]] constexpr bool operator==(const Token& other) const noexcept = default;
};
```

**Validation Rules**:
- `text_` must be a valid substring of the source buffer
- `location_.byteOffset` must point to the start of `text_`
- Error tokens must have `error_.has_value() == true`
- Non-error tokens must have `error_.has_value() == false`

**Relationships**:
- Materialized in `std::vector<Token>` for random access by parser
- `text_` references the lexer's source buffer (non-owning)
- Parser accesses tokens by index: `tokens[index]`

**Lifetime Management**:
- Token vector must not outlive the lexer's source buffer
- Lexer owns source buffer; tokens hold `std::string_view` into it
- Move semantics enabled for efficient token vector transfer

---

## Entity: Utf8Decoder

**Description**: Utility class for UTF-8 validation and decoding with error reporting.

**Fields**:
```cpp
class Utf8Decoder {
public:
    // Decode a single UTF-8 sequence starting at pos
    // Returns: code point on success, error on failure
    [[nodiscard]] static constexpr std::variant<char32_t, LexicalError> 
    decode(std::string_view input, std::size_t& pos) noexcept;
    
    // Validate a complete UTF-8 string
    // Returns: true if all sequences are valid
    [[nodiscard]] static constexpr bool isValid(std::string_view input) noexcept;
    
    // Encode a code point to UTF-8
    // Returns: UTF-8 bytes (1-4 bytes)
    [[nodiscard]] static constexpr std::array<std::byte, 4> 
    encode(char32_t codePoint, std::size_t& outLen) noexcept;
};
```

**Validation Rules**:
- Reject invalid start bytes (0xC0, 0xC1, 0xF5-0xFF)
- Reject incomplete sequences (truncated at EOF)
- Reject overlong encodings (e.g., 0xC0 0x80 for NUL)
- Reject surrogate halves (U+D800-U+DFFF)
- Reject code points > U+10FFFF

**Relationships**:
- Used internally by `Lexer` for UTF-8 validation
- Used by escape sequence decoder for `\u` and `\U`

---

## Entity: XidClassifier

**Description**: Unicode XID_Start/XID_Continue classification for identifier recognition.

**Fields**:
```cpp
class XidClassifier {
public:
    // Check if code point can start an identifier
    [[nodiscard]] static constexpr bool isXidStart(char32_t codePoint) noexcept;
    
    // Check if code point can continue an identifier
    [[nodiscard]] static constexpr bool isXidContinue(char32_t codePoint) noexcept;
    
    // Classify identifier type (ASCII vs Unicode)
    [[nodiscard]] static constexpr TokenType classifyIdentifier(std::string_view text) noexcept;
};
```

**Validation Rules**:
- XID_Start: Letters from 10 scripts + underscore
- XID_Continue: XID_Start + digits + combining marks
- ASCII identifiers: Only [a-zA-Z0-9_]
- Unicode identifiers: Any XID_Start + XID_Continue characters

**Implementation Details**:
- Range tables stored as `constexpr std::array<CodePointRange, N>`
- Binary search for O(log n) lookup
- Tables cover: Latin, Greek, Cyrillic, Arabic, Hebrew, Devanagari, Bengali, Thai, CJK, Hangul

---

## Entity: EscapeDecoder

**Description**: Unicode escape sequence decoder for string and character literals.

**Fields**:
```cpp
class EscapeDecoder {
public:
    // Decode \uXXXX (4 hex digits) or \UXXXXXXXX (8 hex digits)
    [[nodiscard]] static constexpr std::variant<char32_t, LexicalError>
    decodeUnicode(std::string_view input, std::size_t& pos, int hexDigits) noexcept;
    
    // Decode simple escapes: \n, \t, \r, \\, \", \', etc.
    [[nodiscard]] static constexpr std::variant<char32_t, LexicalError>
    decodeSimple(std::string_view input, std::size_t& pos) noexcept;
    
    // Decode complete escape sequence (any type)
    [[nodiscard]] static constexpr std::variant<char32_t, LexicalError>
    decode(std::string_view input, std::size_t& pos) noexcept;
};
```

**Validation Rules**:
- `\u` must be followed by exactly 4 hex digits
- `\U` must be followed by exactly 8 hex digits
- Reject surrogate halves (U+D800-U+DFFF)
- Reject code points > U+10FFFF
- Accept both uppercase and lowercase hex digits (a-f, A-F)

**Relationships**:
- Used by `Lexer` when processing string and character literals
- Returns code points that are re-encoded to UTF-8 for token text

---

## Entity: Lexer

**Description**: Main lexer class that tokenizes UTF-8 source input into a materialized token vector.

**Fields**:
```cpp
class Lexer {
private:
    std::string source_;           // Owning source buffer
    std::string_view sourceView_;  // Non-owning view for tokenization
    std::size_t pos_;              // Current position
    std::size_t line_;             // Current line (1-based)
    std::size_t column_;           // Current column (0-based)
    std::vector<Token> tokens_;    // Materialized tokens
    
public:
    // Constructors
    explicit Lexer(std::string source);
    explicit Lexer(std::string_view source);  // Non-owning
    
    // Tokenize entire source, return materialized vector
    [[nodiscard]] std::vector<Token> tokenize();
    
    // Tokenize and store internally (for streaming)
    void tokenizeInPlace();
    
    // Access tokens (after tokenization)
    [[nodiscard]] const std::vector<Token>& tokens() const noexcept { return tokens_; }
    
    // Get source location from byte offset
    [[nodiscard]] SourceLocation locationFromOffset(std::size_t byteOffset) const noexcept;
    
private:
    // Internal tokenization methods
    void skipWhitespaceAndComments();
    Token nextToken();
    Token lexIdentifier();
    Token lexStringLiteral();
    Token lexCharacterLiteral();
    Token lexNumericLiteral();
    Token lexOperator();
    void advancePastError();
};
```

**Validation Rules**:
- Source buffer lifetime exceeds token lifetime
- All tokens reference valid substrings of source
- Error tokens inserted for all lexical errors
- Token vector allows O(1) random access

**Relationships**:
- Uses `Utf8Decoder` for UTF-8 validation
- Uses `XidClassifier` for identifier recognition
- Uses `EscapeDecoder` for escape sequences
- Produces `std::vector<Token>` for parser consumption

**Performance Characteristics**:
- **Time**: O(n) where n = source bytes
- **Space**: O(n) for source buffer + O(t) for token vector (t = number of tokens)
- **Token access**: O(1) random access by index

---

## State Transitions

### Lexer State Machine

```
[Start] → (skip whitespace/comments) → [TokenStart]
[TokenStart] → (identifier start) → [LexingIdentifier] → (non-identifier) → [TokenComplete]
[TokenStart] → (digit) → [LexingNumeric] → (non-digit) → [TokenComplete]
[TokenStart] → (") → [LexingString] → (") → [TokenComplete]
[TokenStart] → (') → [LexingCharacter] → (') → [TokenComplete]
[TokenStart] → (operator char) → [LexingOperator] → (non-operator) → [TokenComplete]
[TokenStart] → (invalid) → [ErrorRecovery] → (next valid boundary) → [TokenStart]
[TokenComplete] → (EOF) → [End]
[TokenComplete] → (more input) → [TokenStart]
```

### UTF-8 Decoder State Machine

```
[Start] → (byte < 0x80) → [Complete: 1-byte]
[Start] → (0xC0-0xDF) → [Expect1Continuation] → (0x80-0xBF) → [Complete: 2-byte]
[Start] → (0xE0-0xEF) → [Expect2Continuations] → (0x80-0xBF)² → [Complete: 3-byte]
[Start] → (0xF0-0xF4) → [Expect3Continuations] → (0x80-0xBF)³ → [Complete: 4-byte]
[Any state] → (invalid byte) → [Error]
```

---

## Validation Rules Summary

### Token-Level Invariants

1. **Text Validity**: `token.text()` must be a valid substring of the source buffer
2. **Location Consistency**: `token.location().byteOffset` must equal the start of `token.text()`
3. **Error Semantics**: `token.isError()` iff `token.error().has_value()`
4. **Type Completeness**: Every token has a valid `TokenType` (including `TokenType::Error`)

### Lexer-Level Invariants

1. **Source Fidelity**: Concatenation of all token texts (excluding whitespace) equals source minus comments
2. **Position Monotonicity**: Token byte offsets are strictly increasing
3. **Error Recovery**: Lexer never enters infinite loop on invalid input (always advances ≥1 byte)
4. **Completeness**: Lexer produces tokens for entire input (including EOF token)

### UTF-8 Validation Rules

1. **Start Byte Validity**: Start bytes must be 0x00-0x7F, 0xC2-0xDF, 0xE0-0xEF, or 0xF0-0xF4
2. **Continuation Byte Validity**: Continuation bytes must be 0x80-0xBF
3. **Shortest Form**: Encodings must use minimum bytes (no overlong forms)
4. **Surrogate Exclusion**: Code points U+D800-U+DFFF are invalid
5. **Range Exclusion**: Code points > U+10FFFF are invalid

---

## Design Decisions

### Decision 1: Owning vs Non-Owning Source Buffer

**Decision**: Lexer provides both constructors:
- `Lexer(std::string source)` - owning (recommended)
- `Lexer(std::string_view source)` - non-owning (advanced use)

**Rationale**:
- Owning model is safer (lexer controls source lifetime)
- Non-owning model enables zero-copy when source is managed externally
- Default to owning; non-owning requires explicit lifetime management

### Decision 2: Error Token vs Exception

**Decision**: Error token insertion (not exceptions)

**Rationale**:
- Exceptions incompatible with `constexpr`
- Error tokens enable multiple error reports per file
- Parser can pattern-match on `TokenType::Error`
- Consistent with LSP diagnostic model

### Decision 3: Materialized Vector vs Streaming

**Decision**: Materialized `std::vector<Token>` (not streaming iterator)

**Rationale**:
- Parser needs random access (backtracking, lookahead)
- O(1) token access by index
- Enables parallel processing if needed
- Memory overhead is acceptable (tokens are small)

### Decision 4: Unicode Normalization

**Decision**: No normalization (NFC/NFD treated as distinct)

**Rationale**:
- Simplifies implementation (no normalization tables)
- Preserves source fidelity
- Matches C++23 identifier rules (no normalization)
- Implementation can add normalization later if needed

---

## References

- Feature Specification: `/specs/001-utf8-unicode-lexer/spec.md`
- Research Report: `/specs/001-utf8-unicode-lexer/research.md`
- Unicode Standard Annex #31: Identifier and Pattern Syntax
- RFC 3629: UTF-8 encoding

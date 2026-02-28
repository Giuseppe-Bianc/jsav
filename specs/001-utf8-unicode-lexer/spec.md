# Feature Specification: UTF-8 Unicode Lexer Support

**Feature Branch**: `001-utf8-unicode-lexer`
**Created**: 2026-02-28
**Status**: Draft
**Input**: Enhance the lexer component to support the complete UTF-8 encoded character set with full Unicode standard compliance. The lexer correctly handles multi-byte UTF-8 sequences (1 to 4 bytes per character) and decodes all valid Unicode scalar values. Invalid UTF-8 sequences (malformed bytes, overlong encodings, surrogate halves) are reported as lexical errors rather than silently skipped. Identifiers can start with any Unicode XID_Start character (including Greek, Cyrillic, Arabic, Hebrew, Devanagari, CJK characters) and continue with XID_Continue characters (including combining diacritical marks and digit forms from various scripts). String and character literals accept any valid UTF-8 character and support Unicode escape sequences (\uXXXX for 16-bit codepoints, \UXXXXXXXX for 32-bit codepoints). Comments (both line and block style) correctly process multi-byte UTF-8 characters. Source position tracking uses byte-based offsets while preserving the original UTF-8 byte sequence in token text for accurate error reporting and source mapping. Line endings are recognized correctly across LF (Unix), CRLF (Windows), and CR (legacy) conventions.

## User Scenarios & Testing

### User Story 1 - Write Source Code with Unicode Identifiers (Priority: P1)

As a developer, I want to write variable names, function names, and type names using characters from my native language (Greek, Cyrillic, Arabic, Hebrew, Devanagari, CJK, etc.) so that I can create more expressive code in my natural language.

**Why this priority**: This is the core value proposition of Unicode support - enabling developers to use their native scripts in code. Without this, the feature provides minimal value.

**Independent Test**: Can be fully tested by writing source files with non-ASCII identifiers and verifying the lexer correctly tokenizes them as single identifier tokens.

**Acceptance Scenarios**:

1. **Given** a source file containing Greek identifiers (e.g., `αβγ`, `Συνάρτηση`), **When** the lexer processes the file, **Then** each identifier is recognized as a single token with correct start/end positions.
2. **Given** a source file containing Cyrillic identifiers (e.g., `переменная`, `функция`), **When** the lexer processes the file, **Then** each identifier is recognized as a single token.
3. **Given** a source file containing CJK identifiers (e.g., `变量`, `函数`), **When** the lexer processes the file, **Then** each identifier is recognized as a single token.
4. **Given** an identifier starting with a digit or invalid character, **When** the lexer processes the file, **Then** a lexical error is reported.
5. **Given** an identifier containing combining diacritical marks (e.g., `café` with combining acute accent), **When** the lexer processes the file, **Then** the full identifier including combining marks is recognized as a single token.

---

### User Story 2 - Use Unicode Characters in String and Character Literals (Priority: P2)

As a developer, I want to include any Unicode character in string and character literals, either directly or via escape sequences, so that I can work with international text, emojis, and special symbols.

**Why this priority**: Essential for any language that needs to handle international text data. Secondary only to identifier support in importance.

**Independent Test**: Can be fully tested by writing string/character literals with various Unicode content and verifying correct tokenization and value representation.

**Acceptance Scenarios**:

1. **Given** a string literal containing multi-byte UTF-8 characters (e.g., `"Hello 世界"`, `"Привет"`), **When** the lexer processes the file, **Then** the entire string is recognized as a single token preserving all characters.
2. **Given** a string literal with `\uXXXX` escape sequences (e.g., `"\u0041\u0042\u0043"`), **When** the lexer processes the file, **Then** the escapes are recognized and decoded to their Unicode characters.
3. **Given** a string literal with `\UXXXXXXXX` escape sequences for characters beyond BMP (e.g., `"\U0001F600"` for emoji), **When** the lexer processes the file, **Then** the escape is recognized and decoded correctly.
4. **Given** a character literal containing a Unicode character (e.g., `'α'`, `'中'`), **When** the lexer processes the file, **Then** it is recognized as a single character literal token.
5. **Given** a string with mixed direct UTF-8 and escape sequences, **When** the lexer processes the file, **Then** both forms are correctly handled within the same literal.
6. **Given** an invalid escape sequence (e.g., `\uGGGG` with non-hex digits, `\uDC00` surrogate half), **When** the lexer processes the file, **Then** a lexical error is reported.

---

### User Story 3 - Write Comments with Unicode Content (Priority: P3)

As a developer, I want to write comments using my native language characters so that I can document my code naturally.

**Why this priority**: Comments are important for code documentation but are stripped during lexing, making this lower priority than identifiers and literals. Still essential for international development teams.

**Independent Test**: Can be fully tested by writing source files with Unicode content in both line (`//`) and block (`/* */`) comments and verifying correct comment recognition.

**Acceptance Scenarios**:

1. **Given** a line comment containing multi-byte UTF-8 characters (e.g., `// Это комментарий`), **When** the lexer processes the file, **Then** the entire line is recognized as a comment token.
2. **Given** a block comment containing multi-byte UTF-8 characters spanning multiple lines (e.g., `/* Комментарий */`), **When** the lexer processes the file, **Then** the entire block is recognized as a single comment token.
3. **Given** a block comment containing Unicode characters and newlines, **When** the lexer processes the file, **Then** the comment is correctly terminated and line counting is accurate.

---

### User Story 4 - Receive Accurate Error Reports for Invalid UTF-8 (Priority: P4)

As a developer, I want to receive clear error messages with accurate source positions when the lexer encounters invalid UTF-8 sequences, so that I can quickly identify and fix encoding issues in my source files.

**Why this priority**: Error reporting is critical for developer experience but is a supporting feature to the core lexing functionality.

**Independent Test**: Can be fully tested by providing source files with various invalid UTF-8 sequences and verifying error messages include correct byte positions.

**Acceptance Scenarios**:

1. **Given** a source file containing a malformed UTF-8 sequence (e.g., continuation byte without start byte), **When** the lexer processes the file, **Then** a lexical error is reported with the exact byte offset.
2. **Given** a source file containing an overlong UTF-8 encoding (e.g., 2-byte sequence for ASCII character), **When** the lexer processes the file, **Then** a lexical error is reported.
3. **Given** a source file containing an unpaired UTF-16 surrogate half (U+D800 to U+DFFF), **When** the lexer processes the file, **Then** a lexical error is reported.
4. **Given** a source file with invalid UTF-8 followed by valid code, **When** the lexer processes the file, **Then** the error is reported at the correct position and lexing can continue or stop as designed.

---

### User Story 5 - Consistent Line Ending Handling Across Platforms (Priority: P5)

As a developer, I want source files with Unix (LF), Windows (CRLF), or legacy Mac (CR) line endings to be handled correctly, so that I can work on any platform without line ending issues.

**Why this priority**: Cross-platform compatibility is important but is infrastructure that should work transparently. Lower priority than core Unicode functionality.

**Independent Test**: Can be fully tested by providing identical source files with different line ending conventions and verifying consistent tokenization.

**Acceptance Scenarios**:

1. **Given** a source file with Unix LF line endings, **When** the lexer processes the file, **Then** line numbers are counted correctly.
2. **Given** a source file with Windows CRLF line endings, **When** the lexer processes the file, **Then** line numbers are counted correctly and CRLF is treated as a single line ending.
3. **Given** a source file with legacy Mac CR line endings, **When** the lexer processes the file, **Then** line numbers are counted correctly.
4. **Given** a source file with mixed line ending styles, **When** the lexer processes the file, **Then** each line ending is recognized correctly regardless of style.

---

### Edge Cases

- **What happens when** an identifier starts with a combining diacritical mark (which is XID_Continue but not XID_Start)? **The lexer reports a lexical error** since combining marks cannot start identifiers.
- **How does the system handle** a file that is valid ASCII but contains an incomplete multi-byte UTF-8 sequence at the end? **The lexer reports a lexical error** at the byte position where the sequence starts.
- **What happens when** a `\uXXXX` escape sequence contains a surrogate half (U+D800-U+DFFF)? **The lexer reports a lexical error** as surrogate halves are not valid Unicode scalar values.
- **How does the system handle** a `\UXXXXXXXX` escape sequence that exceeds the maximum Unicode code point (U+10FFFF)? **The lexer reports a lexical error** for the invalid code point.
- **What happens when** an identifier contains a zero-width joiner or other invisible Unicode character? **The identifier is accepted** as these are valid XID_Continue characters, preserving the original byte sequence in token text.
- **How does the system handle** a 5-byte or 6-byte UTF-8 sequence (which are invalid per RFC 3629)? **The lexer reports a lexical error** as only 1-4 byte sequences are valid.

## Requirements

### Functional Requirements

- **FR-001**: The lexer MUST correctly decode all valid UTF-8 byte sequences (1 to 4 bytes) into Unicode scalar values (code points U+0000 to U+10FFFF, excluding surrogate halves U+D800-U+DFFF).
- **FR-002**: The lexer MUST recognize **IdentifierAscii** matching the pattern `[a-zA-Z_][a-zA-Z0-9_]*`.
- **FR-003**: The lexer MUST recognize **IdentifierUnicode** matching the pattern `[\p{Letter}\p{Mark}_][\p{Letter}\p{Mark}\p{Number}_]*`.
- **FR-004**: The lexer MUST accept any valid UTF-8 character within string literals delimited by double quotes.
- **FR-005**: The lexer MUST accept any valid UTF-8 character within character literals delimited by single quotes.
- **FR-006**: The lexer MUST decode `\uXXXX` escape sequences where XXXX is exactly 4 hexadecimal digits representing a code point in the Basic Multilingual Plane (BMP).
- **FR-007**: The lexer MUST decode `\UXXXXXXXX` escape sequences where XXXXXXXX is exactly 8 hexadecimal digits representing any valid Unicode code point (U+0000 to U+10FFFF).
- **FR-008**: The lexer MUST recognize line comments starting with `//` and continuing to the next line ending.
- **FR-009**: The lexer MUST recognize block comments delimited by `/*` and `*/` that may span multiple lines.
- **FR-010**: The lexer MUST report a lexical error when encountering malformed UTF-8 sequences (e.g., continuation byte without start byte, incomplete sequence at end of file).
- **FR-011**: The lexer MUST report a lexical error when encountering overlong UTF-8 encodings (e.g., encoding ASCII characters using multi-byte sequences).
- **FR-012**: The lexer MUST report a lexical error when encountering UTF-8 sequences that decode to surrogate halves (U+D800 to U+DFFF).
- **FR-013**: The lexer MUST report a lexical error when encountering `\uXXXX` or `\UXXXXXXXX` escape sequences with invalid hexadecimal digits.
- **FR-014**: The lexer MUST report a lexical error when encountering `\uXXXX` or `\UXXXXXXXX` escape sequences that decode to surrogate halves or exceed U+10FFFF.
- **FR-015**: The lexer MUST track source positions using byte-based offsets from the start of the source.
- **FR-016**: The lexer MUST preserve the original UTF-8 byte sequence in token text for accurate error reporting and source mapping.
- **FR-017**: The lexer MUST recognize LF (`\n`, 0x0A) as a line ending.
- **FR-018**: The lexer MUST recognize CRLF (`\r\n`, 0x0D 0x0A) as a single line ending.
- **FR-019**: The lexer MUST recognize CR (`\r`, 0x0D) as a line ending.
- **FR-020**: The lexer MUST correctly increment line numbers when encountering any recognized line ending style.
- **FR-021**: The lexer MUST handle identifiers containing combining diacritical marks as part of the identifier (e.g., `café` with combining acute accent is a single identifier).
- **FR-022**: The lexer MUST insert a special error token at the location of a lexical error and continue processing subsequent input, preserving the token stream structure for downstream parser-level error recovery.
- **FR-023**: The lexer MUST materialize all tokens into a random-access container (e.g., `std::vector<Token>`) allowing the parser to access tokens by index with O(1) complexity.

### Key Entities

- **Unicode Scalar Value**: A valid Unicode code point from U+0000 to U+10FFFF, excluding surrogate halves (U+D800 to U+DFFF).
- **UTF-8 Byte Sequence**: A sequence of 1 to 4 bytes that encodes a Unicode scalar value according to UTF-8 encoding rules.
- **XID_Start**: The Unicode property defining characters that can start an identifier (letters, certain symbols, etc.).
- **XID_Continue**: The Unicode property defining characters that can continue an identifier (XID_Start characters plus digits, combining marks, etc.).
- **IdentifierAscii**: An identifier matching the pattern `[a-zA-Z_][a-zA-Z0-9_]*` — ASCII letters and underscore for start, ASCII alphanumeric and underscore for continuation.
- **IdentifierUnicode**: An identifier matching the pattern `[\p{Letter}\p{Mark}_][\p{Letter}\p{Mark}\p{Number}_]*` — any Unicode letter, mark, or underscore for start; letter, mark, number, or underscore for continuation.
- **Token**: A lexical unit produced by the lexer, containing a strongly-typed token type (scoped `enum class TokenType`), the original UTF-8 byte sequence, and source position information (byte offset, line number, column).
- **Error Token**: A special token inserted when a lexical error is encountered, containing error type, byte position, and allowing the lexer to continue producing a valid token stream.
- **Lexical Error**: An error reported when the lexer encounters invalid input (malformed UTF-8, invalid escape sequences, etc.), including error type and byte position.

## Success Criteria

### Measurable Outcomes

- **SC-001**: The lexer correctly tokenizes **IdentifierAscii** (`[a-zA-Z_][a-zA-Z0-9_]*`) with 100% accuracy.
- **SC-002**: The lexer correctly tokenizes **IdentifierUnicode** (`[\p{Letter}\p{Mark}_][\p{Letter}\p{Mark}\p{Number}_]*`) from at least 10 different Unicode scripts (Latin, Greek, Cyrillic, Arabic, Hebrew, Devanagari, Bengali, Thai, CJK, Hangul) with 100% accuracy.
- **SC-003**: The lexer correctly decodes all valid `\uXXXX` escape sequences (65,536 possible values excluding surrogate halves) with 100% accuracy.
- **SC-004**: The lexer correctly decodes all valid `\UXXXXXXXX` escape sequences (1,114,112 possible Unicode scalar values) with 100% accuracy.
- **SC-005**: The lexer reports a lexical error for 100% of invalid UTF-8 sequences in the Unicode 15.0 Conformance Test Suite.
- **SC-006**: The lexer reports a lexical error for 100% of overlong UTF-8 encodings in test cases (all possible overlong forms for ASCII, BMP, and supplementary characters).
- **SC-007**: The lexer reports a lexical error for 100% of surrogate half inputs (both direct UTF-8 encoding and via escape sequences).
- **SC-008**: Source position tracking reports byte offsets within ±0 bytes of actual position for all tokens in test files.
- **SC-009**: Line number counting is accurate (±0 lines) for source files with LF, CRLF, and CR line endings.
- **SC-010**: The lexer processes UTF-8 source files with a slowdown ratio ≤1.10 compared to ASCII-only lexing (UTF-8 file time / ASCII file time for equivalent byte length) on identical hardware.
- **SC-011**: 100% of test cases pass for mixed-script identifiers (identifiers combining characters from multiple Unicode scripts).

## Clarifications

### Session 2026-02-28

- Q: When the lexer encounters a lexical error (invalid UTF-8, malformed escape, etc.), what error recovery behavior should it use to continue processing the rest of the file? → A: Error token insertion - Insert special "error token" at error location and continue. Preserves token stream structure for parser-level error recovery.
- Q: What data structure should the Token entity use to represent the token type (e.g., Identifier, StringLiteral, ErrorToken, etc.)? → A: Enum class (strongly typed) - scoped enum providing type safety and compiler exhaustiveness checking.
- Q: What performance baseline should be used to measure the "10% of ASCII-only lexing performance" success criterion (SC-009)? → A: Relative slowdown ratio - UTF-8 file lexing time / ASCII file lexing time for same byte length.
- Q: How should the lexer expose tokens to the downstream parser component? → A: Random-access token stream - Lexer materializes all tokens in a vector; parser accesses by index.
- Q: Should identifiers with different Unicode normalization forms (e.g., `café` as precomposed U+00E9 vs. decomposed `e` + combining U+0301) be treated as the same identifier or different identifiers? → A: Different identifiers (no normalization) - Treat precomposed and decomposed forms as distinct names.

## Assumptions

- The lexer follows Unicode 15.0 or later for XID_Start and XID_Continue character properties.
- The lexer follows RFC 3629 UTF-8 encoding rules (only 1-4 byte sequences, excluding surrogates).
- String and character literals use standard escape sequence syntax (`\uXXXX` and `\UXXXXXXXX`).
- The lexer continues processing after reporting lexical errors (error recovery) rather than halting immediately.
- Source position tracking uses 0-based or 1-based byte offsets consistently (implementation detail).
- The lexer does not perform Unicode normalization on identifiers; identifiers differing only by normalization form (e.g., precomposed `é` U+00E9 vs. `e` + combining acute U+0301) are treated as distinct names.

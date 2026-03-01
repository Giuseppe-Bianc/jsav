# Feature Specification: Lexer Full UTF-8 Support

**Feature Branch**: `001-lexer-utf8-support`  
**Created**: 2026-03-01  
**Status**: Draft  
**Input**: User description: "Update the lexical analysis component (Lexer.hpp and Lexer.cpp) to achieve full compatibility with the complete set of characters representable through UTF-8 encoding. The system must correctly decode and validate multi-byte sequences (one to four bytes), properly handle malformed sequences, recognize characters from all Unicode planes including extended characters, diacritical marks, combining characters, and emoji. The lexer must classify symbols according to Unicode standard properties for identifier recognition, maintain ASCII compatibility, and ensure efficient text processing performance."

## Clarifications

### Session 2026-03-01

- Q: Is Unicode normalization (NFC/NFD) in scope for the lexer? → A: Explicitly out of scope. Identifiers are compared by exact code point sequence; visually identical identifiers with different code point sequences are treated as distinct.
- Q: How should the lexer handle UTF-8 inside string/char literals? → A: The lexer validates UTF-8 correctness inside string/char literals (detects malformed sequences) but does NOT classify code points for Unicode identifier properties within literals.
- Q: How should the lexer handle valid Unicode characters that are not identifiers, operators, whitespace, or literals? → A: The lexer emits an error token with a specific diagnostic message (e.g., "unexpected Unicode character U+1F600") and advances past the code point.
- Q: Must UTF-8 decoding functions and identifier classification lookup tables be constexpr-compatible? → A: Yes. Both the UTF-8 decoding/validation functions and the Unicode General Category classification functions (including lookup tables) must be constexpr-compatible, consistent with the project's C++23 constexpr style.
- Q: How should the lexer handle malformed UTF-8 inside string/char literals — error token, separate diagnostic, or split tokens? → A: The entire literal becomes an error token (the literal is considered invalid).
- Q: Should Unicode identifier classification use XID_Start/XID_Continue (UAX #31) or Unicode General Category properties? → A: Use Unicode General Categories: `\p{Letter}` (General Category L) for identifier start, `[\p{Letter}\p{Mark}\p{Number}]` (General Categories L, M, N) for identifier continue. Mark (\p{M}) is NOT allowed in start position. This replaces all XID_Start/XID_Continue references. Regex patterns: `IdentifierAscii` = `[a-zA-Z_][a-zA-Z0-9_]*`, `IdentifierUnicode` = `\p{Letter}[\p{Letter}\p{Mark}\p{Number}]*`.
- Q: How should the lexer handle an identifier starting with underscore `_` that contains non-ASCII Unicode characters (e.g., `_变量`)? → A: The lexer produces a single `IdentifierUnicode` token. Tokenization starts using ASCII rules (underscore is a valid ASCII start); when a non-ASCII `\p{Letter}`, `\p{Mark}`, or `\p{Number}` character is encountered in continue position, the token is promoted to `IdentifierUnicode`. The effective start rule for `IdentifierUnicode` is `[\p{Letter}_]`, and continue rule is `[\p{Letter}\p{Mark}\p{Number}a-zA-Z0-9_]`.

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Correct Multi-Byte UTF-8 Decoding (Priority: P1)

As a developer writing source code that contains characters from multiple languages and scripts, I need the lexer to correctly decode all valid UTF-8 sequences (1 to 4 bytes) into their corresponding Unicode code points so that my multilingual source files are parsed accurately.

**Why this priority**: This is the foundational capability upon which all other UTF-8 features depend. Without correct decoding of multi-byte sequences, no higher-level feature (identifier recognition, error handling, etc.) can function correctly.

**Independent Test**: Can be fully tested by feeding the lexer source strings containing known characters from all four UTF-8 byte-length categories and verifying each code point is decoded correctly. Delivers the core ability to read any valid UTF-8 input.

**Acceptance Scenarios**:

1. **Given** source text containing only ASCII characters (U+0000–U+007F), **When** the lexer processes the text, **Then** each character is decoded as a single-byte sequence and produces the correct code point.
2. **Given** source text containing 2-byte UTF-8 sequences (e.g., `é` U+00E9, `ñ` U+00F1, `Ω` U+03A9), **When** the lexer processes the text, **Then** each 2-byte sequence is decoded to the correct code point.
3. **Given** source text containing 3-byte UTF-8 sequences (e.g., `変` U+5909, `₹` U+20B9, `€` U+20AC), **When** the lexer processes the text, **Then** each 3-byte sequence is decoded to the correct code point.
4. **Given** source text containing 4-byte UTF-8 sequences (e.g., `𐍈` U+10348, `😀` U+1F600, `𝕜` U+1D55C), **When** the lexer processes the text, **Then** each 4-byte sequence is decoded to the correct code point.
5. **Given** source text interleaving ASCII and multi-byte characters, **When** the lexer processes the text, **Then** all characters are decoded correctly without corruption or misalignment.

---

### User Story 2 - Malformed UTF-8 Sequence Handling (Priority: P1)

As a developer whose source file may contain corrupted or improperly encoded data, I need the lexer to detect and gracefully handle malformed UTF-8 sequences so that a single encoding error does not crash the parser or silently corrupt subsequent tokens.

**Why this priority**: Robustness against malformed input is equally critical as correct decoding. Without it, real-world files with encoding issues cause undefined behavior or cascading parse failures.

**Independent Test**: Can be fully tested by feeding the lexer deliberately malformed byte sequences and verifying the lexer emits appropriate error tokens and continues processing subsequent valid content.

**Acceptance Scenarios**:

1. **Given** source text containing an orphaned continuation byte (0x80–0xBF appearing without a leading byte), **When** the lexer encounters it, **Then** it emits an error token for the malformed byte and resumes decoding at the next byte.
2. **Given** source text containing a truncated multi-byte sequence (e.g., a 3-byte leader followed by only 1 continuation byte), **When** the lexer encounters it, **Then** it emits an error token covering the incomplete sequence and resumes at the next byte following the sequence.
3. **Given** source text containing an overlong encoding (e.g., U+002F encoded as 0xC0 0xAF), **When** the lexer encounters it, **Then** it rejects the sequence and emits an error token.
4. **Given** source text containing a surrogate half code point (U+D800–U+DFFF), **When** the lexer encounters it, **Then** it rejects the sequence and emits an error token.
5. **Given** source text containing a code point above U+10FFFF, **When** the lexer encounters it, **Then** it rejects the sequence and emits an error token.
6. **Given** source text where a malformed sequence appears mid-file followed by valid tokens, **When** the lexer processes the file, **Then** all tokens after the malformed sequence are still correctly parsed.

---

### User Story 3 - Unicode Identifier Recognition (Priority: P2)

As a developer writing identifiers using non-Latin scripts (e.g., Chinese, Arabic, Cyrillic, Devanagari, or mathematical symbols), I need the lexer to classify characters according to Unicode General Category properties (`\p{Letter}` for start, `\p{Letter}\p{Mark}\p{Number}` for continue) so that my identifiers are correctly recognized across all Unicode scripts.

**Why this priority**: Identifier recognition is the primary consumer of Unicode classification in the lexer. Correct classification enables internationalized source code. It depends on Story 1 (decoding) being complete.

**Independent Test**: Can be fully tested by providing identifiers composed of characters from various scripts and verifying the lexer produces the correct identifier tokens.

**Acceptance Scenarios**:

1. **Given** an identifier composed of CJK characters (e.g., `变量名`), **When** the lexer tokenizes it, **Then** it produces a single Unicode identifier token spanning all characters.
2. **Given** an identifier starting with a Cyrillic letter followed by combining diacritical marks (e.g., `и̃мя`), **When** the lexer tokenizes it, **Then** it produces a single identifier token including the combining marks.
3. **Given** an identifier using Devanagari characters (e.g., `गणना`), **When** the lexer tokenizes it, **Then** it produces a single Unicode identifier token.
4. **Given** an identifier starting with a digit, a combining mark (`\p{Mark}`), or a number character (`\p{Number}`) — i.e., a character that is not `\p{Letter}` — **When** the lexer tokenizes it, **Then** it does not recognize it as the start of an identifier.
5. **Given** an identifier containing characters from the Supplementary Multilingual Plane (e.g., mathematical italic `𝑥` U+1D465), **When** the lexer tokenizes it, **Then** it produces a correct identifier token.
6. **Given** an emoji character (e.g., `😀` U+1F600) appearing where an identifier is expected, **When** the lexer tokenizes it, **Then** it does not classify the emoji as a valid identifier start, since emoji do not belong to Unicode General Category L (`\p{Letter}`).
7. **Given** an identifier starting with underscore followed by non-ASCII Unicode letters (e.g., `_变量`), **When** the lexer tokenizes it, **Then** it produces a single `IdentifierUnicode` token containing the full identifier `_变量`.

---

### User Story 4 - ASCII Compatibility Preservation (Priority: P2)

As a developer working with predominantly ASCII source files, I need the lexer's UTF-8 support to maintain full backward compatibility with existing ASCII processing so that all existing ASCII-only source files continue to tokenize identically.

**Why this priority**: The vast majority of existing source files are ASCII. Any regression in ASCII handling would be a breaking change with broad impact.

**Independent Test**: Can be fully tested by running the existing test suite and verifying all results remain unchanged after the UTF-8 changes.

**Acceptance Scenarios**:

1. **Given** a source file consisting entirely of ASCII characters, **When** the lexer processes it, **Then** the output tokens are identical to the output produced before the UTF-8 changes.
2. **Given** ASCII operator sequences and punctuation, **When** the lexer processes them, **Then** they are recognized as the same token kinds as before.
3. **Given** ASCII string literals and character literals, **When** the lexer processes them, **Then** they produce the same token content as before.

---

### User Story 5 - Efficient Text Processing (Priority: P3)

As a user processing large source files, I need the lexer's UTF-8 handling to maintain efficient performance so that tokenization time does not degrade noticeably compared to ASCII-only processing.

**Why this priority**: Performance is important but secondary to correctness and robustness. Optimization can be applied iteratively once the correct behavior is established.

**Independent Test**: Can be tested by measuring tokenization time on large files (pure ASCII and mixed Unicode) and comparing against baseline measurements.

**Acceptance Scenarios**:

1. **Given** a large ASCII-only source file, **When** the lexer processes it after the UTF-8 changes, **Then** tokenization time remains within 10% of the pre-change baseline.
2. **Given** a large source file with mixed ASCII and multi-byte characters, **When** the lexer processes it, **Then** tokenization completes without disproportionate slowdown relative to file size.
3. **Given** the Unicode General Category lookup tables used for identifier classification, **When** a character is looked up, **Then** the lookup completes in sub-linear time (e.g., binary search or equivalent).

---

### Edge Cases

- What happens when input contains the null byte (U+0000) encoded in UTF-8? The lexer must treat it as a valid code point but not as string terminator when processing `string_view`-based input.
- What happens when a UTF-8 BOM (0xEF 0xBB 0xBF) appears at the start of the file? The lexer should skip it transparently without emitting a token.
- What happens when a multi-byte sequence is split exactly at a buffer boundary? The lexer must correctly handle sequences that span from one read position to another without splitting them.
- What happens when the input consists entirely of continuation bytes (e.g., `0x80 0x80 0x80`)? Each byte should produce a separate error token.
- What happens when a valid character is followed by an unexpected continuation byte? The continuation byte should be treated as a separate malformed sequence.
- What happens when combining characters appear without a base character at the start of an identifier? The combining characters alone should not form a valid identifier start.
- What happens when a valid Unicode character that is not an identifier, operator, whitespace, or literal delimiter appears in source code (e.g., `©`, `®`, emoji outside strings)? The lexer emits an error token with a diagnostic identifying the unexpected code point.

## Requirements *(mandatory)*

### Functional Requirements

#### UTF-8 Decoding

- **FR-001**: The lexer MUST correctly decode all valid UTF-8 sequences of 1 byte (U+0000–U+007F), producing the corresponding code point.
- **FR-002**: The lexer MUST correctly decode all valid UTF-8 sequences of 2 bytes (U+0080–U+07FF), producing the corresponding code point.
- **FR-003**: The lexer MUST correctly decode all valid UTF-8 sequences of 3 bytes (U+0800–U+FFFF, excluding surrogates U+D800–U+DFFF), producing the corresponding code point.
- **FR-004**: The lexer MUST correctly decode all valid UTF-8 sequences of 4 bytes (U+10000–U+10FFFF), producing the corresponding code point.
- **FR-005**: The lexer MUST verify that each continuation byte in a multi-byte sequence matches the pattern `10xxxxxx` (0x80–0xBF).

#### Malformed Sequence Handling

- **FR-006**: The lexer MUST reject overlong encodings (code points encoded in more bytes than the minimum required) and emit an error token.
- **FR-007**: The lexer MUST reject UTF-8 sequences that decode to surrogate half code points (U+D800–U+DFFF) and emit an error token.
- **FR-008**: The lexer MUST reject UTF-8 sequences that decode to values above U+10FFFF and emit an error token.
- **FR-009**: The lexer MUST detect orphaned continuation bytes (0x80–0xBF appearing without a preceding leading byte) and emit an error token for each.
- **FR-010**: The lexer MUST detect truncated multi-byte sequences (leading byte not followed by the expected number of continuation bytes) and emit an error token.
- **FR-011**: After encountering any malformed sequence, the lexer MUST resynchronize and continue processing subsequent bytes correctly without cascading errors.

#### Unicode Character Classification

- **FR-012**: The lexer MUST classify characters as identifier start characters based on the Unicode General Category `\p{Letter}` (category L — all subcategories Lu, Ll, Lt, Lm, Lo) **or** the ASCII underscore `_` (U+005F). Characters with General Category M (Mark) or N (Number) MUST NOT be accepted as identifier start. The effective start pattern for `IdentifierUnicode` is `[\p{Letter}_]`. For `IdentifierAscii`, the start pattern is `[a-zA-Z_]`.
- **FR-013**: The lexer MUST classify characters as identifier continuation characters based on Unicode General Categories `\p{Letter}` (L), `\p{Mark}` (M — subcategories Mn, Mc, Me), and `\p{Number}` (N — subcategories Nd, Nl, No), **plus** the ASCII underscore `_` (U+005F) and ASCII digits `0-9`. The effective continuation pattern for `IdentifierUnicode` is `[\p{Letter}\p{Mark}\p{Number}a-zA-Z0-9_]`. For `IdentifierAscii`, the continuation pattern is `[a-zA-Z0-9_]`.
- **FR-014**: The Unicode General Category classification MUST cover all Unicode scripts and planes, including but not limited to: Latin, Greek, Cyrillic, Armenian, Hebrew, Arabic, Devanagari, Bengali, Tamil, Telugu, Kannada, Malayalam, Thai, Lao, Tibetan, Myanmar, Georgian, Hangul, Hiragana, Katakana, CJK Unified Ideographs (including Extensions A through G), Ethiopic, Cherokee, and Mathematical Alphanumeric Symbols.
- **FR-015**: The lexer MUST correctly handle combining diacritical marks (Unicode General Category M — subcategories Mn, Mc, Me) as identifier continuation characters when they follow a valid identifier start character (`\p{Letter}`). Combining marks MUST NOT be accepted as identifier start characters.
- **FR-016**: The lexer MUST NOT classify emoji characters (Unicode Emoji property) as valid identifier start or continuation characters, unless they also belong to Unicode General Category L (Letter), M (Mark), or N (Number).

#### ASCII Compatibility

- **FR-017**: The lexer MUST produce identical tokens for all pure-ASCII input as it did before the UTF-8 changes (backward compatibility).
- **FR-018**: The lexer MUST continue to distinguish between `IdentifierAscii` and `IdentifierUnicode` token kinds based on whether the identifier contains any non-ASCII characters. An identifier that starts with ASCII characters (including `_`) but later contains non-ASCII characters matching `\p{Letter}`, `\p{Mark}`, or `\p{Number}` MUST be classified as `IdentifierUnicode`. The full regex patterns are: `IdentifierAscii` = `[a-zA-Z_][a-zA-Z0-9_]*` ; `IdentifierUnicode` = `[\p{Letter}_][\p{Letter}\p{Mark}\p{Number}a-zA-Z0-9_]*` (must contain at least one non-ASCII character).

#### Constexpr Compatibility

- **FR-023**: The UTF-8 decoding functions, UTF-8 validation functions, and Unicode General Category identifier classification functions (including lookup tables) MUST be `constexpr`-compatible (usable in constant expressions at compile time), consistent with the project's C++23 constexpr style and existing `relaxed_constexpr_tests` suite.

#### Special Cases

- **FR-019**: The lexer MUST skip a UTF-8 BOM (byte sequence 0xEF 0xBB 0xBF) at the start of input without emitting a token.
- **FR-020**: The lexer MUST correctly handle the null code point (U+0000) within `string_view`-based input without treating it as a string terminator.
- **FR-021**: The lexer MUST validate UTF-8 correctness within string literal and character literal content. When a malformed UTF-8 sequence is encountered inside a literal, the entire literal token MUST be emitted as an error token (the literal is considered invalid). The lexer MUST NOT apply Unicode identifier classification to code points within literals.
- **FR-022**: When the lexer encounters a validly-encoded Unicode character outside of a literal that does not match any recognized lexical category (not an identifier start, not an operator, not whitespace, not a literal delimiter), the lexer MUST emit an error token with a diagnostic message identifying the unexpected code point (e.g., "unexpected Unicode character U+1F600") and advance past the code point.

#### Out of Scope

- **OOS-001**: Unicode normalization (NFC, NFD, NFKC, NFKD) is NOT performed by the lexer. Identifiers are compared by their exact sequence of Unicode code points. Two identifiers that are visually identical but composed of different code point sequences (e.g., U+00E9 vs. U+0065+U+0301) are treated as distinct identifiers.
- **OOS-002**: Unicode identifier classification (General Category-based) is NOT applied to code points within string or character literals. Literal content is validated only for UTF-8 encoding correctness.

### Key Entities

- **Code Point**: A Unicode scalar value (U+0000 to U+D7FF and U+E000 to U+10FFFF). The fundamental unit that the decoder produces from UTF-8 byte sequences.
- **UTF-8 Sequence**: A sequence of 1 to 4 bytes encoding a single code point. Characterized by a leading byte indicating sequence length and 0 to 3 continuation bytes.
- **Identifier Start (`\p{Letter}` or `_`)**: A character belonging to Unicode General Category L (Letter — subcategories Lu, Ll, Lt, Lm, Lo) or the ASCII underscore `_` (U+005F) that may begin an identifier. Derived from the Unicode Character Database UnicodeData.txt General_Category field. The underscore is accepted as a start character for both `IdentifierAscii` and `IdentifierUnicode` tokens.
- **Identifier Continue (`\p{Letter}\p{Mark}\p{Number}` + `_` + `0-9`)**: A character belonging to Unicode General Categories L (Letter), M (Mark — subcategories Mn, Mc, Me), or N (Number — subcategories Nd, Nl, No), plus ASCII underscore `_` and ASCII digits `0-9`, that may appear in the body of an identifier. The token kind (`IdentifierAscii` vs. `IdentifierUnicode`) is determined by whether the identifier contains at least one non-ASCII character.
- **Malformed Sequence**: Any byte sequence that does not conform to valid UTF-8 encoding rules—includes overlong encodings, surrogate halves, out-of-range values, truncated sequences, and orphaned continuation bytes.

## Assumptions

- The Unicode version targeted for General Category classification tables will be Unicode 15.1 or later (the latest stable release at implementation time). The specific version will be documented in the generated table source.
- Unicode General Category classification data will be generated from the official Unicode Character Database (UnicodeData.txt General_Category field) rather than maintained by hand, to ensure completeness and accuracy.
- Column tracking in source locations will remain byte-based (not code-point-based or grapheme-cluster-based), consistent with the current approach. This is an industry-standard convention for compiler source locations.
- Error recovery for malformed sequences follows the "maximal subpart" strategy recommended by the Unicode Standard (Chapter 3, Section 3.9), where each maximal subpart of an ill-formed sequence generates one error.
- The U+FFFD REPLACEMENT CHARACTER will be used internally to represent malformed sequences in error token content, following Unicode best practices.
- All UTF-8 decoding, validation, and Unicode General Category classification functions will be implemented as `constexpr` functions with `constexpr`-compatible data structures (e.g., `constexpr std::array` for lookup tables), enabling compile-time evaluation and compatibility with the project's `relaxed_constexpr_tests` suite.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: The lexer correctly decodes and tokenizes source files containing characters from at least 30 distinct Unicode scripts without errors.
- **SC-002**: 100% of known malformed UTF-8 patterns (overlong, surrogate, out-of-range, truncated, orphaned) produce an error token and do not cause processing to halt or corrupt subsequent tokens.
- **SC-003**: All existing tests for ASCII-only source files pass without modification after the UTF-8 changes.
- **SC-004**: Tokenization time for ASCII-only files remains within 10% of the pre-change baseline.
- **SC-005**: Unicode General Category identifier classification covers all characters designated `\p{Letter}`, `\p{Mark}`, and `\p{Number}` in the targeted Unicode version, with 100% conformance verified against the official Unicode Character Database (UnicodeData.txt).
- **SC-006**: Characters from all 17 Unicode planes (Basic Multilingual Plane through Supplementary Private Use Area-B) are handled without decoding errors when validly encoded.
- **SC-007**: The lexer processes a 1 MB mixed-content source file (ASCII + multilingual + emoji) and completes tokenization within a reasonable time proportional to file size.

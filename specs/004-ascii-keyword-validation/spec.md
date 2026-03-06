# Feature Specification: ASCII-Only Keyword Validation in Lexer

**Feature Branch**: `004-ascii-keyword-validation`
**Created**: 2026-03-05
**Status**: Draft
**Input**: Lexer keyword recognition must use ASCII-only character validation

## User Scenarios & Testing

### User Story 1 - ASCII Keywords Recognized Correctly (Priority: P1)

As a developer writing code in the language, I expect that standard keywords like `if`, `for`, `class`, `while`, `return`, etc. are recognized correctly when typed with standard ASCII characters, so that my code compiles and behaves as expected.

**Why this priority**: This is the core functionality that must continue working exactly as before. Without this, the lexer fails its primary purpose of recognizing language keywords.

**Independent Test**: Compile source code containing standard ASCII keywords and verify they are tokenized as keyword tokens, not identifiers.

**Acceptance Scenarios**:

1. **Given** source code containing `if`, `for`, `class` keywords with standard ASCII characters, **When** the lexer tokenizes the source, **Then** each keyword is recognized and returned as its corresponding keyword token type
2. **Given** source code containing keywords with mixed case (e.g., `If`, `FOR`), **When** the lexer tokenizes the source, **Then** case-sensitivity rules are preserved (keywords are recognized only in their defined case)
3. **Given** source code containing keywords as substrings of longer identifiers (e.g., `iffy`, `format`, `classify`), **When** the lexer tokenizes the source, **Then** these are recognized as identifiers, not keywords

---

### User Story 2 - Non-ASCII Lookalikes Treated as Identifiers (Priority: P2)

As a developer, I expect that character sequences that visually resemble keywords but contain non-ASCII characters (e.g., `fôr` with circumflex, `clàss` with grave accent, `іf` with Cyrillic и) are NOT recognized as keywords, but are instead treated as regular identifiers, so that the lexer maintains strict ASCII-only keyword matching.

**Why this priority**: This is the new security/correctness feature. It prevents potential confusion or security issues from homoglyph attacks where non-ASCII characters visually mimic keywords.

**Independent Test**: Tokenize source code containing keyword lookalikes with non-ASCII characters and verify they are returned as identifier tokens, not keyword tokens.

**Acceptance Scenarios**:

1. **Given** source code containing `fôr` (with non-ASCII circumflex 'ô'), **When** the lexer tokenizes the source, **Then** the sequence is recognized as an identifier token, not the `for` keyword
2. **Given** source code containing `clàss` (with non-ASCII grave accent 'à'), **When** the lexer tokenizes the source, **Then** the sequence is recognized as an identifier token, not the `class` keyword
3. **Given** source code containing `іf` (with Cyrillic 'і' U+0456 instead of ASCII 'i'), **When** the lexer tokenizes the source, **Then** the sequence is recognized as an identifier token, not the `if` keyword
4. **Given** source code containing non-ASCII sequences that don't resemble keywords (e.g., `αβγ`, `日本語`), **When** the lexer tokenizes the source, **Then** these are recognized as identifier tokens (Unicode identifier support unchanged)

---

### User Story 3 - Unicode Identifiers Continue to Work (Priority: P3)

As a developer using international characters in variable names, I expect that identifiers containing Unicode characters (e.g., `variável`, `名前`, `αβγ`) continue to be recognized as valid identifiers, so that the lexer supports internationalized code.

**Why this priority**: This ensures the ASCII validation change doesn't break existing Unicode identifier support. The lexer must continue to accept Unicode in identifiers while restricting keywords to ASCII only.

**Independent Test**: Tokenize source code containing identifiers with various Unicode characters and verify they are recognized as valid identifier tokens.

**Acceptance Scenarios**:

1. **Given** source code containing identifiers with Latin extended characters (e.g., `variável`, `naïve`), **When** the lexer tokenizes the source, **Then** these are recognized as valid identifier tokens
2. **Given** source code containing identifiers with non-Latin scripts (e.g., `名前`, `αβγ`, `переменная`), **When** the lexer tokenizes the source, **Then** these are recognized as valid identifier tokens
3. **Given** source code containing mixed ASCII and Unicode identifiers (e.g., `my_変数`, `value_α`), **When** the lexer tokenizes the source, **Then** these are recognized as valid identifier tokens

---

### Edge Cases

- **Empty sequence**: What happens when the scanner encounters an empty character sequence? (Should not occur in normal operation; scanner should handle gracefully)
- **Single non-ASCII character**: How does the lexer handle a single non-ASCII character like `α` or `ñ`? (Should be treated as identifier start or continuation per existing identifier rules)
- **Mixed ASCII/non-ASCII in keyword-like sequence**: How does the lexer handle sequences like `iф` (ASCII 'i' + Cyrillic 'ф')? (Should be treated as identifier, not keyword)
- **Whitespace after non-ASCII keyword lookalike**: How does the lexer handle `fôr ` (with trailing space)? (Should tokenize as identifier followed by whitespace)
- **Punctuation after non-ASCII keyword lookalike**: How does the lexer handle `fôr;` or `fôr(`? (Should tokenize as identifier followed by operator/punctuation)
- **Keywords with ASCII control characters**: Sequences containing ASCII control characters (U+0000-U+001F) MUST fail the ASCII validation check and be treated as identifiers, not keywords. For example, `if\u0000` or `for\u0001` are tokenized as identifiers.

## Clarifications

### Session 2026-03-05

- Q: Should ASCII control characters (U+0000-U+001F) be considered valid ASCII for keyword matching, or should they cause the sequence to be treated as an identifier? → A: Printable ASCII: range U+0020–U+007E and underscore (U+005F). Sequences containing characters outside this set are treated as identifiers.
- Q: Selected validation range option → A: Option C — use U+0021–U+007E (printable ASCII excluding space U+0020); underscore (U+005F) explicitly allowed.

## Requirements

### Functional Requirements

- **FR-001**: The lexer MUST continue to read and return identifiers exactly as before, including identifiers containing Unicode or extended characters
- **FR-002**: The lexer MUST collect character sequences during identifier/keyword scanning using the existing scanning logic without modification
- **FR-003**: Before comparing a collected character sequence against the keyword table, the lexer MUST verify that every character in the sequence belongs to the valid ASCII set (Unicode code points U+0020 through U+007F, printable ASCII only, excluding control characters U+0000-U+001F)
- **FR-003**: Before comparing a collected character sequence against the keyword table, the lexer MUST verify that every character in the sequence belongs to the validated ASCII set: Unicode code points U+0021 through U+007E (printable ASCII excluding space U+0020). The underscore character (U+005F) is explicitly allowed. Sequences containing characters outside this set MUST be treated as identifiers and NOT matched against the keyword table.
- **FR-003B**: (Obsoleted by FR-003) Removed: prior alternatives referencing U+0020–U+007E or U+0020–U+007F are superseded by FR-003.
- **FR-004**: The lexer MUST perform keyword table comparison only if the ASCII validation check passes (all characters are ASCII)
- **FR-005**: If the ASCII validation check fails (any character is non-ASCII), the lexer MUST treat the sequence as an identifier and NOT attempt keyword matching
- **FR-006**: The lexer MUST preserve existing case-sensitivity rules for keyword recognition
- **FR-007**: The lexer MUST preserve all existing identifier validity rules (what characters can start an identifier, what characters can continue an identifier)
- **FR-008**: The ASCII validation check MUST operate in O(n) time complexity where n is the length of the character sequence, without expensive overhead
- **FR-009**: The function signature, return values, error handling, and scanning behavior outside the keyword/identifier determination logic MUST remain unchanged
- **FR-010**: Existing test cases for ASCII keywords (e.g., `if`, `for`, `class`, `while`, `return`) MUST continue to pass
- **FR-011**: New test cases MUST verify that visually similar sequences with non-ASCII characters (e.g., `fôr`, `clàss`, `іf`) are NOT recognized as keywords but are recognized as identifiers

### Key Entities

- **Character Sequence**: The contiguous sequence of characters read by the scanner that potentially forms an identifier or keyword
- **Valid ASCII Set**: Characters with Unicode code points in the range U+0020 through U+007F (printable ASCII only, excluding control characters U+0000-U+001F)
- **Valid ASCII SetB**: Characters with Unicode code points in the range U+0020 through U+007E (printable ASCII); underscore (U+005F) is explicitly allowed. Sequences containing characters outside this set are treated as identifiers.
- **Valid ASCII Set**: Characters with Unicode code points in the range U+0021 through U+007E (printable ASCII excluding space U+0020). The underscore character (U+005F) is explicitly allowed. Sequences containing characters outside this set are treated as identifiers.
- **Valid ASCII SetB**: (Deprecated) Previous definitions using U+0020 ranges are superseded by the above `Valid ASCII Set` definition.
- **Keyword Table**: The language's predefined set of reserved words that have special meaning (e.g., `if`, `for`, `class`, `while`, `return`)
- **Identifier Token**: A token representing a user-defined name (variable, function, class, etc.)
- **Keyword Token**: A token representing a language reserved word with special syntactic meaning

## Success Criteria

### Measurable Outcomes

- **SC-001**: All existing test cases for ASCII keyword recognition pass without modification (100% backward compatibility for valid ASCII keywords)
- **SC-002**: New test cases for non-ASCII keyword lookalikes pass: sequences like `fôr`, `clàss`, `іf` are tokenized as identifiers, not keywords (100% detection rate)
- **SC-003**: Unicode identifier test cases pass: identifiers containing non-ASCII characters (e.g., `variável`, `名前`, `αβγ`) continue to be recognized as valid identifiers (100% Unicode identifier compatibility)
- **SC-004**: Performance benchmark: lexer throughput for identifier/keyword tokenization does not decrease by more than 5% compared to baseline (pre-modification) performance
- **SC-005**: Code coverage: the ASCII validation logic achieves 100% branch coverage in unit tests (all code paths tested, including both ASCII-only and non-ASCII sequences)
- **SC-006**: Zero regressions: no existing test cases fail as a result of the modification (all pre-existing tests continue to pass)

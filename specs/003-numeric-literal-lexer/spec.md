# Feature Specification: Riconoscimento Completo dei Literal Numerici nel Lexer

**Feature Branch**: `003-numeric-literal-lexer`  
**Created**: 2026-03-03  
**Status**: Draft  
**Input**: User description: "Update the lexical analysis component (lexer) of the system for the recognition of numeric literals using the complete pattern `(\d+\.?\d*|\.\d+)([eE][+-]?\d+)?([uUfFdD]|[iIuU](?:8|16|32))?`, composed of three ordered and distinct groups: group G1 (mandatory) defines the numeric part; group G2 (optional) defines the exponent in scientific notation; group G3 (optional) defines the type suffix. The three groups must appear exclusively in the order G1 → G2 → G3, with no spaces or intervening characters between one group and the next. Group G1 has two mutually exclusive alternatives evaluated in the order shown: (A) `\d+\.?\d*`, which recognizes numbers with an explicit integer part composed of one or more decimal digits, optionally followed by a decimal point and zero or more fractional digits — the forms `42`, `3.`, `3.14`, and `0.5` are therefore valid, including the trailing-point form without subsequent digits such as `3.`, which must neither be rejected nor truncated; (B) `\.\d+`, which recognizes numbers with a fractional part only, composed of a mandatory decimal point followed by one or more digits — the forms `.5`, `.14`, `.0` are valid. Alternative B is activated only when the first character is `.` immediately followed by at least one digit; an isolated `.` or one followed by a non-digit character does not constitute a numeric literal and must not be consumed by the lexer as such. Group G2, when present, is composed of: a character `[eE]` (uppercase or lowercase, interchangeably), an optional sign `[+-]` (assumed positive if absent), and one or more mandatory decimal digits `\d+`. Group G2 must be recognized only if the sequence `[eE][+-]?\d+` is immediately contiguous with the end of G1. If no digits appear after `[eE]` (with or without an intervening sign), the character `e`/`E` must not be included in the numeric token and must be returned to the stream as a separate token; likewise, if `[eE]` is present but the sign `[+-]` is not followed by digits, neither the sign nor the exponent marker must be consumed. Group G3, when present, must be recognized according to the longest-match rule (greedy/maximal munch): compound suffixes `[iIuU](?:8|16|32)` take priority over single-character suffixes `[uU]` when the subsequent character forms a valid width. The permitted single-character suffixes are: `u`/`U` for unsigned integer, `f`/`F` for 32-bit float, `d`/`D` for 64-bit double. The permitted compound suffixes are: `i8`, `i16`, `i32`, `I8`, `I16`, `I32` for signed integers of 8, 16, and 32 bits respectively; `u8`, `u16`, `u32`, `U8`, `U16`, `U32` for unsigned integers of 8, 16, and 32 bits respectively. Valid widths for compound suffixes are exclusively 8, 16, and 32: sequences such as `i64`, `u64`, `i128`, `u128` do not constitute valid suffixes, and the prefix letter must not be consumed into the token. The character `i` or `I` alone (without a valid width) is not a valid suffix. The character `f` or `F` never forms a compound suffix with digits (e.g., `f32` must be read as suffix `f` followed by a separate token `32`). The lexer must recognize the width by attempting `32` first, then `16`, then `8`, to prevent `16` from being read as `1` followed by `6`. The lexer must apply the maximal munch rule: given the same starting position, the longest possible numeric token must be produced. The numeric token ends at the first character that cannot be included in the pattern at the current position, including: whitespace, operators, delimiters, end of file, and any alphabetic character that does not form a valid suffix at the current position. The characters `+` and `-` are part of the numeric token exclusively when they appear immediately after `[eE]` within group G2; in all other contexts, including before the literal, they are separate tokens (e.g., in `-42` the `-` is a unary operator and the numeric token is `42`). The generated token must be of type `TokenKind::Numeric` and the `text` field must contain the entire sequence of consumed characters exactly as they appear in the source, without any normalization: leading zeros must not be removed (`007` remains `007`), the trailing point must not be dropped (`3.` remains `3.`), the leading point must not be expanded (`.5` remains `.5`), the case of suffixes must not be altered (`1.0F` remains `1.0F`), and the numeric value must not be computed or interpreted. The token must also carry source position information: start index, end index (inclusive), line number (1-based), and column number (1-based) of the first character. Recognition must occur in a single pass over the input stream (single-pass, O(n) with respect to the length of the literal) without non-linear backtracking and without the use of regex libraries at runtime. A numeric literal cannot span multiple lines. Non-ASCII characters are always treated as non-digits and terminate any numeric literal in progress. Previously supported cases must continue to work without regressions: simple integers (`0`, `1`, `42`, `007`), decimals with an integer part (`1.0`, `3.14`, `0.5`), decimals with a trailing point (`1.`, `42.`), decimals with a fractional part only (`.5`, `.14`). The following edge cases must produce the tokens indicated: `1e` → token `1` + separate token `e`; `1e+` → token `1` + separate tokens `e`, `+`; `1u64` → `Numeric` token `1u64` (maximal munch: `u` + digits are lexically consumed even though `u64` is not a semantically valid suffix); `1i` → token `1` + separate token `i`; `42 u8` → token `42` + separate token `u8` (the space prevents the suffix from attaching); `42u` → token `42` + separate token `u` (`u` alone is not a valid suffix); `.` → is not a numeric token; `-42` → token `-` + token `42`."

## User Scenarios & Testing *(mandatory)*

### User Story 1 — Recognizing basic integers and decimals (Priority: P1)

The lexer must correctly recognize numeric literals in the following fundamental forms: simple integers (e.g., `0`, `1`, `42`, `007`), decimal numbers with both an integer and a fractional part (e.g., `1.0`, `3.14`, `0.5`), decimal numbers with a trailing dot only (e.g., `1.`, `42.`), and numbers with a fractional part only, preceded by a dot (e.g., `.5`, `.14`, `.0`). The token text must be preserved exactly as it appears in the source, without normalization.

**Why this priority**: This is the foundational functionality (group G1) upon which all other scenarios are built. Without correct recognition of basic numeric forms, scientific notation and suffixes cannot be added. This also includes non-regression coverage of already supported cases.

**Independent Test**: It can be tested by supplying the lexer with strings containing each basic numeric form and verifying that the produced token contains the exact text and correct position coordinates.

**Acceptance Scenarios**:

1. **Given** the input `42`, **When** the lexer parses the stream, **Then** produces a Numeric token with text `42`
2. **Given** the input `3.14`, **When** the lexer parses the stream, **Then** produces a Numeric token with text `3.14`
3. **Given** the input `3.`, **When** the lexer parses the stream, **Then** produces a Numeric token with text `3.` (ending point preserved)
4. **Given** the input `.5`, **When** the lexer parses the stream, **Then** produces a Numeric token with text `.5` (start point preserved)
5. **Given** the input `007`, **When** the lexer parses the stream, **Then** produces a Numeric token with text `007` (leading zeros preserved)
6. **Given** the input `.`, **When** the lexer parses the stream, **Then** the period is NOT recognized as a numeric token
7. **Given** the input `.abc`, **When** the lexer parses the stream, **Then** the period is NOT recognized as a numeric token (no digits after the period)

---

### User Story 2 — Scientific Notation Recognition (Priority: P2)

The lexer must recognize the exponent group (G2) immediately following the numeric portion (G1). The group consists of an `e`/`E` marker, an optional `+`/`-` sign, and one or more mandatory digits. If the digits are absent after the marker, the marker and any sign must not be consumed in the numeric token.

**Why this priority**: Scientific notation is the second level of complexity, dependent on the correct functioning of G1, and is widely used for float literals in scientific and engineering contexts.

**Independent Test**: It can be tested by providing strings with valid and invalid scientific notation and verifying that the tokens produced are correct.

**Acceptance Scenarios**:

1. **Given** the input `1e10`, **When** the lexer parses, **Then** produces a single Numeric token with text `1e10`
2. **Given** the input `3.14E+2`, **When** the lexer parses, **Then** produces a single Numeric token with text `3.14E+2`
3. **Given** the input `2.5e-3`, **When** the lexer parses, **Then** produces a single Numeric token with text `2.5e-3`
4. **Given** the input `.5E10`, **When** the lexer parses, **Then** produces a single Numeric token with text `.5E10`
5. **Given** the input `1e`, **When** the lexer parses, **Then** produces Numeric token `1` followed by a separate token `e` (no digit after (the exponent)
6. **Given** the input `1e+`, **When** the lexer parses, **Then** produces the Numeric token `1` followed by separate tokens `e` and `+` (sign without digits)
7. **Given** the input `1E-`, **When** the lexer parses, **Then** produces the Numeric token `1` followed by separate tokens `E` and `-`

---

### User Story 3 — Type Suffix Recognition (Priority: P3)

The lexer must recognize type suffixes (G3) immediately following G1 or G2. Allowed single-character suffixes are `u`/`U` (unsigned), `f`/`F` (float 32-bit), and `d`/`D` (double 64-bit). Allowed compound suffixes are `i8`/`i16`/`i32` and `u8`/`u16`/`u32` (case insensitive). The maximal munch rule dictates that compound suffixes have priority. The character `i`/`I` alone is not a valid suffix. `f`/`F` never forms compound suffixes with digits.

**Why this priority**: Type suffixes complete the numeric pattern and allow literals to be typed, but they are less common than pure scientific notation.

**Independent Test**: Can be tested by providing digits followed by all allowed suffixes and verifying correct tokenization.

**Acceptance Scenarios**:

1. **Given** the input `42u`, **When** the lexer parses, **Then** produces `Numeric("42")` + token `u` (`u` alone is not a valid suffix)
2. **Given** the input `42U`, **When** the lexer parses, **Then** produces `Numeric("42")` + token `U` (`U` alone is not a valid suffix)
3. **Given** the input `1.0F`, **When** the lexer parses, **Then** produces Numeric token with text `1.0F` (`F` is a valid single suffix)
4. **Given** the input `1.0f`, **When** the lexer parses, **Then** produces Numeric token with text `1.0f` (`f` is a valid single suffix)
5. **Given** the input `10d`, **When** the lexer parses, **Then** produces Numeric token with text `10d` (`d` is a valid single suffix)
6. **Given** the input `10D`, **When** the lexer parses, **Then** produces Numeric token with text `10D` (`D` is a valid single suffix)
7. **Given** the input `255u8`, **When** the lexer parses, **Then** produces Numeric token with text `255u8` (valid compound suffix)
8. **Given** the input `1000i32`, **When** the lexer parses, **Then** produces Numeric token with text `1000i32` (valid compound suffix)
9. **Given** the input `50i16`, **When** the lexer parses, **Then** produces Numeric token with text `50i16` (valid compound suffix)
10. **Given** the input `50I16`, **When** the lexer parses, **Then** produces Numeric token with text `50I16` (valid compound suffix)
11. **Given** the input `1i`, **When** the lexer parses, **Then** produces token `1` + token `i` (`i` alone is not a valid suffix)
12. **Given** the input `1I`, **When** the lexer parses, **Then** produces token `1` + token `I` (`I` alone is not a valid suffix)
13. **Given** the input `1u64`, **When** the lexer parses, **Then** produces Numeric token with text `1u64` (maximal munch: `u` + digits consumes everything)
14. **Given** the input `1U64`, **When** the lexer parses, **Then** produces Numeric token with text `1U64` (maximal munch: `U` + digits consumes everything)
15. **Given** the input `1i64`, **When** the lexer parses, **Then** produces Numeric token with text `1i64` (maximal munch: `i` + digits consumes everything)
16. **Given** the input `1I64`, **When** the lexer parses, **Then** produces Numeric token with text `1I64` (maximal munch: `I` + digits consumes everything)
17. **Given** the input `5f32`, **When** the lexer parses, **Then** produces token `5f` + token `32` (`f` never forms compound suffixes with digits)
18. **Given** the input `5F32`, **When** the lexer parses, **Then** produces token `5F` + token `32` (`F` never forms compound suffixes with digits)

---

### User Story 4 — Complete G1-G2-G3 Combined Pattern (Priority: P4)

The lexer must recognize the combination of the three groups in the exclusive order G1 → G2 → G3, contiguous and without separators. Expressions combining scientific notation and a type suffix must produce a single numeric token.

**Why this priority**: This scenario covers the complete combination of the pattern and is the least frequent in real inputs, but necessary for overall correctness.

**Independent Test**: It can be tested with strings that combine all three groups and checking the resulting token.

**Acceptance Scenarios**:

1. **Given** the input `1.5e10f`, **When** the lexer parses, **Then** produces a single Numeric token with the text `1.5e10f`
2. **Given** the input `2.0E-3d`, **When** the lexer parses, **Then** produces a single Numeric token with the text `2.0E-3d`
3. **Given** the input `1e2u16`, **When** the lexer parses, **Then** produces a single Numeric token with the text `1e2u16`
4. **Given** the input `.5e1i32`, **When** the lexer parses, **Then** produces a single Numeric token with the text `.5e1i32`

---

### User Story 5 — Maximal Munch Rule and Token Boundaries (Priority: P5)

The lexer must apply the maximal munch rule: it must produce the longest possible numeric token. The characters `+` and `-` are part of the token only within the G2 group. Spaces, operators, delimiters, end-of-file characters, and non-ASCII characters terminate the literal.

**Why this priority**: Correct token boundaries are essential to avoid over- or under-consumption of characters, but it relies on the correct implementation of the previous groups.

**Independent Test**: Can be tested with inputs containing numbers adjacent to other tokens, verifying correct separation.

**Acceptance Scenarios**:

1. **Given** the input `-42`, **When** the lexer parses, **Then** produces the token `-` (operator) followed by the token Numeric `42`
2. **Given** the input `42 u8`, **When** the lexer parses, **Then** produces the token Numeric `42` followed by a separate token `u8` (the space interrupts the suffix start)
3. **Given** the input `3.14+2`, **When** the lexer parses, **Then** produces the token Numeric `3.14` followed by the token `+` and the token Numeric `2`
4. **Given** the input `1e2+3`, **When** the lexer parses, **Then** produces the token Numeric `1e2` followed by the token `+` and the token Numeric `3`

---

### Edge Cases

- A single period (`.`) must not be recognized as a numeric token.
- A period followed by a non-digit character (`.abc`) is not a numeric token.
- `1e` produces token `1` + token `e` (incomplete exponent)
- `1e+` produces token `1` + token `e` + token `+` (exponent without digits after the sign)
- `1E-` produces token `1` + token `E` + token `-`
- `1u` produces token `1` + token `u` (`u` alone is not a valid suffix)
- `1U` produces token `1` + token `U` (`U` alone is not a valid suffix)
- `1u64` produces Numeric token `1u64` (maximal munch: `u` followed by digits consumes all)
- `1U64` produces Numeric token `1U64` (maximal munch: `U` followed by digits consumes all)
- `1i` produces token `1` + token `i` (`i` alone is not a valid suffix)
- `1I` produces token `1` + token `I` (`I` alone is not a valid suffix)
- `1i64` produces Numeric token `1i64` (maximal munch: `i` followed by digits consumes all)
- `1I64` produces Numeric token `1I64` (maximal munch: `I` followed by digits consumes all)
- `42 u8` produces token `42` + token `u8` (the space prevents the suffix from attaching to the number).
- `-42` produces tokens `-` + token `42` (`-` is not part of the numeric literal)
- `5f32` produces tokens `5f` + token `32` (`f` never forms compound suffixes with digits)
- `1d` produces Numeric token `1d` (`d` is a valid single suffix)
- `1D` produces Numeric token `1D` (`D` is a valid single suffix)
- `1f` produces Numeric token `1f` (`f` is a valid single suffix)
- `1F` produces Numeric token `1F` (`F` is a valid single suffix)
- Non-ASCII characters immediately terminate the numeric token.
- The numeric literal cannot span multiple lines.
- The priority for recognizing compound suffix widths is: `32`, then `16`, then `8`.
- `1e2i32` produces a single token `1e2i32` (G1 + G2 + G3 combined).

## Requirements *(mandatory)*

### Functional Requirements

#### Group G1 — Numeric Part (required)

- **FR-001**: The system MUST recognize integers composed of one or more decimal digits (e.g., `0`, `1`, `42`, `007`)
- **FR-002**: The system MUST recognize decimal numbers with both a whole number and a fractional part (e.g., `1.0`, `3.14`, `0.5`)
- **FR-003**: The system MUST recognize numbers with a final period without fractional digits (e.g., `3.`, `42.`) while preserving the period in the token text.
- **FR-004**: The system MUST recognize numbers with only a fractional part composed of a period followed by one or more digits (e.g., `.5`, `.14`, `.0`)
- **FR-005**: The system MUST NOT recognize a single period (`.`) as a numeric token.
- **FR-006**: The system MUST NOT recognize a period followed by a non-digit character (e.g., `.abc`) as a numeric token.
- **FR-007**: The two G1 alternatives must be evaluated in order: first the form with the integer part (A), then the form with only the period (B).

#### Group G2 — Scientific Notation (Optional)

- **FR-008**: The system MUST recognize an exponent group consisting of `e`/`E`, an optional sign `+`/`-`, and one or more mandatory digits, if immediately adjacent to G1.
- **FR-009**: If no digits (with or without an intervening sign) appear after `e`/`E`, the `e`/`E` marker MUST NOT be included in the numeric token and MUST be returned as a separate token.
- **FR-010**: If `e`/`E` is followed by `+`/`-` but with no subsequent digits, neither the sign nor the marker MUST be consumed in the numeric token.

#### Group G3 — Type Suffix (optional)

> **NOTE: "Valid Suffix" vs "Maximal Munch Consumption"**
> - **Valid suffix**: A suffix that confers semantic type information to the numeric token (e.g., `f` → float32, `u8` → unsigned 8-bit). Valid suffixes are **included** in the numeric token.
> - **Maximal munch consumption**: A lexical rule where the lexer consumes characters to produce the longest possible token, even if the consumed sequence is not a valid suffix. This prevents ambiguous tokenization (e.g., `1u64` → single token `1u64`, not `1` + `u64` or `1u` + `64`).
> - **Key distinction**: `u`/`U` alone are **NOT valid suffixes** (they don't confer type info), but `u`/`U` followed by digits **ARE consumed** by maximal munch (producing a single token, even though the suffix is semantically invalid).

- **FR-011**: The system MUST recognize **valid** single-character suffixes: `f`/`F` (float 32-bit), `d`/`D` (double 64-bit). The characters `u`/`U` **alone (without following digits)** are **NOT** valid suffixes and MUST NOT be consumed as part of the numeric token (e.g., `42u` → `Numeric("42")` + `u`, `42U` → `Numeric("42")` + `U`).
- **FR-011b**: The system MUST apply the maximal munch rule for `u`/`U` and `i`/`I` followed by digits: if the letter is followed by one or more digits, the lexer MUST consume the entire sequence (letter + digits) as part of the numeric token, even if the digits do not constitute a valid width (e.g., `1u64` → `Numeric("1u64")`, `1U64` → `Numeric("1U64")`, `1i64` → `Numeric("1i64")`, `1I64` → `Numeric("1I64")`).
- **Rationale**: This prevents ambiguous tokenization. The token `1u64` is a single numeric token (maximal munch), even though `u64` is not a valid suffix. Semantic validation of the suffix occurs after tokenization.
- **See also**: FR-020 (maximal munch rule definition)
- **FR-012**: The system MUST recognize the compound suffixes: `i8`/`i16`/`i32`, `I8`/`I16`/`I32` (signed integer), `u8`/`u16`/`u32`, `U8`/`U16`/`U32` (unsigned integer)
- **Note**: These are **valid suffixes** that confer semantic type information.
- **FR-013**: The system MUST apply the longest match (maximal munch) rule for suffixes: compound suffixes take priority over single-character suffixes when the next character forms a valid width.
- **FR-014**: Valid widths for compound suffixes are only `8`, `16` and `32`; Sequences such as `i64`, `u64`, `i128`, `u128` are NOT valid suffixes (but are consumed for maximal munch if preceded by `u`/`U` or `i`/`I`).
- **Example**: `1u64` → `Numeric("1u64")` (maximal munch consumes all), but `u64` is not a valid suffix semantically.

> **NOTE: Lexical Consumption vs Semantic Validity**
> - **Lexical consumption (maximal munch)**: The lexer consumes characters to form the longest possible token. This is a **syntactic** rule that prevents ambiguous tokenization.
>   - Example: `1u64` → single token `Numeric("1u64")` because `u` followed by digits triggers maximal munch consumption.
> - **Semantic validity**: A suffix is **semantically valid** if it confers meaningful type information to the numeric token.
>   - Valid suffixes: `f`/`F` (float32), `d`/`D` (float64), `i8`/`i16`/`i32` (signed int), `u8`/`u16`/`u32` (unsigned int)
>   - Invalid suffixes: `u64`, `i64`, `u128`, `i128` (widths not supported), `u`/`U` alone (no width specified)
> - **Key point**: A token can be **lexically consumed** (single token) but **semantically invalid** (triggers compiler warning/error in later phases).
> - **Analogy**: The lexer recognizes `int x = 42unknown;` as valid tokens (`int`, `x`, `=`, `42`, `unknown`, `;`), but `unknown` is not a valid type—semantic analysis catches the error. The lexer's job is tokenization, not semantic validation.
- **FR-015**: The character `i`/`I` alone (without any following digits) is NOT a valid suffix and MUST NOT be consumed in the numeric token.
- **Example**: `1i` → token `1` + token `i` (no digits follow `i`, so maximal munch does not apply).
- **FR-015b**: The character `u`/`U` alone (without any following digits) is NOT a valid suffix and MUST NOT be consumed in the numeric token.
- **Example**: `42u` → `Numeric("42")` + token `u` (no digits follow `u`, so maximal munch does not apply).
- **FR-016**: The character `f`/`F` NEVER forms a composite suffix with digits (e.g., `f32` must be read as the suffix `f` followed by a separate token `32`).
- **FR-017**: Width recognition MUST first attempt `32`, then `16`, then `8`, to avoid partial matches (e.g. `16` read as `1` + `6`)

#### Group Order and Contiguity

- **FR-018**: The three groups MUST appear exclusively in the order G1 → G2 → G3, without spaces or intervening characters.
- **FR-019**: Each group is optional except G1, which is mandatory.

#### Maximal Munch Rule and Token Boundaries

- **FR-020**: The system MUST apply the maximal munch rule: for the same starting position, the longest possible numeric token must be produced.
- **FR-021**: The characters `+` and `-` are part of the numeric token ONLY when they appear immediately after `e`/`E` in group G2; in all other contexts, they are separate tokens.
- **FR-022**: The numeric token ends at the first non-consumable character: whitespace, operators, delimiters, end-of-file, non-ASCII characters, or alphabetic characters that do not form a valid suffix at the current position.

#### Token Format

- **FR-023**: The generated token MUST be of type Numeric
- **FR-024**: The text field MUST contain the entire sequence of consumed characters exactly as they appear in the source, without any normalization (leading zeros preserved, trailing/leading periods preserved, suffix case preserved)
- **FR-025**: The token MUST contain positional information: start index, end index (inclusive), line number (1-based), and column number (1-based) of the first character

#### Performance and Architecture Constraints

- **FR-026**: Recognition MUST occur in a single pass through the stream (single-pass, complexity O(n) with respect to the literal length), without nonlinear backtracking.
- **Definition**: "Nonlinear backtracking" means scanning the same character position more than twice when recognizing a single numeric literal. A position is "scanned" each time `peek_byte()` is called at that position. Save/restore operations for failed exponent or suffix attempts do not count as additional scans if each position is visited at most twice.
- **FR-027**: The system MUST NOT use runtime regex libraries for recognition.
- **FR-028**: The numeric literal MUST end at the first newline character (`\n`, `\r`, or `\r\n`); the newline character MUST NOT be consumed by the `TokenKind::Numeric` token and MUST remain in the stream for the next token, even if the pattern G1→G2→G3 would otherwise be continuable on the next line.

#### Backward Compatibility

- **FR-029**: All previously supported cases MUST continue to work without regressions: plain integers, decimals with integer part, decimals with a dot trailing part, decimals with only a fractional part.

### Key Entities

- **Numeric Token**: Numeric type token produced by the lexer; it contains the literal text of the recognized number, the position coordinates in the source (start index, end index, row, column), and represents the concatenation of groups G1, G2 (if present), and G3 (if present).
- **Group G1 (Numeric Part)**: Mandatory component of the numeric literal, with two mutually exclusive alternatives: (A) integer part with optional period and fractional digits, (B) period followed by fractional digits.
- **Group G2 (Exponent)**: Optional component for scientific notation, consisting of the `e`/`E` marker, optional sign, and mandatory digits.
- **Group G3 (Type Suffix)**: Optional component indicating the numeric type. Can be single-character (`u`, `f`, `d`) or compound with width (`i8`, `u16`, `i32`, etc.)
- **Single Suffix**: One of `u`/`U`, `f`/`F`, `d`/`D`
- **Compound Suffix**: Prefix letter (`i`/`I`/`u`/`U`) followed by a valid width (`8`, `16`, `32`)

## Assumptions

- The lexer operates in a single-threaded context when parsing a single input stream.
- The digit character set is limited to `0-9` (ASCII digits); Unicode digits are not considered.
- The order of precedence of G1 alternatives (first A, then B) is relevant only when the first character is a digit. If the first character is `.`, alternative B is activated directly.
- Type suffixes are case-insensitive in recognition, but the original text is preserved in the token.
- The suffix `d`/`D` is only allowed as a single character (it does not form compounds with digits, similarly to `f`/`F`).
- The characters `+` and `-` preceding a numeric literal are always separate tokens (unary operators), even when the semantic intent is to indicate a negative number.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: 100% of the test cases for simple integers (`0`, `1`, `42`, `007`) produce tokens with exact text and Numeric type.
- **SC-002**: 100% of the test cases for decimal numbers (with integer part, with trailing point, with fractional part only) produce correct tokens without altering the source text.
- **SC-003**: 100% of the test cases for valid scientific notation (`1e10`, `3.14E+2`, `2.5e-3`) produce a single Numeric token containing the entire expression.
- **SC-004**: 100% of the boundary cases for invalid scientific notation (`1e`, `1e+`, `1E-`) produce separate tokens that comply with the non-consumption rules.
- **SC-005**: 100% of the test cases for Valid type suffixes (single and compound) produce tokens with correctly included suffixes.
- **SC-006**: 100% of boundary cases for invalid suffixes (`1i`, `1u64`, `5f32`) produce tokenization that conforms to maximal munch rules.
- **SC-007**: All pre-existing lexer tests continue to pass without regressions.
- **SC-008**: The recognition time for a single numeric literal remains proportional to its length (linear complexity O(n)), verifiable for inputs up to 1000 characters.

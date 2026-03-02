# Feature Specification: Full UTF-8 Unicode Whitespace Support in Lexer

**Feature Branch**: `002-utf8-unicode-whitespace`  
**Created**: 2025-03-02  
**Status**: Draft  
**Input**: User description: "Update the lexer's lexical analysis component (specifically the skip_whitespace_and_comments function in Lexer.cpp) to provide full UTF-8 Unicode whitespace support. The function must recognize and correctly handle all Unicode code points that have the \p{White_Space} property according to the official Unicode standard (currently defined in UnicodeData.hpp), replacing the current limitation to ASCII whitespace characters only. The implementation must properly decode multi-byte UTF-8 sequences, handle invalid UTF-8 sequences gracefully without crashing, and maintain correct line/column tracking when whitespace spans multiple bytes."

## User Scenarios & Testing *(mandatory)*

<!--
  IMPORTANT: User stories should be PRIORITIZED as user journeys ordered by importance.
  Each user story/journey must be INDEPENDENTLY TESTABLE - meaning if you implement just ONE of them,
  you should still have a viable MVP (Minimum Viable Product) that delivers value.
  
  Assign priorities (P1, P2, P3, etc.) to each story, where P1 is the most critical.
  Think of each story as a standalone slice of functionality that can be:
  - Developed independently
  - Tested independently
  - Deployed independently
  - Demonstrated to users independently
-->

### User Story 1 - Complete Unicode Whitespace Recognition (Priority: P1)

A developer writes source code using Unicode whitespace characters (e.g., no-break space U+00A0, em space U+2003, ideographic space U+3000, or vertical tab U+000B) between tokens. The lexer correctly identifies all of these as whitespace separators, consuming them silently and producing the same token stream as if ASCII spaces had been used.

**Why this priority**: This is the core value of the feature — without recognizing all `\p{White_Space}` code points, the lexer will reject or misparse valid source files that contain non-ASCII whitespace. This directly impacts users who edit files on international keyboards, copy-paste from rich text sources, or work with documents that contain non-breaking spaces.

**Independent Test**: Can be fully tested by feeding the lexer strings containing each of the 26 `\p{White_Space}` code points between two valid tokens and verifying that both tokens are produced correctly with no errors.

**Acceptance Scenarios**:

1. **Given** source input containing U+00A0 (NO-BREAK SPACE) between two identifiers, **When** the lexer tokenizes the input, **Then** both identifiers are produced as separate tokens with no error.
2. **Given** source input containing U+3000 (IDEOGRAPHIC SPACE) between a keyword and a literal, **When** the lexer tokenizes the input, **Then** both tokens are produced correctly.
3. **Given** source input containing U+000B (VERTICAL TAB) between tokens, **When** the lexer tokenizes the input, **Then** the vertical tab is consumed as whitespace and both tokens are produced.
4. **Given** source input containing U+000C (FORM FEED) between tokens, **When** the lexer tokenizes the input, **Then** the form feed is consumed as whitespace and both tokens are produced.
5. **Given** source input containing U+0085 (NEXT LINE / NEL) between tokens, **When** the lexer tokenizes the input, **Then** the NEL is consumed as whitespace and treated as a line terminator (incrementing the line counter).

---

### User Story 2 - Correct Line and Column Tracking for Multi-Byte Whitespace (Priority: P2)

A developer uses Unicode line terminators (U+2028 LINE SEPARATOR, U+2029 PARAGRAPH SEPARATOR, U+0085 NEXT LINE) in their source file. When an error occurs on a subsequent line, the error message reports the correct line number and column position so the developer can locate the issue immediately.

**Why this priority**: Accurate error reporting is essential for developer productivity. If multi-byte whitespace causes line/column counters to drift, every subsequent error message in the file will point to the wrong location, making debugging extremely difficult.

**Independent Test**: Can be fully tested by constructing input with known multi-byte whitespace line terminators, followed by a deliberate error token, and verifying the reported line and column numbers match expectations.

**Acceptance Scenarios**:

1. **Given** two lines of code separated by U+2028 (LINE SEPARATOR, 3 bytes in UTF-8), **When** the lexer reaches the second line, **Then** the line counter has incremented by 1 and the column counter has reset to 1.
2. **Given** two lines of code separated by U+2029 (PARAGRAPH SEPARATOR), **When** the lexer reaches the second line, **Then** the line counter has incremented by 1 and the column counter has reset to 1.
3. **Given** two lines of code separated by U+0085 (NEXT LINE, 2 bytes in UTF-8), **When** the lexer reaches the second line, **Then** the line counter has incremented by 1 and the column counter has reset to 1.
4. **Given** a non-breaking space U+00A0 (2 bytes in UTF-8) used mid-line between tokens, **When** the lexer advances past it, **Then** the column counter advances by the number of bytes in the UTF-8 encoding (2), not by 1.

---

### User Story 3 - Graceful Handling of Malformed UTF-8 Sequences (Priority: P2)

A developer opens a file that has been corrupted or saved with a wrong encoding, containing invalid UTF-8 byte sequences in positions where whitespace would normally appear. The lexer does not crash, hang, or enter an infinite loop. Instead, it stops treating the invalid bytes as whitespace and allows the normal token-processing path to handle or report the error.

**Why this priority**: Robustness against malformed input is critical for any parser. A crash or infinite loop on invalid input is a severe defect that could affect editor integrations, CI pipelines, and developer trust.

**Independent Test**: Can be fully tested by feeding the lexer known invalid UTF-8 byte sequences (e.g., lone continuation bytes 0x80–0xBF, overlong encodings, truncated multi-byte sequences) in whitespace positions and verifying no crash occurs and the lexer produces a meaningful error or passes control to the token handler.

**Acceptance Scenarios**:

1. **Given** source input containing a lone continuation byte (0x80) where whitespace is expected, **When** the lexer processes it, **Then** the lexer does not crash and does not enter an infinite loop.
2. **Given** source input containing a truncated 3-byte UTF-8 sequence (first byte 0xE0 followed by end-of-file), **When** the lexer processes it, **Then** the lexer terminates gracefully without crashing.
3. **Given** source input containing an overlong encoding of U+0020 (SPACE), **When** the lexer processes it, **Then** the overlong sequence is NOT recognized as whitespace (per UTF-8 security best practices) and is handled by the normal error path.

---

### User Story 4 - Backward Compatibility with ASCII Whitespace (Priority: P1)

A developer using only standard ASCII whitespace characters (space, tab, carriage return, newline) experiences no change in lexer behavior. All existing source files continue to tokenize identically.

**Why this priority**: Regression in ASCII whitespace handling would break every existing user. This is a non-negotiable constraint alongside the new Unicode support.

**Independent Test**: Can be fully tested by running the complete existing test suite against the updated lexer and verifying all tests pass with identical results.

**Acceptance Scenarios**:

1. **Given** source input using only ASCII spaces, tabs, carriage returns, and newlines, **When** the lexer tokenizes the input, **Then** the token stream, line numbers, and column numbers are identical to the current lexer behavior.
2. **Given** source input with mixed ASCII whitespace and comments (line and block), **When** the lexer tokenizes the input, **Then** comment skipping and whitespace handling remain unchanged.

---

### Edge Cases

- What happens when a Unicode whitespace character appears inside a string literal? It must NOT be consumed as whitespace — it is part of the string content.
- What happens when a Unicode whitespace character appears inside a comment? It is ignored as part of the comment body (already consumed by comment-skipping logic).
- What happens when multiple different Unicode whitespace characters appear consecutively (e.g., U+00A0 followed by U+2003 followed by U+3000)? All must be consumed as a single whitespace run.
- What happens at end-of-file immediately after a multi-byte whitespace character? The lexer must stop cleanly and produce an EOF token without reading past the buffer.
- What happens when an invalid UTF-8 lead byte (0xFE or 0xFF, which are never valid in UTF-8) appears in a whitespace position? The lexer must not crash and must not treat it as whitespace.
- What happens when a valid multi-byte UTF-8 sequence encodes a non-whitespace character (e.g., U+00E9 LATIN SMALL LETTER E WITH ACUTE, 2 bytes)? The lexer must NOT consume it as whitespace; it must be passed to the normal token-handling path.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: The lexer MUST recognize all 26 code points with the Unicode `\p{White_Space}` property as whitespace. The complete list is: U+0009 (HT), U+000A (LF), U+000B (VT), U+000C (FF), U+000D (CR), U+0020 (SPACE), U+0085 (NEL), U+00A0 (NBSP), U+1680 (OGHAM SPACE MARK), U+2000–U+200A (11 code points from EN QUAD to HAIR SPACE), U+2028 (LINE SEPARATOR), U+2029 (PARAGRAPH SEPARATOR), U+202F (NARROW NO-BREAK SPACE), U+205F (MEDIUM MATHEMATICAL SPACE), U+3000 (IDEOGRAPHIC SPACE).
- **FR-002**: The lexer MUST add recognition of the currently missing `\p{White_Space}` code points: U+000B (VT), U+000C (FF), and U+0085 (NEL). VT and FF (single-byte, ≤0x7F) MUST be added to the ASCII fast-path in `skip_whitespace_and_comments`. NEL (2-byte sequence 0xC2 0x85) MUST be handled as a special case in the non-ASCII whitespace path. The existing `whitespace_ranges` table and `is_unicode_whitespace()` function in UnicodeData.hpp MUST remain unchanged — they retain their Zs/Zl/Zp category semantics.
- **FR-003**: The lexer MUST correctly decode multi-byte UTF-8 sequences (2-byte, 3-byte, and 4-byte) when checking for Unicode whitespace code points.
- **FR-004**: The lexer MUST NOT crash, hang, or enter an infinite loop when encountering invalid UTF-8 byte sequences in whitespace positions. Invalid sequences include: lone continuation bytes, truncated multi-byte sequences, overlong encodings, and bytes 0xFE/0xFF.
- **FR-005**: The lexer MUST reject overlong UTF-8 encodings and NOT treat them as the code point they decode to. For example, an overlong encoding of U+0020 must not be recognized as a space.
- **FR-006**: The lexer MUST treat the following code points as line terminators that increment the line counter and reset the column counter: U+000A (LF), U+0085 (NEL), U+2028 (LINE SEPARATOR), U+2029 (PARAGRAPH SEPARATOR). U+000D (CR) is NOT a line terminator — it is consumed as plain whitespace (preserving current behavior; CR+LF sequences produce a single line increment via the LF).
- **FR-007**: The lexer MUST advance the column counter by the number of bytes in the UTF-8 encoding for non-line-terminating whitespace code points (consistent with the existing byte-based column tracking convention).
- **FR-008**: The lexer MUST NOT consume Unicode whitespace characters that appear inside string literals or comments — those are already handled by separate lexing paths.
- **FR-009**: All existing ASCII whitespace behavior (space, tab, CR, LF), comment skipping (line and block comments), and BOM handling MUST remain unchanged.
- **FR-010**: The lexer MUST handle end-of-file occurring mid-way through a multi-byte whitespace sequence gracefully (treating the truncated sequence as not-whitespace, not crashing).

### Key Entities

- **Unicode Whitespace Code Point**: A code point that has the `\p{White_Space}` property in the Unicode standard. Comprises 26 code points across categories Zs, Zl, Zp, and Cc. Each has a specific UTF-8 byte-length (1–3 bytes) and a classification as either a line terminator or non-line-terminator.
- **UTF-8 Sequence**: A variable-length encoding (1–4 bytes) of a Unicode code point. The lexer must decode these sequences to identify the code point and then check it against the whitespace table. Invalid sequences (malformed, overlong, truncated) must be detected and handled safely.
- **Line Terminator**: A whitespace code point that represents a logical line break. When encountered, the line counter increments and the column counter resets. The set of line terminators is: LF, NEL, LINE SEPARATOR, PARAGRAPH SEPARATOR. CR is explicitly excluded — it is plain whitespace consumed without affecting the line counter.

## Clarifications

### Session 2026-03-02

- Q: How should the lexer treat a bare CR (U+000D) — as a line terminator or plain whitespace? FR-006 listed CR as a line terminator but FR-009 requires preserving current behavior where CR does not increment the line counter. → A: CR remains plain whitespace (Option A). Only LF, NEL, LINE SEPARATOR, and PARAGRAPH SEPARATOR are line terminators. FR-006 corrected accordingly.
- Q: Where should the 3 missing \p{White_Space} code points (VT, FF, NEL) be added — in the UnicodeData.hpp table, in the lexer fast-path, or in a new function? → A: VT and FF added to the ASCII fast-path in skip_whitespace_and_comments; NEL handled as a special case in the non-ASCII path. The whitespace_ranges table and is_unicode_whitespace() in UnicodeData.hpp remain unchanged (Option A).

## Assumptions

- The existing byte-based column tracking convention (columns count UTF-8 bytes, not Unicode code points or grapheme clusters) is intentional and will be maintained. This is consistent with how many editors and tools report column positions.
- The CR+LF sequence is already handled by the existing lexer logic (CR consumed as whitespace, then LF triggers line increment) and does not require special-case handling beyond what exists today.
- U+000B (VERTICAL TAB) and U+000C (FORM FEED) are treated as non-line-terminating whitespace. While some systems treat FF as a page break, for lexing purposes it simply separates tokens without incrementing the line counter. This aligns with the behavior of most modern language lexers (JavaScript, Python, Rust).
- The Unicode `\p{White_Space}` property is stable across Unicode versions — the set of 26 code points has not changed since Unicode 4.1 and is unlikely to change.
- The `is_unicode_whitespace()` function in UnicodeData.hpp is a general-purpose utility used beyond the lexer; its semantics (Zs/Zl/Zp categories only) must not change. The lexer's whitespace logic supplements it with VT, FF, and NEL handling locally.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: The lexer correctly tokenizes input containing any of the 26 `\p{White_Space}` code points as token separators, producing identical token sequences to equivalent input using ASCII spaces — verified by automated tests covering all 26 code points.
- **SC-002**: Line and column numbers reported after multi-byte whitespace line terminators (NEL, LINE SEPARATOR, PARAGRAPH SEPARATOR) are accurate to within 0 deviation from expected values — verified by targeted position-tracking tests.
- **SC-003**: The lexer processes files containing malformed UTF-8 sequences without any crashes, hangs, or undefined behavior — verified by fuzz-style tests with at least 10 distinct classes of invalid byte sequences.
- **SC-004**: All existing tests continue to pass with no regressions — verified by running the full existing test suite after changes.
- **SC-005**: Tokenization throughput for ASCII-only input does not degrade by more than 5% compared to the baseline — verified by benchmarking before and after the change on a representative corpus.

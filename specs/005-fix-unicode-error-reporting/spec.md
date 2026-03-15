# Feature Specification: Unicode-Aware Error Reporter

**Feature Branch**: `005-fix-unicode-error-reporting`
**Created**: 2026-03-14
**Status**: Draft
**Input**: Update the ErrorReporter class so that it correctly handles source files containing UTF-8 encoded Unicode characters, ensuring perfect visual alignment between error markers and the actual characters that caused the error.

## Clarifications

### Session 2026-03-14

- Q: What performance constraints should the Unicode error reporting meet? → A: Zero overhead required - Unicode handling must be as fast as current byte-based calculation (microsecond-level)
- Q: What is the maximum line length the ErrorReporter must handle correctly? → A: Practical limit - 10,000 code points per line
- Q: Should error markers use color for enhanced visibility in terminals that support it? → A: Optional color with fallback - use colored carets when terminal supports ANSI colors, fall back to '^' otherwise
- Q: Should the spec terminology be standardized to use "code point" consistently instead of "character"? → A: Use "code point" exclusively - replace all instances of "character" with "Unicode code point" or "code point"
- Q: Should error positioning use grapheme clusters (user-perceived characters) instead of code points for languages with combining characters? → A: Code points only - "é" (e + combining accent) counts as 2 positions, simpler implementation
- Q: How should tab characters be handled for visual marker alignment? → A: Configurable tab width (4, 8, or custom)
- Q: How should ANSI terminal color support be detected? → A: Environment variable detection (`COLORTERM`, `TERM`) - standard cross-platform approach respecting `NO_COLOR`
- Q: What color should error markers use for ANSI terminal output? → A: Red - traditional compiler error color (GCC, Clang, MSVC standard)
- Q: What format should invalid UTF-8 error messages follow? → A: Existing ErrorReporter format - reuse current error message structure
- Q: How does ErrorReporter receive error position information? → A: SourceSpan - token-based positioning with source span metadata
- Q: What Unicode edge cases should be handled (BOM, null bytes, bidirectional text, emoji sequences, normalization forms)? → A: Full coverage - skip BOM at file start, reject null bytes (U+0000), display bidirectional text as-is, treat emoji ZWJ sequences as separate code points, accept all normalization forms
- Q: How should ErrorReporter integrate with the compiler pipeline for line tracking? → A: Integration via LineTracker
- Q: What security measures should be in place for malicious UTF-8 input (overlong encodings, surrogate halves, DoS attempts)? → A: Defensive validation - reject overlong encodings and surrogate halves, limit line length to prevent DoS, report all encoding errors with byte offsets
- Q: What logging/observability should ErrorReporter provide for debugging and operational monitoring? → A: Minimal logging - only log critical errors (invalid UTF-8); no operational metrics
- Q: What explicit exclusions should be documented to prevent scope creep during implementation? → A: Explicit exclusions - no IDE integration, no auto-fix suggestions, no multi-line error highlighting, no syntax-aware error messages
- Q: How should FR-016 reference the ErrorReporter format? → A: Cross-reference FR-001 explicitly - enumerate the five format components (header, location, source line, marker row, optional help)

## User Scenarios & Testing

### User Story 1 - Unicode Source File Error Positioning (Priority: P1)

As a developer writing source code with Unicode characters (such as international identifiers, string literals with emojis, or non-ASCII comments), when the compiler encounters an error in my code, I need the error marker to point to the exact character position visually, so that I can immediately identify the problematic code without manually counting characters.

**Why this priority**: This is the core functionality that delivers immediate value. Without correct Unicode handling, error messages are misleading and waste developer time debugging the wrong positions. This is the minimum viable improvement over the current byte-based system.

**Independent Test**: Can be fully tested by compiling a source file containing multi-byte Unicode characters with a deliberate syntax error and verifying the caret marker aligns visually under the correct character in the displayed source line.

**Acceptance Scenarios**:

1. **Given** a source file containing multi-byte Unicode characters (e.g., "let x = 你好;") with a syntax error at a specific character, **When** the compiler reports the error, **Then** the visual marker row displays the caret (^) directly beneath the correct Unicode character, not offset by byte count.

2. **Given** a source file where an error spans multiple Unicode characters (e.g., an invalid identifier "αβγδ"), **When** the compiler reports the error, **Then** the marker row displays one caret for each Unicode code point in the span, aligned precisely under each corresponding character.

3. **Given** a source file containing only ASCII characters with a syntax error, **When** the compiler reports the error, **Then** the behavior is identical to the current implementation (backward compatibility).

---

### User Story 2 - Invalid UTF-8 Detection and Reporting (Priority: P2)

As a developer who may accidentally open a file with incorrect encoding or encounter corrupted source files, when the ErrorReporter encounters invalid UTF-8 byte sequences, I need a clear error message that identifies the exact byte offset and line number, so that I can locate and fix the encoding issue.

**Why this priority**: Invalid UTF-8 handling is critical for robustness but is secondary to correct handling of valid UTF-8. Without this, the compiler may produce misleading errors or crash on malformed input. However, valid UTF-8 handling (P1) is more frequently encountered.

**Independent Test**: Can be fully tested by providing a source file with intentionally invalid UTF-8 byte sequences and verifying the compiler reports a specific encoding error with byte offset and line number, rather than misinterpreting the bytes or producing misaligned markers.

**Acceptance Scenarios**:

1. **Given** a source file containing an invalid UTF-8 byte sequence at byte offset 42 on line 5, **When** the ErrorReporter processes the file, **Then** it reports a specific encoding error stating "Invalid UTF-8 sequence at byte offset 42, line 5" rather than displaying garbled characters or incorrect column positions. The visual marker row MUST use byte-based column calculation to position caret(s) under the invalid byte(s) for that specific line only.

2. **Given** a source file with valid UTF-8 on most lines but an invalid sequence on one line, **When** an error is reported on a different line with valid UTF-8, **Then** the marker alignment is correct on the valid line using code point-based calculation (no fallback to byte-based calculation).

3. **Given** a source file containing an invalid UTF-8 sequence on line 3 and a syntax error on line 7 (valid UTF-8), **When** the syntax error is reported, **Then** line 7's marker uses code point-based calculation and line 3's encoding error marker uses byte-based calculation for the invalid bytes only.

---

### User Story 3 - Edge Case Handling for Unicode Display (Priority: P3)

As a developer working with source code containing unusual Unicode constructs (combining characters, zero-width characters, or errors at line boundaries), I need the error reporter to handle these edge cases correctly, so that error messages remain accurate and readable regardless of the Unicode content.

**Why this priority**: Edge cases affect a smaller subset of users but are critical for correctness when they occur. This can be delivered after core functionality (P1) and invalid UTF-8 handling (P2) are complete.

**Independent Test**: Can be fully tested by providing source files with specific edge case Unicode constructs and verifying the error marker positioning matches the expected behavior for each case.

**Acceptance Scenarios**:

1. **Given** a source file where an error occurs at the first character of a line (column 1), **When** the error is reported, **Then** the marker row begins with a caret and has no leading spaces.

2. **Given** a source file where an error occurs at the last character of a line, **When** the error is reported, **Then** the marker appears beneath that final character with no trailing content.

3. **Given** a source file containing combining Unicode code points (e.g., "e" followed by combining acute accent), **When** an error involves these code points, **Then** each code point in the sequence counts as one column unit independently for marker positioning (e.g., "é" = 2 column positions).

4. **Given** a source file with tab characters before an error position, **When** the error is reported, **Then** each tab character is expanded to its visual width based on configured tab stops (default: 8 columns) for marker alignment purposes.

5. **Given** an empty line with an error pointing to it, **When** the error is reported, **Then** the marker row displays a single caret at column 1.

---

## Out-of-Scope

The following items are explicitly excluded from this feature:

- **IDE integration**: ErrorReporter does not provide IDE-specific protocols (LSP, DAP) or editor plugins.
- **Auto-fix suggestions**: ErrorReporter does not suggest fixes or corrections for the reported errors.
- **Multi-line error highlighting**: Errors are reported line-by-line; multi-line error spans (e.g., unterminated strings spanning multiple lines) are not highlighted with visual markers.
- **Syntax-aware error messages**: Error descriptions are generated by the parser/lexer, not by ErrorReporter; ErrorReporter focuses solely on visual positioning accuracy.

---

### Edge Cases

- What happens when a source line contains only whitespace (spaces and tabs) and an error points to it? The marker should align with the correct column position, treating spaces as one column unit each and tabs expanded to their visual width based on configured tab stops.

- What happens when a source line contains a mix of single-byte ASCII and multi-byte Unicode code points before the error position? The marker should count all code points as one column unit each, regardless of byte length.

- How does the system handle a file that is entirely valid UTF-8 except for one invalid byte sequence? The invalid sequence MUST be reported as an encoding error with byte offset and line number. For the line containing invalid UTF-8, the marker row MUST use byte-based column calculation to position caret(s) under the invalid byte(s). All other lines with valid UTF-8 MUST use code point-based calculation (no fallback).

- What happens when an error position falls in the middle of a multi-byte UTF-8 sequence (due to byte-based position input)? The ErrorReporter must handle this gracefully by identifying the start of the code point and reporting the position correctly.

- How should the ErrorReporter handle whitespace-only lines (spaces and tabs only)? The marker should align with the correct column position, treating spaces as one column unit each and tabs expanded per FR-018 formula.

- How should the ErrorReporter handle a Byte Order Mark (BOM, U+FEFF) at the beginning of a file? The BOM MUST be skipped when calculating column positions (it does not count as column 1), but the byte offset in error messages MUST include the BOM bytes.

- How should null bytes (U+0000) in source files be handled? Null bytes MUST be rejected as invalid in source files and reported as an encoding error with byte offset and line number.

- How should bidirectional text (Arabic, Hebrew) be displayed in error messages? Bidirectional text MUST be displayed as-is without reordering; marker alignment is calculated by code point position, not visual display order.

- How should emoji sequences with Zero Width Joiner (ZWJ) be handled? Each code point in the ZWJ sequence (including the ZWJ characters themselves) counts as one column unit independently; a family emoji (👨‍👩‍👧‍👦) counts as 7 code points (man + ZWJ + woman + ZWJ + girl + ZWJ + boy).

- How should Unicode normalization forms (NFC, NFD, NFKC, NFKD) be handled? All normalization forms MUST be accepted without conversion; "é" may be represented as a single precomposed code point (U+00E9) or as base + combining accent (e + U+0301), and both are treated according to their actual code point count (1 or 2 respectively).

## Requirements

### Functional Requirements

- **FR-001**: ErrorReporter MUST generate error messages containing three elements: a human-readable error description, the full text of the source line where the error was detected, and a visual marker row placed beneath the source line.

- **FR-002**: The visual marker row MUST consist of leading spaces followed by one or more caret symbols ('^') indicating the span of the problematic token or code point.

- **FR-003**: Column positions for placing carets in the marker row MUST be calculated by counting logical Unicode code points from the beginning of the line, starting at column 1.

- **FR-004**: Each Unicode code point MUST be counted as exactly one column unit for positioning purposes, regardless of whether it is encoded as one, two, three, or four bytes in UTF-8.

- **FR-005**: When an error position is preceded by multi-byte Unicode code points, the number of leading spaces in the marker row MUST equal the number of Unicode code points before the error position, not the number of bytes.

- **FR-006**: When an error spans multiple code points, the marker row MUST display one caret for each code point in the span, aligned precisely under each corresponding code point.

- **FR-007**: The source line displayed in the error message MUST reproduce the original file content byte-for-byte without any transformation, substitution, or truncation.

- **FR-008**: Line boundaries MUST be determined by newline delimiters (line feed, carriage return, or carriage return followed by line feed).

- **FR-009**: Line numbering MUST count lines sequentially starting at 1, unaffected by the presence of multi-byte code points.

- **FR-010**: Tab code points within the source line MUST be expanded to their visual width based on configurable tab stops for marker alignment purposes.

- **FR-011**: If a source line contains combining Unicode code points (such as a base letter followed by a combining accent), each code point in the sequence MUST count as one column unit independently (e.g., "é" as e + combining acute = 2 column positions, not 1).

- **FR-012**: If an error occurs at the first code point of a line, the marker row MUST begin with a caret and no leading spaces.

- **FR-013**: If an error occurs at the last code point of a line, the marker MUST appear beneath that final code point.

- **FR-014**: If a line is empty and an error points to it, the marker row MUST display a single caret at column 1.

- **FR-015**: All behavior on files containing only ASCII code points (byte values 0 through 127) MUST remain identical to the current behavior, ensuring full backward compatibility.

- **FR-016**: When ErrorReporter encounters a byte sequence that does not constitute valid UTF-8, it MUST report a specific encoding error identifying the byte offset and the line number where the invalid sequence was found. The error message MUST follow the existing ErrorReporter format as defined in **FR-001**, consisting of: (1) a header line with `ERROR [Exxxx] LEX: <message>`, (2) a location line, (3) the source line with line number prefix, (4) a visual marker row with caret(s) beneath the problematic byte(s), and optionally (5) a help line. For the invalid byte sequence only, the marker position MUST be calculated using byte-based column calculation: each byte before and within the invalid sequence counts as 1 column unit (regardless of UTF-8 structure), while all valid code points preceding the invalid sequence count as 1 column unit each. The visual marker row formula is: `leading_spaces = count_valid_codepoints_before_invalid_sequence + byte_offset_within_invalid_sequence`, followed by `caret_count = byte_length_of_invalid_sequence`.

- **FR-017**: ErrorReporter MUST never fall back to byte-based column calculation for valid UTF-8 lines. For lines containing invalid UTF-8 sequences, byte-based column calculation MAY be used solely for positioning the marker under the invalid bytes, but all other lines in the same file MUST use code point-based calculation regardless of invalid UTF-8 elsewhere in the file.

- **FR-018**: ErrorReporter MUST accept a configurable tab stop width parameter (default: 8 columns). Tab expansion MUST calculate visual column position as: `visualColumn = ((currentColumn - 1) / tabStopWidth + 1) * tabStopWidth + 1`.

- **FR-019**: If a source file begins with a Byte Order Mark (BOM, U+FEFF), the BOM MUST be skipped when calculating column positions for error marker alignment (the BOM does not count as column 1), but byte offset calculations MUST include the BOM bytes (3 bytes: 0xEF 0xBB 0xBF).

- **FR-020**: If a null byte (U+0000) is encountered in a source file, the ErrorReporter MUST reject the file and report an encoding error stating "Null byte (U+0000) not allowed in source files at byte offset X, line Y".

- **FR-021**: If bidirectional text (such as Arabic or Hebrew) is present in a source line, the text MUST be displayed in the error message as-is without Unicode bidirectional reordering; marker alignment MUST be calculated by code point position from line start, not by visual display order.

- **FR-022**: If an emoji sequence contains Zero Width Joiner (ZWJ, U+200D) characters, each code point in the sequence (including each ZWJ) MUST count as one column unit independently for marker positioning purposes.

- **FR-023**: If a source file contains Unicode in any normalization form (NFC, NFD, NFKC, or NFKD), the ErrorReporter MUST accept it without conversion and calculate column positions based on the actual code points present in the file (not after normalization).

- **FR-024**: ErrorReporter MUST query LineTracker to retrieve the source line content (as a byte-for-byte string) and line number for a given error byte offset before calculating code point-based marker positions.

- **FR-025**: ErrorReporter MUST reject overlong UTF-8 encodings (sequences that use more bytes than necessary for a code point) and report them as encoding errors with byte offset and line number.

- **FR-026**: ErrorReporter MUST reject UTF-16 surrogate halves (U+D800 through U+DFFF, which are invalid in UTF-8) and report them as encoding errors with byte offset and line number.

- **FR-027**: ErrorReporter MUST enforce the line length limit of 10,000 code points (NFR-002) by reporting an error if a line exceeds this limit, preventing denial-of-service via extremely long lines.

- **FR-028**: ErrorReporter MUST log critical encoding errors (invalid UTF-8, null bytes, surrogate halves, overlong encodings) using the project's spdlog infrastructure at error level; no operational metrics or debug-level logging of normal error reporting operations are required.

### Key Entities

- **ErrorReporter**: The component responsible for generating human-readable error messages with visual positioning markers. It receives error position information and source file content, and produces formatted error output.

- **Source Line**: A single line of text from the source file, bounded by newline delimiters (LF, CR, or CRLF), preserved byte-for-byte in error output.

- **Visual Marker Row**: A formatting element consisting of leading spaces and caret symbols ('^') that visually indicates the exact code point position(s) where an error occurs.

- **Unicode Code Point**: The fundamental unit of Unicode text, representing a single character regardless of its UTF-8 byte encoding length (1-4 bytes). **Terminology note**: This spec uses "code point" exclusively; the term "character" in user stories refers to "Unicode code point" unless otherwise noted. **Grapheme cluster note**: Combining character sequences (e.g., "e" + combining acute accent = "é") are treated as multiple code points for positioning purposes, not as a single grapheme cluster.

- **Column Position**: A 1-based index representing the logical position of a code point within a source line, counted as Unicode code points from the line start.

- **Tab Stop Width**: A configurable parameter (default: 8 columns) that determines the visual width of tab characters. Tab expansion follows the formula: `visualColumn = ((currentColumn - 1) / tabStopWidth + 1) * tabStopWidth + 1`.

- **SourceSpan**: A metadata structure containing error position information, including byte offset, line number, and column information. ErrorReporter receives SourceSpan objects from the LineTracker component, which tracks line boundaries and converts lexer byte offsets to line/column positions.

- **LineTracker**: A component responsible for tracking line boundaries and maintaining a mapping between byte offsets and (line number, column) pairs. ErrorReporter queries LineTracker to retrieve the source line content and line number for a given error position.

## Non-Functional Requirements

### Performance

- **NFR-001**: Unicode code point counting for error marker positioning MUST introduce ≤1% overhead compared to byte-based calculation. Error message formatting latency for Unicode-containing source files MUST be ≤1ms per error message (measured at 95th percentile) and ≤5ms at 99th percentile. Performance MUST be verified via Catch2 `BENCHMARK` macros in `test/benchmark.cpp` with statistics showing mean execution time and standard deviation across 100 iterations.

### Scale

- **NFR-002**: ErrorReporter MUST correctly handle source lines containing up to 10,000 Unicode code points. Lines exceeding this limit have undefined behavior.

### User Experience

- **NFR-003**: Error markers MUST support optional ANSI color output for improved error detection in terminals that support it. Color support detection MUST check environment variables (`COLORTERM`, `TERM`) and respect `NO_COLOR` standard. Error markers MUST use red color (ANSI escape code `\033[31m`) for caret symbols. When color is unavailable or disabled, the marker row MUST fall back to monochrome caret characters ('^') with identical caret count and column positions as colored output (zero positioning deviation).

## Success Criteria

### Measurable Outcomes

- **SC-001**: For any source file containing valid UTF-8, the visual marker caret appears directly beneath the correct Unicode code point in 100% of test cases, verified by visual inspection or automated code point-position matching.

- **SC-002**: For source files containing only ASCII code points, error message output is byte-for-byte identical to the current implementation (zero regression).

- **SC-003**: Invalid UTF-8 byte sequences are reported as encoding errors with correct byte offset and line number in 100% of test cases.

- **SC-004**: Error marker alignment is correct for all edge cases defined in User Story 3 (first code point, last code point, empty line, combining code points, tab code points) in 100% of test cases.

- **SC-005**: No fallback to byte-based column calculation occurs under any circumstance, verified by testing files with mixed valid and invalid UTF-8 sequences.

- **SC-006**: Users can identify the error position in Unicode-containing source files within 5 seconds of reading the error message (compared to potentially minutes with incorrect byte-based positioning).

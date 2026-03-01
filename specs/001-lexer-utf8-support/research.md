# Research: Lexer Full UTF-8 Support

**Branch**: `001-lexer-utf8-support` | **Date**: 2026-03-01

## R-01: UTF-8 Validated Decoding Algorithm

**Question**: What algorithm should be used for validated UTF-8 decoding with maximal
subpart error recovery in constexpr C++23?

**Decision**: If-else cascade decoder with inline validation, decomposed into
sub-functions per sequence length (`decode_2byte`, `decode_3byte`, `decode_4byte`).

**Rationale**:

- **Maximal subpart** (Unicode Standard Chapter 3, Section 3.9, D93b): the longest
  initial sub-sequence of an ill-formed sequence that is either a valid prefix of a
  well-formed sequence or a single byte that is not part of any valid sequence.
  The if-else cascade implements this naturally — when a continuation byte fails
  validation, the decoder knows exactly how many bytes to consume as the error
  and where to resume.
- **ASCII fast-path**: A single `if (first < 0x80)` test as the first branch handles
  the >95% ASCII case with one comparison and zero table access.
- **Diagnostic granularity**: Each error type (overlong, surrogate, out-of-range,
  orphaned continuation, truncated, invalid lead) can set a distinct `Utf8Status`
  enum value, satisfying FR-022 diagnostic requirements.
- **constexpr native**: Pure computation with no table lookup on the hot path.
  MSVC 19.44+ evaluates this without hitting constexpr step limits.
- **Cyclomatic complexity**: Decomposition into sub-functions keeps each function
  under CCN 15 (Lizard threshold). Dispatcher ~5, each sub-function ~4-6.

**Alternatives considered**:

| Alternative | Rejected because |
|-------------|-----------------|
| DFA (Bjoern Hoehrmann) | Every byte (including ASCII) pays table lookup cost; produces single "error" state without distinguishing overlong/surrogate/truncated; maximal subpart requires post-processing |
| Large table-driven | Same ASCII penalty as DFA; constexpr evaluation of large tables slower on MSVC; no diagnostic granularity |
| SIMD (SSE4.2/Neon) | Not constexpr; not cross-platform without `#ifdef`; out of scope for initial implementation |

## R-02: Decode Result Type Design

**Question**: What return type should `decode_utf8()` use?

**Decision**: POD struct `Utf8DecodeResult { char32_t codepoint; std::uint8_t byte_length; Utf8Status status; }`.

**Rationale**:

- `byte_length` is always needed (success and error) — the caller must know how many
  bytes to advance. `std::expected` would require duplicating `byte_length` in both
  the value and error types.
- The lexer needs the codepoint even on error for diagnostic messages like
  "unexpected Unicode character U+1F600".
- Compact layout: 8 bytes (4 + 1 + 1 + 2 padding). Optimal for value passing.
- Pattern matching via `switch (result.status)` is idiomatic and clean.
- `constexpr` POD aggregate initialization works on all three target compilers.

**Alternatives considered**:

| Alternative | Rejected because |
|-------------|-----------------|
| `std::expected<char32_t, Utf8Error>` | `byte_length` doesn't fit cleanly in either side; error side lacks codepoint for diagnostics |
| `std::optional<char32_t>` + out param | Out parameters (pointers) are less constexpr-friendly |
| `std::pair<char32_t, uint8_t>` + sentinel | No type-safety for distinguishing error classes |

## R-03: Unicode General Category Lookup Table Representation

**Question**: How should Unicode General Category data be stored for constexpr
binary search lookup?

**Decision**: Three separate sorted `constexpr std::array<CodepointRange, N>` arrays —
one per classifier function (`id_start`, `id_continue`, `whitespace`).

**Rationale**:

- The lexer needs only 3 boolean classifiers, never the actual General Category.
  Separate arrays avoid carrying unused data.
- Estimated sizes: ~750 ranges for id\_start (L), ~850 for id\_continue (L+M+N),
  ~18 for whitespace (Zs+Zl+Zp). Total ~13 KB compiled.
- `constexpr std::array` with ~850 elements is well within MSVC constexpr evaluation
  limits.
- Binary search via `std::upper_bound` (constexpr since C++20, verified on all three
  compilers) yields O(log n) ≈ 10 comparisons.
- Each function has an ASCII fast-path (`if (cp < 0x80)`) that avoids binary search
  entirely for the common case.

**Alternatives considered**:

| Alternative | Rejected because |
|-------------|-----------------|
| Single unified `{start, end, category}` array | +33% per-entry size (category field unused); post-filter needed after binary search |
| Two-stage table (ICU-style) | ~64 KB minimum; complex generation; risks hitting MSVC constexpr limits |
| Bitset indexed by codepoint | ~136 KB per category × 3 = ~408 KB; too large for constexpr; massive source file |

## R-04: Binary Search Implementation

**Question**: Hand-rolled binary search or `std::upper_bound`?

**Decision**: `std::upper_bound` from `<algorithm>` with a custom comparator.

**Rationale**:

- `std::upper_bound` is constexpr since C++20 (P0202R3). Verified on MSVC 19.44+
  (since VS 2019 16.6), GCC 13.2 (since GCC 10), and Clang 17 (since Clang 12).
- Lookup pattern: `it = upper_bound(ranges, cp, cmp_on_first)` → if `it == begin()`
  then not found, else `--it` and check `cp <= it->last`.
- Standard library is well-tested; hand-rolled binary search adds bug risk with
  zero benefit.

**Alternatives considered**:

| Alternative | Rejected because |
|-------------|-----------------|
| Hand-rolled binary search | Duplicates standard library; more bug-prone; no advantage |
| `std::ranges::upper_bound` with projection | Valid alternative (cleaner syntax) but classic `std::upper_bound` is more widely tested in constexpr contexts on MSVC |
| `std::lower_bound` | Requires slightly different logic; `upper_bound` + decrement is the idiomatic pattern for range-contains queries |

## R-05: Public API Function Signatures

**Question**: What should the Unicode classification API look like?

**Decision**: Four free functions in `namespace jsv::unicode`:

- `constexpr bool is_letter(char32_t cp) noexcept` — General Category L
- `constexpr bool is_id_start(char32_t cp) noexcept` — `\p{Letter}` or `_`
- `constexpr bool is_id_continue(char32_t cp) noexcept` — `\p{Letter}` + `\p{Mark}` + `\p{Number}` + `_` + ASCII digits
- `constexpr bool is_unicode_whitespace(char32_t cp) noexcept` — General Category Zs + Zl + Zp

**Rationale**:

- Separation from `Lexer` class enables reuse (future parser, semantic analysis).
- `char32_t` is semantically correct for Unicode code points (replacing current
  `std::uint32_t`).
- `is_letter()` exposed separately for testing and future diagnostic messages.
- `is_id_start` delegates: `return cp == U'_' || is_letter(cp)`.
- All functions include ASCII fast-path before binary search.
- `[[nodiscard]]` and `noexcept` per AI\_GUIDELINES.md conventions.

**Alternatives considered**:

| Alternative | Rejected because |
|-------------|-----------------|
| Single `general_category(char32_t)` returning enum | Requires unified table; caller must switch on result; more data, more complexity for 3 boolean use cases |
| Static member functions on `Lexer` | Couples Unicode classification to Lexer; prevents reuse |

## R-06: Python Generation Script Design

**Question**: How should the offline script generate the C++ lookup tables?

**Decision**: Standalone Python script at `scripts/generate_unicode_tables.py` that
downloads UnicodeData.txt from a pinned Unicode version URL, parses it, merges
adjacent ranges per classifier, and outputs a formatted C++ header.

**Rationale**:

- **Version pinning**: Hardcoded `UNICODE_VERSION = "16.0.0"` in URL. Never uses
  `latest/`. Reproducible output.
- **Range parsing**: UnicodeData.txt format is `codepoint;name;general_category;...`
  (14 semicolon-separated fields). Script handles `<..., First>` / `<..., Last>`
  range markers for CJK blocks.
- **Three merge passes**: One per classifier (id\_start = L, id\_continue = L+M+N,
  whitespace = Zs+Zl+Zp). Adjacent codepoints with same classification are merged
  into `{first, last}` pairs.
- **Output format**: `constexpr std::array<CodepointRange, N>` with N calculated
  exactly. ~4 entries per line for readability. Includes `static_assert` size guards,
  generation metadata header, `#pragma once`, and minimal includes.
- **Built-in validation**: Script verifies ranges are sorted, non-overlapping, and
  count is within expected order of magnitude.
- **Not a build dependency**: Generated file is committed to the repository. CMake
  does not invoke Python. Unicode version updates are manual and deliberate.

**Alternatives considered**:

| Alternative | Rejected because |
|-------------|-----------------|
| Use DerivedCoreProperties.txt (XID\_Start/XID\_Continue) | Spec explicitly chose General Category (L/M/N) over XID properties |
| Build-time generation via CMake custom command | Adds Python as build dependency; violates project constraint |
| UCD XML (ucd.all.flat.xml) | ~300 MB input vs ~2 MB for UnicodeData.txt; complex parsing for no benefit |

## R-07: Impact on Existing Lexer Code

**Question**: What is the migration path from current hardcoded XID ranges to
General Category lookup?

**Decision**: Replace `Lexer::is_xid_start()` and `Lexer::is_xid_continue()` static
methods with calls to `jsv::unicode::is_id_start()` and `jsv::unicode::is_id_continue()`.
Add BOM detection at the start of `tokenize()`. Add Unicode whitespace recognition
in `skip_whitespace_and_comments()`. Upgrade `peek_codepoint()`/`advance_codepoint()`
to use validated `decode_utf8()`.

**Rationale**:

- Current `is_xid_start` covers ~12 scripts with hardcoded ranges (~80 lines).
  Replacing with a single function call to the generated table is a net simplification.
- Current `peek_codepoint`/`advance_codepoint` have zero validation (no overlong,
  surrogate, or range checks). They must be rewritten to use `Utf8DecodeResult`.
- `skip_whitespace_and_comments()` currently checks only ASCII whitespace. Adding
  `jsv::unicode::is_unicode_whitespace()` call for non-ASCII bytes is a
  single-branch addition.
- BOM skip is a 3-byte prefix check at the start of `tokenize()`.
- All changes are within the lexer module — no impact on Token, SourceSpan, or
  downstream consumers.

**Alternatives considered**:

| Alternative | Rejected because |
|-------------|-----------------|
| Keep XID hardcoded ranges and expand them | Unmaintainable; would grow to ~750 ranges of hand-written code |
| Use ICU or uni-algo library | Adds external dependency violating constitution principle V |

## R-08: Testing Strategy for UTF-8 and Unicode Classification

**Question**: How should the new functionality be tested across the three test tiers?

**Decision**: Constexpr tests first (relaxed\_constexpr → constexpr), runtime tests
for I/O-dependent and malformed-input scenarios.

**Rationale**:

- **constexpr\_tests.cpp**: `STATIC_REQUIRE` for `decode_utf8()` with known byte
  sequences (valid 1/2/3/4-byte, overlong, surrogate, orphaned, truncated).
  `STATIC_REQUIRE` for `is_id_start()`, `is_id_continue()`, `is_unicode_whitespace()`
  with representative codepoints from multiple scripts.
- **tests.cpp**: Runtime `REQUIRE` for full lexer tokenization of UTF-8 source
  strings (multi-byte identifiers, malformed sequences producing error tokens,
  BOM skipping, Unicode whitespace consumption, ASCII backward compatibility).
- Test naming: `Utf8Decoder_OverlongTwoByte_ReturnsOverlongError`,
  `UnicodeClassifier_CJKIdeograph_IsIdStart`,
  `Lexer_MalformedUtf8MidFile_EmitsErrorAndContinues`.
- TDD workflow: Write failing test → implement minimum code → verify green → refactor.

**Alternatives considered**: None — this follows the constitution's mandatory
three-tier test pyramid and TDD workflow.

## Summary of All Decisions

| ID | Topic | Decision |
|----|-------|---------|
| R-01 | Decode algorithm | If-else cascade with maximal subpart, decomposed per sequence length |
| R-02 | Result type | POD struct `Utf8DecodeResult{char32_t, uint8_t, Utf8Status}` |
| R-03 | Lookup tables | 3 separate sorted `constexpr std::array<CodepointRange, N>` (~13 KB total) |
| R-04 | Binary search | `std::upper_bound` (constexpr C++20, all compilers verified) |
| R-05 | Public API | 4 free functions in `jsv::unicode` with `char32_t` + ASCII fast-path |
| R-06 | Python script | Standalone, version-pinned, 3 merge passes, committed output |
| R-07 | Migration | Replace XID statics → `jsv::unicode` calls; upgrade decode; add BOM + Unicode WS |
| R-08 | Testing | Constexpr tests for decode/classify; runtime tests for full lexer tokenization |

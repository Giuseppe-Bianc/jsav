# Research: Full UTF-8 Unicode Whitespace Support in Lexer

**Feature Branch**: `002-utf8-unicode-whitespace`
**Date**: 2026-03-02

## R-01: Reuse existing decode_utf8 vs. new decoder in Lexer.cpp

**Decision**: Reuse the existing `unicode::decode_utf8(string_view, size_t)` from `Utf8.hpp`.

**Rationale**: The spec proposed adding a new `decode_utf8(const char*, const char*, uint32_t&)`
in an anonymous namespace within `Lexer.cpp`. However, `Utf8.hpp` already provides a fully
`constexpr`, production-quality decoder that handles all error categories (overlong, surrogate,
truncated, invalid lead bytes, orphaned continuation). The existing decoder returns a
`Utf8DecodeResult` struct with `codepoint`, `byte_length`, and `status` — exactly the
information needed. `Lexer.cpp` already includes `Utf8.hpp` and calls `decode_utf8()` in
`skip_unicode_whitespace()`, `advance_codepoint()`, and `peek_codepoint()`. Adding a parallel
decoder with a different signature would:

- Violate the constitution's code consistency principle (Pattern: Pervasive Const Correctness,
  no duplication)
- Create maintenance divergence when UTF-8 edge cases are discovered
- Add ~60 lines of redundant logic

The only functional difference from the spec's proposed helper is the return convention (byte
length vs. struct). The existing struct-based return is strictly more informative.

**Alternatives considered**:

1. **New decoder per spec** — rejected: code duplication, maintenance burden
2. **Thin wrapper** (adapts `Utf8DecodeResult` to return `int`) — rejected: unnecessary
   indirection, `skip_unicode_whitespace()` already uses the struct directly
3. **Reuse as-is** — chosen: zero new code, proven correct, already tested with 30+
   constexpr tests in the codebase

## R-02: Where to add VT (U+000B) and FF (U+000C) handling

**Decision**: Add `case '\v':` and `case '\f':` to the ASCII fast-path `switch` in
`skip_whitespace_and_comments()`, treated identically to space/tab (advance byte, increment
column by 1, no line change).

**Rationale**: VT and FF are single-byte ASCII characters (0x0B and 0x0C). They are
`\p{White_Space}` code points in category Cc. The existing `is_unicode_whitespace()` function
intentionally excludes them (it covers only Zs/Zl/Zp categories). Adding them to the
ASCII switch is the zero-overhead approach — no non-ASCII path is entered. This matches
FR-002 exactly.

**Alternatives considered**:

1. **Add to `whitespace_ranges` in UnicodeData.hpp** — rejected: spec explicitly forbids
   changing that table; would also change semantics for all callers
2. **Check in non-ASCII path** — rejected: VT/FF are < 0x80, would never reach the
   non-ASCII branch
3. **ASCII switch** — chosen: correct, fast, localized change

## R-03: NEL (U+0085) handling as line terminator

**Decision**: Add a special case for U+0085 in `skip_unicode_whitespace()` before the
`is_unicode_whitespace()` check. NEL is a 2-byte UTF-8 sequence (0xC2 0x85) in category
Cc, so `is_unicode_whitespace()` returns false for it. It must be recognized as both
whitespace and a line terminator.

**Rationale**: U+0085 (NEXT LINE) has the `\p{White_Space}` property but belongs to
category Cc (control), not Zs/Zl/Zp. The `is_unicode_whitespace()` function intentionally
does not include it. The lexer needs to:

1. Decode the UTF-8 sequence (already done by `decode_utf8()`)
2. Check if codepoint == U+0085
3. Treat as line terminator: increment line, reset column

This check must come before the `is_unicode_whitespace()` call because NEL would otherwise
fall through as "not whitespace" and exit the loop.

**Alternatives considered**:

1. **Add NEL to `whitespace_ranges`** — rejected: spec forbids, changes all callers
2. **Separate `is_full_whitespace()` function** — rejected: overengineered for one code point
3. **Inline check in `skip_unicode_whitespace()`** — chosen: minimal, clear, correct

## R-04: is_unicode_line_terminator helper placement and scope

**Decision**: Add a `constexpr` free function `is_unicode_line_terminator(char32_t cp)` in the
`jsv::unicode` namespace in `UnicodeData.hpp`, returning `true` for U+0085, U+2028, U+2029 only.

**Rationale**: The spec requests this helper. Placing it in `UnicodeData.hpp` alongside
`is_unicode_whitespace()` is the natural location for Unicode classification functions. It
is a pure classifier (no state, constexpr, noexcept) and aligns with the project's pattern
of keeping Unicode utilities in the `unicode` namespace. LF (U+000A) is excluded because it
is already handled by the ASCII path in `skip_whitespace_and_comments` and does not need
runtime classification.

Wait — the spec says "returns true for U+0085 (NEL), U+2028 (LINE SEPARATOR), and U+2029
(PARAGRAPH SEPARATOR) only" and "is placed in anonymous namespace within Lexer.cpp". However,
the constitution says to keep Unicode classification functions in `UnicodeData.hpp`. Also,
`constexpr_tests.cpp` needs to test it, which requires it to be in a header.

**Revised decision**: Place `is_unicode_line_terminator` in `UnicodeData.hpp` as a public
`constexpr` function in `jsv::unicode` namespace. This:

- Enables `STATIC_REQUIRE` testing from `constexpr_tests.cpp`
- Follows existing pattern (`is_unicode_whitespace`, `is_id_start`, `is_id_continue`)
- Zero runtime cost (constexpr, trivial comparisons)

**Alternatives considered**:

1. **Anonymous namespace in Lexer.cpp** — rejected: cannot be tested from constexpr_tests.cpp
   without exposing it; breaks test-first workflow
2. **In UnicodeData.hpp** — chosen: testable, consistent with project patterns
3. **In Utf8.hpp** — rejected: Utf8.hpp is for encoding/decoding, not classification

## R-05: Line terminator set — CR excluded

**Decision**: CR (U+000D) is NOT a line terminator.

**Rationale**: The existing lexer treats CR as plain whitespace (advance byte, increment
column). CR+LF sequences produce one line increment via the LF. This behavior is explicitly
preserved per FR-006 and FR-009. Changing CR to a line terminator would break CR+LF handling
(double increment). The spec clarification from 2026-03-02 confirms this.

Line terminators are exactly: LF (U+000A) [ASCII path], NEL (U+0085), LINE SEPARATOR
(U+2028), PARAGRAPH SEPARATOR (U+2029) [non-ASCII path].

**Alternatives considered**: None — this is a settled clarification from the spec.

## R-06: Column tracking for multi-byte whitespace

**Decision**: Non-line-terminating whitespace advances the column by the UTF-8 byte
length (2 or 3 bytes), consistent with the existing byte-offset convention.

**Rationale**: The project uses byte-based columns (1-indexed). The existing
`skip_unicode_whitespace()` already does `m_column += res.byte_length` for non-line-terminating
whitespace. No change needed in this logic — it is already correct. Line terminators (NEL,
U+2028, U+2029) reset the column to 1 regardless of byte length.

**Alternatives considered**: None — existing behavior is correct.

## R-07: Invalid UTF-8 handling in whitespace scanning

**Decision**: When `decode_utf8()` returns a non-Ok status, or the decoded codepoint is not
whitespace, `skip_unicode_whitespace()` returns `false`. The caller (`skip_whitespace_and_comments`)
then `break`s out of the loop, passing control to `next_token()` for normal token processing.
The invalid bytes are NOT consumed.

**Rationale**: The existing code already implements this pattern:

```cpp
if(res.status != unicode::Utf8Status::Ok || !unicode::is_unicode_whitespace(res.codepoint)) {
    return false;
}
```

The `break` in the caller ensures the whitespace loop terminates. `next_token()` will then
attempt to process the bytes as a token (or produce an error token). This satisfies FR-004
(no crash, no hang, no infinite loop) and FR-010 (truncated sequences handled gracefully).

The only modification needed is to also check for NEL before this guard so that NEL is
consumed as whitespace even though `is_unicode_whitespace()` returns false for it.

**Alternatives considered**: None — existing pattern is correct and robust.

## R-08: Benchmark approach for ASCII throughput regression

**Decision**: Use Catch2's `BENCHMARK` macro in `tests.cpp`. Generate a 1 MB string of
`"var x\n"` repeated, tokenize 100 times, assert throughput does not regress beyond 5%.

**Rationale**: Catch2 v3 includes a built-in benchmarking facility (`BENCHMARK`). The project
already links `Catch2::Catch2WithMain`. A baseline measurement can be captured as a constant
(tokens/second or MB/s), and the benchmark asserts the measured throughput is within 95% of
that baseline. This avoids adding any new dependency (e.g., Google Benchmark).

**Alternatives considered**:

1. **Google Benchmark** — rejected: new dependency, violates constitution Principle V
2. **Manual timing with `<chrono>`** — rejected: less accurate, no warm-up/statistics
3. **Catch2 BENCHMARK** — chosen: zero new deps, integrated with test runner

## R-09: Test organization — constexpr vs runtime

**Decision**: Tests are split across two files per the project's established pattern:

- `constexpr_tests.cpp`: `STATIC_REQUIRE`-based tests for `is_unicode_line_terminator()`.
  The spec's request for decode_utf8 constexpr tests is already covered by existing tests in
  this file (T070–T084 cover all decode scenarios). Only the new
  `is_unicode_line_terminator()` truth table needs testing here.
- `tests.cpp`: All runtime Lexer tests (token separation, line/column tracking, robustness,
  backward compatibility, benchmark).

**Rationale**: `decode_utf8()` is NOT being added or modified — it already exists in `Utf8.hpp`
and already has comprehensive constexpr tests. Adding 13 more `static_assert` tests for the
same function would be redundant. The spec's constexpr test requirements are reinterpreted as:
test `is_unicode_line_terminator()` truth table (new function) plus verify that the new code
paths are exercised through runtime Lexer tests.

**Alternatives considered**: Duplicating decode tests — rejected: tests already exist.

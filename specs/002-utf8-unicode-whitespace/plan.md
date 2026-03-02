# Implementation Plan: Full UTF-8 Unicode Whitespace Support in Lexer

**Branch**: `002-utf8-unicode-whitespace` | **Date**: 2026-03-02 | **Spec**: [spec.md](spec.md)
**Input**: Feature specification from `/specs/002-utf8-unicode-whitespace/spec.md`

## Summary

Extend the lexer's `skip_whitespace_and_comments` function to recognize all 26 Unicode
`\p{White_Space}` code points (currently only ASCII space, tab, CR, LF are handled). VT and FF
are added to the ASCII fast-path; NEL (U+0085) is special-cased as a line terminator in the
non-ASCII path; all Zs/Zl/Zp whitespace is already handled by the existing
`is_unicode_whitespace()` + `skip_unicode_whitespace()` infrastructure. A companion
`is_unicode_line_terminator()` constexpr helper is added via the Python generator script
(`scripts/generate_unicode_tables.py`) that produces `UnicodeData.hpp`. No new files, no new
dependencies.

## Technical Context

**Language/Version**: C++23 (MSVC / Visual Studio 2026, GCC 14.x, Clang 18.x)
**Primary Dependencies**: Catch2 3.13.0 (test only), fmtlib, spdlog, CLI11 — no new deps
**Storage**: N/A (in-memory lexer)
**Testing**: Catch2 — three targets: `constexpr_tests` (STATIC_REQUIRE), `relaxed_constexpr_tests`, `tests` (runtime)
**Target Platform**: Cross-platform (Windows, Linux, macOS)
**Project Type**: Compiler (lexer component of the jsav language)
**Performance Goals**: ASCII-only tokenization throughput must not regress >5% (SC-005)
**Constraints**: Zero sanitizer findings (ASan + UBSan), all functions ≤100 LOC / CCN ≤15 / ≤6 params
**Scale/Scope**: ~95 net lines in Lexer.cpp, ~120 lines constexpr tests, ~350 lines runtime tests — 3 files total

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| # | Principle | Status | Notes |
|---|-----------|--------|-------|
| I | Platform Independence | PASS | Pure C++ standard library, no OS APIs |
| II | VS 2026 Compatibility | PASS | C++23 constexpr, no feature gaps in MSVC |
| III | C++ Core Guidelines | PASS | constexpr helpers, no raw pointers, no new/delete, string_view usage, const-correct |
| IV | TDD (Red-Green) | PASS | Tests specified before implementation; constexpr-first workflow |
| V | Dependency Management | PASS | No new dependencies; existing Utf8.hpp decoder reused |
| VI | Documentation Standards | PASS | Plan + spec in Markdown; code comments follow conventions |

**Complexity gate**: `skip_whitespace_and_comments` currently has ~35 lines. With VT/FF/NEL additions
it will grow to ~70 lines. The two-tier structure (ASCII switch + non-ASCII decode) keeps CCN within
the ≤15 threshold. The `is_unicode_line_terminator` helper is 3 lines. No violations anticipated.

**Code duplication gate**: The spec mentions adding a new `decode_utf8(const char*, const char*,
uint32_t&)` in Lexer.cpp, but `Utf8.hpp` already provides a robust constexpr `decode_utf8(string_view,
size_t)` that handles all error cases (overlong, surrogate, truncated, invalid lead bytes). Duplicating
this logic would violate the constitution's code consistency principles. **Decision**: Reuse the existing
`unicode::decode_utf8()` from Utf8.hpp (already included in Lexer.cpp). The `skip_unicode_whitespace()`
method already calls it. No new decoder needed.

## Project Structure

### Documentation (this feature)

```text
specs/002-utf8-unicode-whitespace/
├── plan.md              # This file
├── research.md          # Phase 0 output
├── data-model.md        # Phase 1 output
├── quickstart.md        # Phase 1 output
└── tasks.md             # Phase 2 output (NOT created by /speckit.plan)
```

### Source Code (files touched)

```text
scripts/generate_unicode_tables.py          # Add is_unicode_line_terminator() to generated output
include/jsav/lexer/unicode/UnicodeData.hpp  # REGENERATED (not hand-edited) — gains is_unicode_line_terminator()
src/jsav_Lib/lexer/Lexer.cpp                # Modify skip_whitespace_and_comments, skip_unicode_whitespace
include/jsav/lexer/Lexer.hpp                # No changes needed (methods already declared)
include/jsav/lexer/unicode/Utf8.hpp         # No changes (reused as-is)
test/constexpr_tests.cpp                    # Add ~13 static_assert tests for decode + line terminator
test/tests.cpp                              # Add ~28 runtime tests (token separation, line/col, robustness, compat, benchmark)
test/CMakeLists.txt                         # No changes needed
```

**IMPORTANT**: `UnicodeData.hpp` is auto-generated. The "DO NOT EDIT" header forbids hand-edits.
All changes to this file must go through `scripts/generate_unicode_tables.py` → regenerate.

**Structure Decision**: Existing single-project layout. All changes within the three files
already in the build system. No CMake modifications required.

## Complexity Tracking

> No violations. All changes within existing thresholds.

## Post-Design Constitution Re-Check

*Re-evaluated after Phase 1 artifact generation.*

| # | Principle | Status | Notes |
|---|-----------|--------|-------|
| I | Platform Independence | PASS | No OS-specific APIs. Pure C++23 standard library. |
| II | VS 2026 Compatibility | PASS | constexpr, [[nodiscard]], noexcept — all MSVC-supported |
| III | C++ Core Guidelines | PASS | const-correct, no raw pointers, CCN ~12, ≤70 LOC |
| IV | TDD (Red-Green) | PASS | Tests specified before implementation; constexpr-first |
| V | Dependency Management | PASS | No new dependencies; reuse existing Utf8.hpp decoder |
| VI | Documentation Standards | PASS | All Markdown artifacts pass structure checks |

**Result**: No violations. Design approved for Phase 2 (task generation).

# Implementation Plan: Lexer Full UTF-8 Support

**Branch**: `001-lexer-utf8-support` | **Date**: 2026-03-01 | **Spec**: [spec.md](spec.md)
**Input**: Feature specification from `/specs/001-lexer-utf8-support/spec.md`

**Note**: This template is filled in by the `/speckit.plan` command. See `.specify/templates/plan-template.md` for the execution workflow.

## Summary

Upgrade the jsav lexer to achieve full UTF-8 compliance: validated multi-byte decoding with error recovery (overlong, surrogate, truncated, orphaned), Unicode General Category-based identifier classification (L for start, L+M+N for continue) replacing the current incomplete XID hardcoded ranges, Unicode whitespace recognition (Zs/Zl/Zp), BOM skipping, constexpr-compatible lookup tables generated offline from UnicodeData.txt via a Python script, and comprehensive test coverage across all three test tiers.

## Technical Context

**Language/Version**: C++23 with MSVC 19.44+ (VS 2026), GCC 13.2, Clang 17.0
**Primary Dependencies**: fmt 12.1.0, spdlog 1.17.0, Catch2 3.13.0, CLI11 2.6.1 (via CPM.cmake)
**Storage**: N/A (source read once from disk into `std::string`, all views into that buffer)
**Testing**: Catch2 — `constexpr_tests.cpp` (STATIC_REQUIRE), `relaxed_constexpr_tests` (runtime constexpr), `tests.cpp` (REQUIRE)
**Target Platform**: Cross-platform (Windows, Linux, macOS) — OS-independent
**Project Type**: Compiler (modular monolith: jsav_core_lib, jsav_lib, jsav executable)
**Performance Goals**: ASCII-only tokenization within 10% of pre-change baseline; Unicode lookup in sub-linear time (binary search)
**Constraints**: Zero-copy token stream (string_view into source buffer); all lookup tables constexpr; no new runtime dependencies
**Scale/Scope**: Single-developer project; lexer directory currently ~650 LOC; Unicode lookup tables will add ~200–500 LOC of generated constexpr data

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

### I. Platform Independence — PASS

All UTF-8 decoding and Unicode General Category classification will use pure C++23 standard library
features (`constexpr std::array`, `std::string_view`, `std::expected`). No platform-specific APIs.
The offline Python script generates portable C++ source code; Python is not a build-time dependency.

### II. Visual Studio 2026 Compatibility — PASS

Features used: `constexpr std::array`, `std::expected<T,E>`, `std::string_view`, `if constexpr`,
`constexpr` functions with `std::array` lookups. All verified as supported by MSVC 19.44+.
No conditionally compiled paths needed for this feature.

### III. C++ Core Guidelines Compliance — PASS

- **Ownership Semantics**: Lookup tables are `constexpr std::array` (static storage duration, no
  allocation). Token stream remains zero-copy `string_view` into the source buffer. No new heap
  allocations introduced.
- **Pervasive Const Correctness**: Lookup tables are `constexpr`/`const`. Decoding functions take
  `const` views. Classification functions are `static constexpr noexcept`.
- **Move Semantics**: No new movable resources. `std::vector<Token>` is already moved at phase
  boundary.
- **Structured Error Handling**: Malformed UTF-8 produces `TokenKind::Error` tokens with diagnostic
  messages. Multi-error accumulation already in place. No new exception paths.

### IV. Test-Driven Development (Red-Green) — PASS

- All new functionality (decoding validation, General Category classification, BOM handling,
  Unicode whitespace, error recovery) will follow Red-Green-Refactor.
- Constexpr tests first in `relaxed_constexpr_tests`, then `constexpr_tests`.
- Runtime tests in `tests.cpp` for I/O-dependent and non-constexpr scenarios.
- Test naming: `[Unit]_[Scenario]_[ExpectedResult]`.

### V. Dependency Management — PASS

- No new dependencies required. Unicode lookup tables are generated offline and committed
  as C++ source.
- Existing dependency versions unchanged.
- Generated file committed to repository — no build-time Python dependency.

### VI. Documentation Standards — PASS

- Generated Unicode tables file will have a documentation header indicating Unicode version,
  generation date, and script path.
- All new Markdown artifacts in `specs/001-lexer-utf8-support/` will pass markdownlint.

### Gate Result: **ALL PASS** — Proceed to Phase 0

## Project Structure

### Documentation (this feature)

```text
specs/[###-feature]/
├── plan.md              # This file (/speckit.plan command output)
├── research.md          # Phase 0 output (/speckit.plan command)
├── data-model.md        # Phase 1 output (/speckit.plan command)
├── quickstart.md        # Phase 1 output (/speckit.plan command)
├── contracts/           # Phase 1 output (/speckit.plan command)
└── tasks.md             # Phase 2 output (/speckit.tasks command - NOT created by /speckit.plan)
```

### Source Code (repository root)

```text
include/jsav/lexer/
├── Lexer.hpp              # Updated: new UTF-8 validation, General Category classification
├── Token.hpp              # Unchanged (TokenKind::Error already exists)
├── SourceLocation.hpp     # Unchanged
├── SourceSpan.hpp         # Unchanged
└── unicode/
    ├── Utf8.hpp           # New: constexpr UTF-8 decode/validate functions
    └── UnicodeData.hpp    # New: generated constexpr General Category lookup tables

src/jsav_Lib/lexer/
├── Lexer.cpp              # Updated: validated decode, GC-based classification, BOM skip,
│                          #          Unicode whitespace, error recovery
└── unicode/
    └── Utf8.cpp           # New: non-constexpr helpers (if any; may be header-only)

scripts/
└── generate_unicode_tables.py  # New: offline script to generate UnicodeData.hpp

test/
├── tests.cpp              # Updated: runtime lexer UTF-8 tests
└── constexpr_tests.cpp    # Updated: constexpr UTF-8 decode + GC classification tests
                           # Two build targets: relaxed_constexpr_tests (runtime, debug first)
                           # and constexpr_tests (compile-time STATIC_REQUIRE)
```

**Structure Decision**: New Unicode-specific code goes in a `unicode/` subdirectory under the
existing `include/jsav/lexer/` and `src/jsav_Lib/lexer/` paths, keeping the lexer module
self-contained. The Python generation script goes in a top-level `scripts/` directory.
Tests stay in the existing two files per constitution (no new test files).

## Complexity Tracking

> No constitution violations found. Table left empty.

| Violation | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|-------------------------------------|
| — | — | — |

## Post-Design Constitution Re-evaluation

*Re-check after Phase 1 artifact generation.*

### I. Platform Independence — PASS (no change)

All designed artifacts (Utf8.hpp, UnicodeData.hpp, Lexer.cpp changes) use only
C++23 standard library. The Python generation script is not a build dependency.
No platform-specific APIs introduced.

### II. Visual Studio 2026 Compatibility — PASS (no change)

`Utf8DecodeResult` is a POD aggregate — constexpr-safe on all compilers.
`constexpr std::array<CodepointRange, ~850>` verified within MSVC constexpr
evaluation limits. `std::upper_bound` constexpr since C++20 on MSVC 19.38+.

### III. C++ Core Guidelines — PASS (verified specifics)

- **Ownership**: `Utf8DecodeResult` is a value type (no allocation). `CodepointRange`
  arrays are static constexpr (no allocation). Existing ownership model unchanged.
- **Const correctness**: All new functions are `constexpr` + `[[nodiscard]]` + `noexcept`.
  All lookup tables are `constexpr`. Parameters are `const` or by-value.
- **Error handling**: Malformed UTF-8 → `Utf8Status` enum (structured, typed).
  Lexer converts to `TokenKind::Error` with diagnostic string. No exceptions added.
- **Complexity**: `decode_utf8` decomposed into sub-functions (CCN ≤ 6 each).
  Classification functions are single binary-search calls (CCN ≈ 3).

### IV. Test-Driven Development — PASS (verified specifics)

- Constexpr tier covers: `decode_utf8()` valid/invalid sequences, `is_letter()`,
  `is_id_start()`, `is_id_continue()`, `is_unicode_whitespace()`.
- Runtime tier covers: full lexer tokenization with UTF-8 input, BOM skipping,
  error recovery, string literal validation, Unicode whitespace.
- No test file boundaries violated (constexpr in constexpr\_tests.cpp, runtime in tests.cpp).

### V. Dependency Management — PASS (no change)

Zero new dependencies. Generated Unicode tables are committed C++ source.

### VI. Documentation Standards — PASS (verified specifics)

- Generated header includes metadata (Unicode version, date, generator script path).
- All spec Markdown files follow markdownlint conventions.
- New `jsv::unicode` namespace and public functions documented with `///` comments.

### Post-Design Gate Result: **ALL PASS** — Design approved for Phase 2 task generation

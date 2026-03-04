# Implementation Plan: Complete Numeric Literal Recognition in the Lexer

**Branch**: `003-numeric-literal-lexer` | **Date**: 2026-03-03 | **Spec**: [spec.md](spec.md)
**Input**: Feature specification from `/specs/003-numeric-literal-lexer/spec.md`

## Summary

Rewrite `scan_numeric_literal()` in the existing `Lexer` class to implement the complete
G1→G2→G3 pattern (numeric part → optional exponent → optional type suffix). Modify `next_token()`
to intercept literals with leading dot (`.5`) before the operator branch. Decompose into private
helpers to stay within Lizard limits (CCN ≤ 15). Zero new dependencies, zero new source files —
changes localized to 4 existing files.

## Technical Context

**Language/Version**: C++23 (ISO standard), compilers MSVC 2026 / GCC / Clang
**Primary Dependencies**: Catch2 3.13.0 (test), fmt 12.1.0, spdlog 1.17.0, CLI11 2.6.1
**Storage**: N/A
**Testing**: Catch2 — three tiers: `constexpr_tests`, `relaxed_constexpr_tests`, `tests`
**Target Platform**: Cross-platform (Windows, Linux, macOS)
**Project Type**: Compiler (lexer component)
**Performance Goals**: Single-pass O(n) per literal, no non-linear backtracking, no runtime regex
**Constraints**: Lizard CCN ≤ 15, function length ≤ 100 lines, ≤ 6 params per function
**Scale/Scope**: Changes localized in 2 methods + private helpers in the monolithic `Lexer` class (namespace `jsv`)

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| Principle | Status | Notes |
|-----------|--------|-------|
| I. Platform Independence | ✅ PASS | Pure standard C++, zero OS-specific APIs |
| II. VS 2026 Compatibility | ✅ PASS | Only `std::isdigit`, `char`, `std::string_view` — full MSVC support |
| III. C++ Core Guidelines | ⚠️ RISK | `scan_numeric_literal()` risks CCN > 15 without decomposition into helpers |
| III.a Ownership Semantics | ✅ PASS | No dynamic allocation, operates on `string_view` |
| III.b Const Correctness | ✅ PASS | Helpers will be `const` where possible, `const&` parameters |
| III.c Move Semantics | ✅ N/A | No resources managed in modified code |
| III.d Error Handling | ✅ PASS | Return via `Token`, no exceptions |
| IV. TDD | ✅ PASS | Red-Green-Refactor workflow, tests before implementation |
| V. Dependency Management | ✅ PASS | Zero new dependencies |
| VI. Documentation | ✅ PASS | Inline doc and explanatory comments in methods |

**Risk mitigation III**: Mandatory decomposition of `scan_numeric_literal()` into 3 private
helpers: `try_scan_exponent()` (G2), `try_scan_type_suffix()` (G3), `match_width_suffix()` (utility).
Each method will have CCN ≤ 10.

## Project Structure

### Documentation (this feature)

```text
specs/003-numeric-literal-lexer/
├── plan.md              # This file
├── research.md          # Phase 0 output
├── data-model.md        # Phase 1 output
├── quickstart.md        # Phase 1 output
├── contracts/           # Phase 1 output
├── tasks.md             # Phase 2 output (/speckit.tasks)
└── performance-report.md # Phase 8 output (T053 performance profiling results)
```

### Source Code (repository root)

```text
include/jsav/lexer/
└── Lexer.hpp            # Private helper declarations (Task 3)

src/jsav_Lib/lexer/
└── Lexer.cpp            # Rewrite scan_numeric_literal + modify next_token (Task 1-2)

test/
├── tests.cpp            # Runtime tests Catch2 (Task 4)
└── constexpr_tests.cpp  # Compile-time tests STATIC_REQUIRE (Task 5)
```

**Structure Decision**: Existing jsav project structure — no new files, no new directories.
Changes confined to 4 existing files in the layout already in use.

## Complexity Tracking

| Risk | Mitigation | Alternative rejected because |
|------|------------|------------------------------|
| `scan_numeric_literal()` CCN > 15 | Decomposition into `try_scan_exponent()`, `try_scan_type_suffix()`, `match_width_suffix()` | Monolithic method: would exceed CCN 15 with 3 groups + edge cases |
| Trailing-dot behavior change | Explicit regression tests + comment updates | Maintaining split `123.`→`Numeric+Dot`: contradicts spec FR-003 |

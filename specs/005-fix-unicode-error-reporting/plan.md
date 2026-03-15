# Implementation Plan: Unicode-Aware Error Reporter

**Branch**: `005-fix-unicode-error-reporting` | **Date**: 2026-03-14 | **Spec**: [spec.md](./spec.md)
**Input**: Feature specification from `/specs/005-fix-unicode-error-reporting/spec.md`

**Note**: This template is filled in by the `/speckit.plan` command. See `.specify/templates/plan-template.md` for the execution workflow.

---

## Executive Summary

### Primary Requirement

Implement Unicode-aware error reporting in the jsav compiler by adding a `UnicodeColumn` module that recalculates visual column positions from UTF-8 encoded source files at display time. The solution uses **code point-based column counting** (not byte-based) to ensure error markers (carets) align precisely with Unicode characters in error messages.

### Technical Approach

All UTF-8 validation, tab expansion, and edge case handling (BOM, null bytes, overlong sequences) are contained in the **display layer**, requiring **no changes** to:

- The lexer (byte-based `SourceLocation::column` remains ground truth)
- The `SourceSpan` pipeline (no ABI changes)
- Serialization formats (byte offsets preserved)

### Key Characteristics

| Characteristic | Value |
|----------------|-------|
| **Module Location** | `jsav/error/` (thin-layer addition) |
| **Lines of Code** | ~500-1000 LOC (new + modified) |
| **Test Fixtures** | 3 categories (P1/P2/P3) |
| **External Dependencies** | Zero (reuses existing `jsv::unicode::decode_utf8`) |
| **Backward Compatibility** | Full (ASCII output byte-for-byte identical) |
| **Performance Impact** | Microsecond-level per error message |
| **Memory Overhead** | O(1) auxiliary space |

---

## Technical Context

**Language/Version**: C++23 (GCC 13+ / Clang 17+ / MSVC 19.38+)
**Primary Dependencies**: spdlog (logging), fmt (formatting fallback), Catch2 (testing)
**Storage**: N/A (compiler in-memory processing)
**Testing**: Catch2 v3.13.0 (constexpr_tests, relaxed_constexpr_tests, tests targets)
**Target Platform**: Windows 10/11, Linux (Ubuntu 20.04+), macOS 11+
**Project Type**: Native compiler executable
**Performance Goals**: Zero overhead for Unicode handling (microsecond-level column calculation), <100ms error formatting latency
**Constraints**: ≤10,000 code points per line limit, no external Unicode dependencies, backward-compatible ASCII output
**Scale/Scope**: Single feature addition to error reporting module (~500-1000 LOC), 3 new test fixtures (P1/P2/P3)

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

### Pre-Design Gate Evaluation

| Constitution Rule | Compliance Status | Detailed Justification | Evidence |
|-------------------|-------------------|------------------------|----------|
| **I. Platform Independence** | ✅ Compliant | Implementation uses **only** C++23 standard library and existing project dependencies (spdlog, fmt). **No OS-specific APIs** (e.g., no Windows Console API, no POSIX-specific calls). UTF-8 decoding reuses existing `jsv::unicode::decode_utf8` from `Utf8.hpp` (already used by `LineTracker`). Environment variable detection (`NO_COLOR`, `COLORTERM`, `TERM`) is cross-platform via `std::getenv`. | `data-model.md` Entity 2.1, `research.md` Decision 4 |
| **II. Visual Studio 2026 Compatibility** | ✅ Compliant | All C++23 features used (`std::expected`, `std::string_view`, `std::span`, `std::ranges`, `std::format`) are **fully supported by MSVC 19.38+**. No conditional compilation (`#ifdef _MSC_VER`) required. No compiler-specific extensions. Verified against MSVC C++23 feature table. | `research.md` Decision 2, MSVC documentation |
| **III. C++ Core Guidelines Compliance** | ✅ Compliant | **Ownership**: `UnicodeColumn` functions return `std::expected` with value or error string (no raw pointers, no heap allocation). **Const**: All parameters are `std::string_view` (read-only, no mutation). **Move Semantics**: Not applicable (no heap allocation, Rule of 0). **Error Handling**: `std::expected` for recoverable errors (invalid UTF-8), `LERROR` logging for diagnostics, no exceptions thrown. | `data-model.md` Entity 2, Constitution III Pattern Compliance |
| **IV. Test-Driven Development** | ✅ Compliant | Testing strategy defined in spec: **P1** (Unicode Marker Alignment), **P2** (Invalid UTF-8 Detection), **P3** (Edge Cases). Tests will be written **before implementation** using Red-Green-Refactor cycle. Three-tier structure: unit tests in `tests.cpp` (runtime-only, no constexpr tests needed). Test names follow `ClassName_MethodName_Scenario` convention. | `research.md` Testing Strategy, `spec.md` User Scenarios |
| **V. Dependency Management** | ✅ Compliant | **No new dependencies**. Uses existing: `jsv::unicode::decode_utf8` (from `Utf8.hpp`), spdlog (logging via `LERROR`), FORMAT macros (formatting via `fmt`). All dependencies already approved and version-locked in `Dependencies.cmake`. No "header leak" anti-pattern (dependency headers not exposed in public API). | `research.md` Decision 10, `Dependencies.cmake` |
| **VI. Documentation Standards** | ✅ Compliant | Phase 1 artifacts complete: `plan.md`, `research.md`, `data-model.md`, `quickstart.md`. All public headers in `include/jsav/error/` will have **Doxygen comments** (function signatures, parameter descriptions, return values, examples). Markdown will pass `markdownlint` validation per `.vscode/settings.json` configuration. Heading hierarchy preserved (single H1, progressive H2/H3). | This document, `data-model.md` Entity Documentation |
| **VII. Algorithmic Design Excellence** | ✅ Compliant | Algorithm is **UTF-8 decoding walk** (O(n) time, O(1) space). No advanced algorithmic paradigms required (no dynamic programming, no backtracking). Complexity is **optimal** for the problem class (must examine each byte to decode UTF-8). Formal analysis: Time O(n), Space O(1), where n = line length in bytes. | `data-model.md` Function 2.3/2.4 Algorithm Analysis |

**Gate Verdict**: ✅ **PASS** — All constitution rules are compliant. No violations. No justifications required. Proceeding to Phase 0.

---

### Post-Design Gate Re-Evaluation

*Performed after Phase 1 design artifacts generated (research.md, data-model.md, quickstart.md)*

| Constitution Rule | Re-Evaluation Status | Design Artifact Verification |
|-------------------|----------------------|------------------------------|
| **I. Platform Independence** | Compliant | Verified in `data-model.md`: All functions use standard C++23 library, no OS-specific APIs. `detect_ansi_color()` reads environment variables (cross-platform). |
| **II. Visual Studio 2026 Compatibility** | Compliant | Verified in `research.md` Decision 2: All C++23 features (`std::expected`, `std::string_view`) confirmed MSVC 19.38+ compatible. |
| **III. C++ Core Guidelines Compliance** | Compliant | Verified in `data-model.md` Entity 2: All functions use `std::expected` for error handling (no raw pointers), parameters are `std::string_view` (const, read-only), no heap allocation (Rule of 0). |
| **IV. Test-Driven Development** | Compliant | Verified in `research.md` Testing Strategy: P1/P2/P3 test fixtures defined. Tests will be written before implementation (Red-Green-Refactor). |
| **V. Dependency Management** | Compliant | Verified in `research.md` Decision 10: No new dependencies. Uses existing `jsv::unicode::decode_utf8`. |
| **VI. Documentation Standards** | Compliant | Phase 1 artifacts complete: `plan.md`, `research.md`, `data-model.md`, `quickstart.md`. All entities in `data-model.md` have Doxygen-style documentation. |
| **VII. Algorithmic Design Excellence** | Compliant | Verified in `data-model.md` Function 2.3/2.4: O(n) time, O(1) space complexity. Optimal for UTF-8 decoding walk. No advanced paradigms required. |

**Post-Design Verdict**: ✅ **PASS** — Design artifacts introduce **no new violations**. All constitution rules remain compliant. All justifications from pre-design evaluation remain valid. Proceeding to Phase 2 (Implementation Task Generation).

---

## Project Structure

### Documentation (this feature)

```text
specs/005-fix-unicode-error-reporting/
├── plan.md              # This file (implementation plan, Phase 1 output)
├── research.md          # Phase 0 output (13 technical decisions with rationale)
├── data-model.md        # Phase 1 output (entity definitions, integration points)
├── quickstart.md        # Phase 1 output (usage guide, testing instructions)
├── spec.md              # Input feature specification (user scenarios, requirements)
└── tasks.md             # Phase 2 output (/speckit.tasks command - NOT created by /speckit.plan)
```

**Document Relationships**:

- `spec.md` → Input (requirements, user scenarios)
- `research.md` → Technical decisions (how requirements are met)
- `data-model.md` → Entity definitions (what is implemented)
- `plan.md` → Implementation strategy (when and where)
- `tasks.md` → Task breakdown (step-by-step implementation)
- `quickstart.md` → Usage guide (how to use and test)

### Source Code (repository root)

```text
include/jsav/error/
├── ErrorReporter.hpp    # Modified: add ErrorDisplayConfig member + constructor
└── UnicodeColumn.hpp    # New: public API for Unicode column calculation

src/jsav_Lib/error/
├── ErrorReporter.cpp    # Modified: format_spanned_error uses marker_extents
└── UnicodeColumn.cpp    # New: UTF-8 decoding, tab expansion, validation

include/jsav/core/
└── LineTracker.hpp      # Modified: add source() accessor (one-liner)

test/
└── tests.cpp            # Modified: add P1/P2/P3 test fixtures
```

**Structure Decision**: Single project structure (compiler executable). New files added to existing `jsav/error/` module. No new directories required. Tests added to existing `test/tests.cpp` (runtime unit tests).

**Rationale for Structure**:

1. **Cohesion**: All error reporting logic remains in `jsav/error/` module
2. **Minimal Disruption**: No refactoring of existing modules required
3. **Test Isolation**: New test fixtures in `tests.cpp` do not interfere with existing tests
4. **Backward Compatibility**: Existing API preserved (single-argument constructor remains)

---

## Complexity Tracking

Not applicable — Constitution Check passed with zero violations. No complexity tracking required.

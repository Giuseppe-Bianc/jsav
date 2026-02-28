# Implementation Plan: [FEATURE]

**Branch**: `[###-feature-name]` | **Date**: [DATE] | **Spec**: [link]
**Input**: Feature specification from `/specs/[###-feature-name]/spec.md`

**Note**: This template is filled in by the `/speckit.plan` command. See `.specify/templates/plan-template.md` for the execution workflow.

## Summary

Implement a UTF-8 Unicode lexer for the jsav C++23 compiler that supports complete UTF-8 encoded character sets with full Unicode standard compliance. The lexer handles multi-byte UTF-8 sequences (1-4 bytes), validates Unicode scalar values, supports Unicode XID-based identifiers across 10+ scripts, processes Unicode escape sequences (\uXXXX, \UXXXXXXXX), and provides accurate error reporting for invalid UTF-8 sequences. All Unicode handling is implemented via hand-written constexpr functions covering 10 scripts (Latin, Greek, Cyrillic, Arabic, Hebrew, Devanagari, Thai, CJK, Hangul, Bengali) with zero runtime Unicode dependencies, enabling deterministic compile-time verifiable behavior.

## Technical Context

**Language/Version**: C++23 (GCC 13+, Clang 18+, MSVC 2022+)
**Primary Dependencies**: fmt 12.1.0, spdlog 1.17.0, Catch2 3.13.0, CLI11 2.6.1 (all already integrated)
**Storage**: N/A (lexer operates on in-memory source buffers)
**Testing**: Catch2 framework with constexpr_tests.cpp (compile-time) and tests.cpp (runtime)
**Target Platform**: Cross-platform (Windows, Linux, macOS) - OS-independent implementation
**Project Type**: Compiler (lexer component)
**Performance Goals**: slowdown ratio ≤1.10 vs ASCII-only input (UTF-8 file time / ASCII file time for equivalent byte length), 100% tokenization accuracy
**Constraints**: Zero additional library dependencies, compile-time Unicode tables, ≤15 cyclomatic complexity per function, ≤100 lines per function, zero static analysis warnings
**Scale/Scope**: Single lexer component with full test coverage (≥95% branch coverage), Unicode 15.0 compliance, 10 scripts for XID classification

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

### Principle I: Platform Independence ✅ PASS

- **Evaluation**: Lexer implemented using C++23 standard library exclusively with no platform-specific APIs
- **Evidence**: UTF-8 decoding via constexpr functions, no OS-specific file I/O or encoding APIs
- **Justification**: Hand-written Unicode tables are pure data, not platform-dependent code

### Principle II: Visual Studio 2026 Compatibility ✅ PASS

- **Evaluation**: C++23 features used are fully supported by MSVC 2022+ (constexpr, concepts, std::format)
- **Evidence**: Project already uses C++23 with MSVC; lexer uses standard constexpr capabilities
- **Justification**: No experimental or MSVC-unsupported features required

### Principle III: C++ Core Guidelines Compliance ✅ PASS

- **Evaluation**: Design follows modern C++ practices (strong types, constexpr, enum class, no raw pointers)
- **Evidence**: TokenType as scoped enum, Token uses std::string_view for zero-copy, constexpr Unicode tables
- **Justification**: Function complexity will be enforced via lizard (≤15 cyclomatic, ≤100 lines)

### Principle IV: Test-Driven Development (Red-Green) ✅ PASS

- **Evaluation**: Three-tier testing strategy: constexpr_tests.cpp, tests.cpp, integration tests
- **Evidence**: Spec requires runtime tests in tests.cpp and constexpr tests in constexpr_tests.cpp
- **Justification**: Success criteria include ≥95% branch coverage, verified via gcovr

### Principle V: Dependency Management ✅ PASS

- **Evaluation**: Zero new dependencies; uses existing project dependencies (fmt, spdlog, Catch2, CLI11)
- **Evidence**: Unicode XID classification via hand-written constexpr functions, no external Unicode libraries
- **Justification**: Feature spec explicitly states "No additional libraries are required"

### Principle VI: Documentation Standards ✅ PASS

- **Evaluation**: All artifacts (research.md, data-model.md, quickstart.md, contracts/) will pass markdownlint
- **Evidence**: Plan follows template structure; documentation will be complete and consistent
- **Justification**: markdownlint compliance verified before commit

**GATE STATUS**: ✅ ALL PASS - No violations. Proceeding to Phase 0.

### Post-Design Constitution Check (After Phase 1)

*Re-evaluation after generating research.md, data-model.md, contracts/, and quickstart.md*

#### Principle I: Platform Independence ✅ PASS (Confirmed)

- **Design Artifacts**: All entities use C++23 standard library (std::string_view, std::optional, std::variant)
- **No Platform APIs**: UTF-8 decoder, XID classifier, escape decoder are pure C++
- **Evidence**: data-model.md shows no platform-specific types or APIs

#### Principle II: Visual Studio 2026 Compatibility ✅ PASS (Confirmed)

- **C++23 Features**: constexpr, std::string_view, std::optional, std::variant, enum class — all MSVC 2022+ supported
- **Evidence**: research.md confirms MSVC compatibility for all chosen features

#### Principle III: C++ Core Guidelines Compliance ✅ PASS (Confirmed)

- **Strong Types**: TokenType (enum class), LexicalError (enum class), SourceLocation (struct)
- **Zero-Copy**: Token uses std::string_view (no raw new/delete)
- **Constexpr**: UTF-8 decoder, XID classifier, escape decoder all constexpr
- **Complexity**: Functions designed for ≤15 cyclomatic complexity, ≤100 lines (enforced via lizard)
- **Evidence**: data-model.md shows Rule of 0 compliance, no raw pointers

#### Principle IV: Test-Driven Development ✅ PASS (Confirmed)

- **Three-Tier Testing**: 
    - constexpr_tests.cpp: UTF-8 decoding, XID classification, escape sequences
    - tests.cpp: Full lexer tokenization, error recovery, Unicode identifiers
- **Coverage Target**: ≥95% branch coverage (contract requirement)
- **Evidence**: contracts/lexer-interface.md specifies testing requirements

#### Principle V: Dependency Management ✅ PASS (Confirmed)

- **Zero New Dependencies**: All functionality via hand-written constexpr code
- **Existing Dependencies**: fmt, spdlog, Catch2 (already in project)
- **Evidence**: research.md explicitly rejects ICU, utfcpp, external Unicode libraries

#### Principle VI: Documentation Standards ✅ PASS (Confirmed)

- **Artifacts Generated**: research.md, data-model.md, quickstart.md, contracts/lexer-interface.md
- **Markdownlint**: All documents follow consistent structure, no violations expected
- **Completeness**: All entities documented with fields, validation rules, relationships

**POST-DESIGN GATE STATUS**: ✅ ALL PASS - No violations introduced during Phase 1. Proceeding to Phase 2.

## Project Structure

### Documentation (this feature)

```text
specs/001-utf8-unicode-lexer/
├── plan.md              # This file (Implementation Plan)
├── research.md          # Phase 0 output (UTF-8 decoding, XID classification, escape sequences)
├── data-model.md        # Phase 1 output (Token, TokenType, Lexer entities)
├── quickstart.md        # Phase 1 output (Building and testing the lexer)
├── contracts/           # Phase 1 output (Lexer interface contract)
└── tasks.md             # Phase 2 output (created by /speckit.tasks command)
```

### Source Code (repository root)

```text
include/jsav/
└── lexer/
    ├── Lexer.hpp        # Main lexer interface
    ├── Token.hpp        # Token and TokenType definitions
    ├── Utf8Decoder.hpp  # UTF-8 decoding utilities
    └── XidClassification.hpp  # Unicode XID_Start/XID_Continue tables

src/jsav_Lib/
└── lexer/
    ├── Lexer.cpp        # Lexer implementation
    ├── Utf8Decoder.cpp  # UTF-8 validation and decoding
    └── XidClassification.cpp  # XID property lookup tables

test/
├── constexpr_tests.cpp  # Compile-time UTF-8 and escape sequence tests
└── tests.cpp            # Runtime lexer tests (Unicode identifiers, error handling)
```

**Structure Decision**: Using single project structure (Option 1) as the lexer is a core component of the jsav compiler library. New `lexer/` subdirectory added under `include/jsav/` and `src/jsav_Lib/` to maintain consistency with existing project organization.

## Complexity Tracking

> **Fill ONLY if Constitution Check has violations that must be justified**

| Violation | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|-------------------------------------|
| [e.g., 4th project] | [current need] | [why 3 projects insufficient] |
| [e.g., Repository pattern] | [specific problem] | [why direct DB access insufficient] |

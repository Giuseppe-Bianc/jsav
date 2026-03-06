# Implementation Plan: ASCII-Only Keyword Validation

**Branch**: `004-ascii-keyword-validation` | **Date**: 2026-03-06 | **Spec**: `specs/004-ascii-keyword-validation/spec.md`
**Input**: Feature specification from `specs/004-ascii-keyword-validation/spec.md`

**Note**: This template is filled in by the `/speckit.plan` command. See `.specify/templates/plan-template.md` for the execution workflow.

## Summary

**Primary Requirement**: Introduce a strict ASCII-only validation mechanism within the lexer's keyword-matching routine in order to prevent tokens containing non-ASCII homoglyph characters from being incorrectly interpreted as reserved language keywords.

During lexical analysis, the lexer typically determines whether a token corresponds to a reserved keyword (such as `if`, `for`, `class`, or similar constructs) by comparing the token's character sequence against a predefined set of language keywords. This comparison process must be augmented with an explicit character-set validation step that ensures the token under evaluation contains exclusively ASCII characters.

The purpose of this requirement is to prevent the use of visually deceptive Unicode homoglyphs—characters originating from other Unicode blocks that closely resemble standard ASCII letters—from being mistakenly interpreted as legitimate language keywords. These homoglyph characters may appear identical or nearly identical to ASCII characters in many fonts, but they differ at the Unicode code point level.

Without explicit ASCII validation, a malicious or careless source file could contain identifiers that visually mimic reserved keywords while technically being different strings. Such tokens could bypass keyword recognition safeguards or introduce ambiguity during code review, debugging, static analysis, or security auditing.

Examples of problematic tokens include:

- `fôr`, which visually resembles `for` but contains a non-ASCII character (`ô`)
- `clàss`, which visually resembles `class` but contains a non-ASCII character (`à`)
- `іf`, which visually resembles `if` but uses the Cyrillic small letter `і` instead of the ASCII `i`

To mitigate this class of issues, the lexer must enforce the following behavior:

1. Before attempting keyword comparison, the lexer must verify that every character within the candidate token belongs to the standard ASCII range.
2. If any character within the token falls outside the ASCII character set, the token must immediately be excluded from keyword matching.
3. Tokens containing non-ASCII characters must be treated strictly as identifiers or general tokens, regardless of whether their visual representation resembles a reserved keyword.
4. Keyword recognition must therefore occur **only** when the token is composed entirely of ASCII characters and matches a keyword string exactly.

This validation rule ensures deterministic and unambiguous keyword recognition while preventing Unicode homoglyph sequences from being used to imitate reserved language constructs.

**Technical Approach**: Introduce a pure, stateless predicate function `is_ascii_keyword_candidate` within the existing `resolve_identifier_or_keyword` execution path of the lexer. This predicate serves as a preliminary validation layer that determines whether a token representing an identifier-like sequence is eligible for keyword resolution.

The function performs a deterministic, character-by-character inspection of the token and verifies that every character falls within the Unicode range U+0021–U+007E. This interval corresponds to the set of printable ASCII characters excluding the space character, thereby ensuring that only tokens composed entirely of standard printable ASCII symbols are considered valid candidates for keyword table lookup.

The predicate is intentionally designed to be pure and stateless: its output depends exclusively on the characters contained in the input token, and it maintains no shared or persistent state across invocations. This guarantees deterministic behavior, simplifies reasoning about the lexer pipeline, and ensures that the function can be safely executed repeatedly without side effects or synchronization concerns.

Within the `resolve_identifier_or_keyword` control flow, the predicate operates as a lightweight filter prior to the keyword lookup stage. The processing sequence proceeds as follows: when the lexer encounters an identifier-like token, the token is forwarded to `resolve_identifier_or_keyword`, which invokes `is_ascii_keyword_candidate`. If the predicate confirms that all characters satisfy the ASCII constraint, the token proceeds to the existing keyword table lookup mechanism. Conversely, if any character falls outside the permitted ASCII range, the keyword lookup stage is bypassed entirely, and the token is emitted directly as an `Identifier`.

This behavior ensures that tokens containing non-ASCII characters cannot be misinterpreted as reserved keywords while still remaining valid identifiers within the language. The approach preserves compatibility with Unicode identifiers without compromising the determinism and simplicity of the keyword recognition mechanism.

From an implementation standpoint, the modification introduces no additional external dependencies and requires no changes to existing public APIs or lexer interfaces. The predicate performs a single linear pass over the token's character sequence, resulting in a time complexity of O(n) relative to the length of the token. Because the scan can terminate immediately upon encountering the first non-conforming character, the practical runtime overhead remains minimal.

## Technical Context

**Language/Version**: C++23 (MSVC 2022+ / GCC 13+ / Clang 16+)
**Primary Dependencies**: spdlog (logging), fmt (formatting fallback), CLI11 (CLI parsing), Catch2 (testing)
**Storage**: N/A (compiler frontend, no persistent storage)
**Testing**: Catch2 v3.13.0 (three targets: constexpr_tests, relaxed_constexpr_tests, tests)
**Target Platform**: Windows 10/11 (MSVC), Linux (GCC/Clang), macOS (Clang)
**Project Type**: Native compiler executable (CLI)
**Performance Goals**: ≤5% throughput regression on lexer tokenization (SC-004)
**Constraints**:

- Zero new runtime dependencies
- No changes to public API headers (`include/jsav/`)
- No changes to function signatures or token variants
- 100% branch coverage on modified code (SC-005)
- O(n) time complexity for validation predicate
**Scale/Scope**:

- Single function insertion (~10-20 lines)
- Modified function: `resolve_identifier_or_keyword` in lexer
- Test files: `test/tests.cpp` only (runtime tests)

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

### Gate Evaluation (Pre-Research)

| Principle | Status | Justification |
|-----------|--------|---------------|
| **I. Platform Independence** | ✅ PASS | Pure C++23 standard library; no platform-specific APIs; Unicode validation uses standard character comparison |
| **II. VS 2026 Compatibility** | ✅ PASS | C++23 `char8_t` and Unicode iteration fully supported by MSVC 2022+; no experimental features |
| **III. C++ Core Guidelines** | ✅ PASS | Pure function (const-correct), no raw pointers, no dynamic allocation, no exceptions, O(n) complexity |
| **IV. Test-Driven Development** | ✅ PASS | All code will be test-first (Red-Green-Refactor); tests in `test/tests.cpp` with Catch2; 100% branch coverage required |
| **V. Dependency Management** | ✅ PASS | Zero new dependencies; uses only C++23 stdlib; no external Unicode libraries required |
| **VI. Documentation Standards** | ✅ PASS | Doxygen comments for public functions; markdownlint compliance for all docs |

**Overall Gate Status**: ✅ PASS — No violations. Proceed to Phase 0.

### Gate Re-Evaluation (Post-Design)

| Principle | Status | Verification |
|-----------|--------|--------------|
| **I. Platform Independence** | ✅ PASS | Confirmed: `data-model.md` specifies pure C++23; `contracts/` uses no platform APIs |
| **II. VS 2026 Compatibility** | ✅ PASS | Confirmed: `constexpr` and `std::string_view` fully supported; quickstart examples compile on MSVC |
| **III. C++ Core Guidelines** | ✅ PASS | Confirmed: Function is `[[nodiscard]] constexpr`, `noexcept`, no heap allocation, no exceptions |
| **IV. Test-Driven Development** | ✅ PASS | Confirmed: All tests specified in `data-model.md` Tier 1/2/3; tests in `test/tests.cpp` only |
| **V. Dependency Management** | ✅ PASS | Confirmed: Zero new dependencies in `contracts/` or `quickstart.md` |
| **VI. Documentation Standards** | ✅ PASS | Confirmed: All artifacts created (`research.md`, `data-model.md`, `contracts/`, `quickstart.md`) follow markdownlint |

**Overall Gate Status**: ✅ PASS — No violations introduced during Phase 1. Proceed to Phase 2.

## Project Structure

### Documentation (this feature)

```text
specs/004-ascii-keyword-validation/
├── plan.md              # This file (implementation plan)
├── research.md          # Phase 0 output (research & decisions)
├── data-model.md        # Phase 1 output (entity definitions)
├── quickstart.md        # Phase 1 output (usage guide)
└── contracts/           # Phase 1 output (interface contracts)
```

### Source Code (repository root)

```text
src/jsav_Lib/lexer/
├── Lexer.cpp            # Modified: add is_ascii_keyword_candidate predicate call
└── (other lexer files)

test/
└── tests.cpp            # New: all runtime tests for ASCII validation feature
```

**Structure Decision**: Single project structure (compiler library). Modification is surgical:

- **Implementation**: `src/jsav_Lib/lexer/Lexer.cpp` — add predicate function and integrate into `resolve_identifier_or_keyword`
- **Tests**: `test/tests.cpp` — comprehensive test coverage (unit + integration + benchmark)
- **No new files or directories** beyond documentation in `specs/`

## Complexity Tracking

> **Status**: Not applicable — Constitution Check passed with zero violations.

No complexity violations. Feature maintains existing architecture with surgical insertion of single predicate function.

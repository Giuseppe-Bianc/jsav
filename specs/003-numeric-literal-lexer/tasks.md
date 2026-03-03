# Tasks: Complete Numeric Literal Recognition in the Lexer

**Input**: Design documents from `/specs/003-numeric-literal-lexer/`
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/numeric-token-contract.md

**Tests**: Included — the project adopts the TDD workflow (Red-Green-Refactor) as per Constitution Principle IV.

**Organization**: Tasks organized by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can be executed in parallel (different files, no dependencies)
- **[Story]**: Which user story the task belongs to (e.g., US1, US2, US3)
- Exact file paths included in descriptions

## Path Conventions

- **Header**: `include/jsav/lexer/Lexer.hpp`
- **Source**: `src/jsav_Lib/lexer/Lexer.cpp`
- **Runtime tests**: `test/tests.cpp`
- **Constexpr tests**: `test/constexpr_tests.cpp`

---

## Phase 1: Setup

**Purpose**: Branch preparation and existing build verification

- [ ] T001 Verify that the branch `003-numeric-literal-lexer` is active and that the build compiles without errors with `cmake -S . -B ./build -Djsav_ENABLE_IPO:BOOL=OFF -Djsav_ENABLE_CPPCHECK:BOOL=OFF -DFMT_PEDANTIC:BOOL=ON`
  - **Note**: AddressSanitizer is temporarily disabled (`-Djsav_ENABLE_SANITIZER_ADDRESS:BOOL=OFF` in command above) due to known MSVC compatibility issues. Re-enable when resolved.
  - **Constitution Principle III**: This is a temporary deviation. Track resolution in project issue tracker.
- [ ] T002 Run the existing test suite (`ninja tests relaxed_constexpr_tests && ctest -R "unittests|relaxed_constexpr" --output-on-failure`) and verify that all tests pass (regression baseline)

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Private helper declarations in the header and modification of `next_token()` entry point — prerequisites that block all user stories

**⚠️ CRITICAL**: No work on user stories can begin before completion of this phase

- [ ] T003 Update the doccomment of `scan_numeric_literal()` in `include/jsav/lexer/Lexer.hpp` to reflect the new trailing-dot behavior:
  - **Remove** reference to old `Numeric + Dot` split (lines 38-40 of `Lexer.hpp`)
  - **Replace with**: "Trailing dot is included in the numeric token (e.g., `3.` → `Numeric("3.")`)"
  - **Reference**: research.md R1 — "Trailing-dot behavior change: impact on codebase"
- [ ] T004 [P] Declare the private helper method `void try_scan_exponent()` in `include/jsav/lexer/Lexer.hpp` with doccomment for G2 (as per data-model.md Method Signatures)
- [ ] T005 [P] Declare the private helper method `void try_scan_type_suffix()` in `include/jsav/lexer/Lexer.hpp` with doccomment for G3 (as per data-model.md Method Signatures)
- [ ] T006 [P] Declare the private helper method `[[nodiscard]] bool match_width_suffix()` in `include/jsav/lexer/Lexer.hpp` with doccomment (as per data-model.md Method Signatures)
- [ ] T007 Modify `next_token()` in `src/jsav_Lib/lexer/Lexer.cpp` to add the leading-dot branch: if `peek_byte() == '.'` and `std::isdigit(peek_byte(1))` then invoke `scan_numeric_literal(start)` before `scan_operator_or_punctuation()` as per research R2
- [ ] T008 Format the modified files with `clang-format -i include/jsav/lexer/Lexer.hpp src/jsav_Lib/lexer/Lexer.cpp`
- [ ] T009 Build and verify that existing tests continue to pass (regression baseline)
- [ ] T010 [P] [FR-027] Verify absence of regex usage in entire lexer module (both `src/` and `include/`):
  - Verify that `#include <regex>` is NOT present in any lexer file
  - Verify that `std::regex`, `std::regex_match`, `std::regex_search`, `std::regex_replace` are NOT used
  - Method (cross-platform): Use `grep` to search for regex usage across lexer module:
    ```bash
    grep -rn "include <regex>" src/jsav_Lib/lexer/ include/jsav/lexer/
    grep -rn "std::regex" src/jsav_Lib/lexer/ include/jsav/lexer/
    ```
  - Both commands MUST return no results (empty output)
  - **Alternative (PowerShell)**:
    ```powershell
    Select-String -Pattern "std::regex" -Path "src/jsav_Lib/lexer/", "include/jsav/lexer/"
    ```
  - **Scope**: Verify all files in `src/jsav_Lib/lexer/` and `include/jsav/lexer/` directories

**Checkpoint**: Foundation ready — user story implementation can begin

---

## Phase 3: User Story 1 — Recognizing basic integers and decimals (Priority: P1) 🎯 MVP

**Goal**: The lexer correctly recognizes simple integers (`0`, `1`, `42`, `007`), decimals with integer and fractional parts (`1.0`, `3.14`), decimals with trailing dot (`3.`, `42.`), and numbers with only fractional part (`.5`, `.14`, `.0`). Token text is preserved without normalization.

**Independent Test**: Provide the lexer with strings containing each basic numeric form and verify `Numeric` type, exact text, and position coordinates.

### Tests for User Story 1 ⚠️

> **NOTE: Write these tests BEFORE implementation. Verify that they FAIL (RED phase) before implementing.**

- [ ] T011 [P] [US1] Write TEST_CASE for simple integers (`0`, `1`, `42`, `007`) verifying `TokenKind::Numeric` and exact text in `test/tests.cpp`; include SECTION that verifies position coordinates (`span.start`, `span.end`, line, column) on at least 5 representative tokens (FR-025)
- [ ] T012 [P] [US1] Write TEST_CASE for decimals with integer and fractional parts (`1.0`, `3.14`, `0.5`) verifying exact text in `test/tests.cpp`
- [ ] T013 [P] [US1] Write TEST_CASE for decimals with trailing dot (`3.`, `42.`) verifying that the dot is included in the `Numeric("3.")` token in `test/tests.cpp`
- [ ] T014 [P] [US1] Write TEST_CASE for numbers with only fractional part (`.5`, `.14`, `.0`) verifying `Numeric(".5")` token in `test/tests.cpp`
- [ ] T015 [P] [US1] Write TEST_CASE for edge cases: isolated dot (`.`) → not Numeric, dot followed by non-digit (`.abc`) → not Numeric in `test/tests.cpp`
- [ ] T016 [P] [US1] Write constexpr tests in `test/constexpr_tests.cpp` BEFORE implementation (Constitution IV: TDD test-first):
  - **Required constexpr methods**: `scan_numeric_literal()`, `try_scan_exponent()`, `try_scan_type_suffix()`, `match_width_suffix()`
  - Define `consteval` functions that invoke the lexer on basic inputs (`42`, `3.14`, `3.`, `.5`)
  - Use `STATIC_REQUIRE` to verify at compile-time that the produced token has type `TokenKind::Numeric` and exact text
  - **TDD Workflow**: Write BEFORE implementation, verify that it does NOT compile (RED because lexer is not yet constexpr), implement by making methods constexpr, verify that it compiles (GREEN)

### RED Phase Verification for User Story 1

- [ ] T016b [US1] [TDD RED PHASE] Verify that constexpr tests T016 do NOT compile (RED phase):
  - Run `ninja constexpr_tests` and verify compilation FAILS
  - Run `ninja tests relaxed_constexpr_tests && ctest -R "unittests" --output-on-failure` and verify runtime tests FAIL
  - Document failure output (expected: lexer methods not yet constexpr-compatible)
  - **DO NOT proceed to T017 until RED phase confirmed**

### Implementation for User Story 1

- [ ] T017 [US1] Rewrite the body of `scan_numeric_literal()` in `src/jsav_Lib/lexer/Lexer.cpp` to implement group G1 branch A: consumption of integer digits, optional consumption of decimal point and fractional digits — the trailing dot (`3.`) MUST be included in the token as per FR-003 and R1
- [ ] T018 [US1] Implement in `scan_numeric_literal()` group G1 branch B in `src/jsav_Lib/lexer/Lexer.cpp`: when the entry point is a dot (from `next_token()`), consume the dot and subsequent fractional digits
- [ ] T019 [US1] Remove/replace the legacy comment on old trailing-dot behavior in `src/jsav_Lib/lexer/Lexer.cpp` (lines ~245-248) as per R1
- [ ] T020 [US1] Format with `clang-format -i src/jsav_Lib/lexer/Lexer.cpp test/tests.cpp test/constexpr_tests.cpp`
- [ ] T021 [US1] Run `ninja tests relaxed_constexpr_tests && ctest -R "unittests|relaxed_constexpr" --output-on-failure` — all US1 tests must pass and no regression

**Checkpoint**: User Story 1 complete — base numbers correctly recognized, independently tested

---

## Phase 4: User Story 2 — Scientific notation recognition (Priority: P2)

**Goal**: The lexer recognizes the exponent group G2 (`[eE][+-]?\d+`) immediately after G1. If the exponent is incomplete (`1e`, `1e+`), the marker and sign are not consumed.

**Independent Test**: Provide strings with valid and invalid scientific notation, verify produced tokens.

### Tests for User Story 2 ⚠️

> **NOTE: Write these tests BEFORE implementation. Verify that they FAIL (RED phase) before implementing.**

- [ ] T022 [P] [US2] Write TEST_CASE for valid exponents (`1e10`, `3.14E+2`, `2.5e-3`, `.5E10`) verifying single Numeric token in `test/tests.cpp`
- [ ] T023 [P] [US2] Write TEST_CASE for invalid exponents: `1e` → `Numeric("1")` + token `e`; `1e+` → `Numeric("1")` + `e` + `+`; `1E-` → `Numeric("1")` + `E` + `-` in `test/tests.cpp`
- [ ] T024 [P] [US2] Write constexpr tests in `test/constexpr_tests.cpp` BEFORE implementation (Constitution IV: TDD test-first):
  - **Required constexpr methods**: `scan_numeric_literal()`, `try_scan_exponent()`
  - Define `consteval` functions that verify valid scientific notation (`1e10`, `3.14E+2`, `2.5e-3`) at compile-time
  - Use `STATIC_REQUIRE` to verify that incomplete exponents (`1e`, `1e+`, `1E-`) produce separate tokens
  - **TDD Workflow**: Write BEFORE implementation, verify RED, implement `try_scan_exponent()` as `constexpr`, verify GREEN

### RED Phase Verification for User Story 2

- [ ] T024b [US2] [TDD RED PHASE] Verify that tests T022-T024 fail before implementation:
  - Run `ninja tests relaxed_constexpr_tests && ctest -R "unittests" --output-on-failure`
  - Verify US2-specific tests FAIL (expected: `try_scan_exponent()` not yet implemented)
  - For constexpr tests: verify `ninja constexpr_tests` compilation status (may fail if methods not constexpr)
  - Document which tests fail and why (expected failure modes)
  - **DO NOT proceed to T025 until RED phase confirmed**

### Implementation for User Story 2

- [ ] T025 [US2] Implement `try_scan_exponent()` in `src/jsav_Lib/lexer/Lexer.cpp` with save/restore pattern for `m_pos` and `m_column`: attempt consumption of `e`/`E`, optional `+`/`-`, mandatory digits; complete rollback if digits absent as per R4
- [ ] T026 [US2] Add the call to `try_scan_exponent()` at the end of G1 in `scan_numeric_literal()` in `src/jsav_Lib/lexer/Lexer.cpp`
- [ ] T027 [US2] Remove the old unconditional exponent consumption code (if still present) in `src/jsav_Lib/lexer/Lexer.cpp`
- [ ] T028 [US2] Format with `clang-format -i src/jsav_Lib/lexer/Lexer.cpp test/tests.cpp test/constexpr_tests.cpp`
- [ ] T029 [US2] Run `ninja tests relaxed_constexpr_tests && ctest -R "unittests|relaxed_constexpr" --output-on-failure` — all US1+US2 tests must pass

**Checkpoint**: User Story 2 complete — scientific notation recognized, incomplete exponents handled correctly

---

## Phase 5: User Story 3 — Type suffix recognition (Priority: P3)

**Goal**: The lexer recognizes type suffixes G3 (`d`/`D`, `f`/`F`, `u`/`U` with optional width, `i`/`I` with mandatory width). Maximal munch: compound suffixes have priority. `f`/`F` does not form compounds. `i`/`I` alone is not a suffix.

**Independent Test**: Provide digits followed by all allowed suffixes and verify correct tokenization.

### Tests for User Story 3 ⚠️

> **NOTE: Write these tests BEFORE implementation. Verify that they FAIL (RED phase) before implementing.**

- [ ] T030 [P] [US3] Write TEST_CASE for **valid** single-character suffixes (`1.0F`, `1.0f`, `10d`, `10D`) verifying text in Numeric token in `test/tests.cpp`; and **invalid** single-character suffixes (`42u`, `42U`) verifying they produce `Numeric("42")` + separate token `u`/`U`
- [ ] T031 [P] [US3] Write TEST_CASE for valid compound suffixes (`255u8`, `1000i32`, `50i16`, `50I16`, `100U32`) verifying text in Numeric token in `test/tests.cpp`
- [ ] T032 [P] [US3] Write TEST_CASE for suffix edge cases: `1i` → `Numeric("1")` + `i`; `1u64` → `Numeric("1u64")`; `5f32` → `Numeric("5f")` + `32`; `1u` → `Numeric("1")` + `u`; `1U` → `Numeric("1")` + `U`; `1I` → `Numeric("1")` + `I` in `test/tests.cpp`
- [ ] T033 [P] [US3] Write constexpr tests in `test/constexpr_tests.cpp` BEFORE implementation (Constitution IV: TDD test-first):
  - **Required constexpr methods**: `scan_numeric_literal()`, `try_scan_type_suffix()`, `match_width_suffix()`
  - Define `consteval` functions that verify valid suffixes (`1.0F`, `255u8`, `1000i32`) at compile-time
  - Use `STATIC_REQUIRE` for edge cases (`1i`, `42u`, `1u64`, `5f32`) that produce separate tokens or Numeric with complete text
  - **TDD Workflow**: Write BEFORE implementation, verify RED, implement `try_scan_type_suffix()` and `match_width_suffix()` as `constexpr`, verify GREEN

### RED Phase Verification for User Story 3

- [ ] T033b [US3] [TDD RED PHASE] Verify that tests T030-T033 fail before implementation:
  - Run `ninja tests relaxed_constexpr_tests && ctest -R "unittests" --output-on-failure`
  - Verify US3-specific tests FAIL (expected: `try_scan_type_suffix()` and `match_width_suffix()` not yet implemented)
  - For constexpr tests: verify compilation status with `ninja constexpr_tests`
  - Document failure output, confirming tests expose missing functionality
  - **DO NOT proceed to T034 until RED phase confirmed**

### Implementation for User Story 3

- [ ] T034 [US3] Implement `match_width_suffix()` in `src/jsav_Lib/lexer/Lexer.cpp`: comparison with priority `32` → `16` → `8` with advance if match, return false without advance if no match as per FR-017 and R6
- [ ] T035 [US3] Implement `try_scan_type_suffix()` in `src/jsav_Lib/lexer/Lexer.cpp`: recognition of `d`/`D` and `f`/`F` as singles; `u`/`U` with width attempt (bare if width invalid); `i`/`I` with mandatory width (do not consume if width absent) as per FR-011–FR-017 and R6
- [ ] T036 [US3] Add the call to `try_scan_type_suffix()` after `try_scan_exponent()` in `scan_numeric_literal()` in `src/jsav_Lib/lexer/Lexer.cpp`
- [ ] T037 [US3] Format with `clang-format -i src/jsav_Lib/lexer/Lexer.cpp test/tests.cpp test/constexpr_tests.cpp`
- [ ] T038 [US3] Run `ninja tests relaxed_constexpr_tests && ctest -R "unittests|relaxed_constexpr" --output-on-failure` — all US1+US2+US3 tests must pass

**Checkpoint**: User Story 3 complete — type suffixes recognized with maximal munch

---

## Phase 6: User Story 4 — Complete combined G1-G2-G3 pattern (Priority: P4)

**Goal**: The lexer recognizes the combination of the three groups in order G1 → G2 → G3, contiguous and without separators, producing a single Numeric token.

**Independent Test**: Provide strings combining all three groups and verify the resulting token.

### Tests for User Story 4 ⚠️

> **NOTE: Write these tests BEFORE implementation. Verify that they FAIL (RED phase) before implementing.**

- [ ] T039 [P] [US4] Write TEST_CASE for G1+G2+G3 combinations: `1.5e10f` → `Numeric("1.5e10f")`, `2.0E-3d` → `Numeric("2.0E-3d")`, `1e2u16` → `Numeric("1e2u16")`, `.5e1i32` → `Numeric(".5e1i32")` in `test/tests.cpp`
- [ ] T040 [P] [US4] Write constexpr tests in `test/constexpr_tests.cpp` BEFORE implementation (Constitution IV: TDD test-first):
  - **Required constexpr methods**: `scan_numeric_literal()`, `try_scan_exponent()`, `try_scan_type_suffix()`, `match_width_suffix()`
  - Define `consteval` functions that verify complete combinations (`1.5e10f`, `2.0E-3d`, `1e2u16`, `.5e1i32`) at compile-time
  - Use `STATIC_REQUIRE` to verify that G1→G2→G3 produce single token with exact text
  - **TDD Workflow**: Write BEFORE final verification, verify that the complete pattern is `constexpr`-compatible
- [ ] T041 [P] [US4] Write TEST_CASE for group optionality (FR-019): verify that only G1 is mandatory:
  - `42` → `Numeric("42")` (G1 only)
  - `42e10` → `Numeric("42e10")` (G1 + G2)
  - `42u` → `Numeric("42")` + `u` (G1 + invalid suffix, `u` alone is not consumed)
  - `42e10u` → `Numeric("42e10")` + `u` (G1 + G2 + invalid suffix)
  - `42d` → `Numeric("42d")` (G1 + valid G3, `d` is valid single suffix)
  - `42e10d` → `Numeric("42e10d")` (G1 + G2 + valid G3)
  - Verify that G2 and G3 are optional but G1 is required

### RED Phase Verification for User Story 4

- [ ] T041b [US4] [TDD RED PHASE] Verify that tests T039-T041 fail before implementation:
  - Run `ninja tests relaxed_constexpr_tests && ctest -R "unittests" --output-on-failure`
  - Verify US4 combined pattern tests FAIL (expected: G1→G2→G3 integration incomplete)
  - Document which specific combinations fail (`1.5e10f`, `2.0E-3d`, etc.)
  - **DO NOT proceed to T042 until RED phase confirmed**

### Implementation for User Story 4

- [ ] T042 [US4] Verify in `scan_numeric_literal()` in `src/jsav_Lib/lexer/Lexer.cpp` that the flow G1 → `try_scan_exponent()` → `try_scan_type_suffix()` is correctly concatenated and produces a single token for combinations like `1.5e10f` and `1e2u16`
- [ ] T043 [US4] Run `ninja tests relaxed_constexpr_tests && ctest -R "unittests|relaxed_constexpr" --output-on-failure` — all US1+US2+US3+US4 tests must pass

**Checkpoint**: User Story 4 complete — G1→G2→G3 pattern working end-to-end

---

## Phase 7: User Story 5 — Maximal munch rule and token boundaries (Priority: P5)

**Goal**: The lexer applies maximal munch, `+`/`-` are part of the token only within G2, spaces/operators/delimiters/EOF/non-ASCII correctly terminate the literal. Newline characters (`\n`, `\r`, `\r\n`) unconditionally terminate the numeric token even with incomplete G1→G2→G3 pattern (FR-028).

**Independent Test**: Provide input with numbers adjacent to other tokens and verify correct separation. Provide input with interspersed newlines (e.g., `"42\n10"`) and verify that the newline terminates the first token and the second number starts a new token.

### Tests for User Story 5 ⚠️

> **NOTE: Write these tests BEFORE implementation. Verify that they FAIL (RED phase) before implementing.**

- [ ] T044 [P] [US5] Write TEST_CASE for token boundaries: `-42` → `-` + `Numeric("42")`; `42 u8` → `Numeric("42")` + `u8`; `3.14+2` → `Numeric("3.14")` + `+` + `Numeric("2")`; `1e2+3` → `Numeric("1e2")` + `+` + `Numeric("3")` in `test/tests.cpp`
- [ ] T045 [P] [US5] Write TEST_CASE for termination on non-ASCII and end of file in `test/tests.cpp`
- [ ] T046 [P] [US5] Write TEST_CASE for newline termination (FR-028): `"42\n10"` → `Numeric("42")` + newline token + `Numeric("10")`; `"3.14\r\n2.5"` → `Numeric("3.14")` + newline token + `Numeric("2.5")`; `"1e2\r3"` → `Numeric("1e2")` + newline token + `Numeric("3")` in `test/tests.cpp`
- [ ] T046b [P] [US5] [FR-028] Write dedicated TEST_CASE for comprehensive newline termination edge cases in `test/tests.cpp`:
  - **Incomplete G1 + newline**: `"3.\n10"` → `Numeric("3.")` + newline + `Numeric("10")` (trailing dot before newline)
  - **Incomplete G2 + newline**: `"1e\n10"` → `Numeric("1")` + `e` + newline + `Numeric("10")` (incomplete exponent before newline)
  - **Incomplete G2+sign + newline**: `"1e+\n5"` → `Numeric("1")` + `e` + `+` + newline + `Numeric("5")` (exponent sign before newline)
  - **Incomplete G3 + newline**: `"42u\n10"` → `Numeric("42")` + `u` + newline + `Numeric("10")` (invalid suffix before newline)
  - **Complete G1+G2 + newline**: `"1e10\n5"` → `Numeric("1e10")` + newline + `Numeric("5")` (complete exponent before newline)
  - **Complete G1+G2+G3 + newline**: `"1.5e10f\n5"` → `Numeric("1.5e10f")` + newline + `Numeric("5")` (complete pattern before newline)
  - **Multiple consecutive newlines**: `"42\n\n10"` → `Numeric("42")` + newline + newline + `Numeric("10")`
  - **Newline at EOF**: `"42\n"` → `Numeric("42")` + newline + EOF
  - **CR-only newline**: `"42\r10"` → `Numeric("42")` + carriage return + `Numeric("10")` (Mac-style line ending)
- [ ] T047 [P] [US5] Write constexpr tests in `test/constexpr_tests.cpp` BEFORE implementation (Constitution IV: TDD test-first):
  - **Required constexpr methods**: `scan_numeric_literal()`, `try_scan_exponent()`, `try_scan_type_suffix()`, `match_width_suffix()`
  - Define `consteval` functions that verify token boundaries (`-42` → `-` + `42`, `42 u8` → `42` + `u8`) at compile-time
  - Use `STATIC_REQUIRE` for maximal munch and newline termination
  - **TDD Workflow**: Write BEFORE final verification, verify that the boundary logic is `constexpr`-compatible

### RED Phase Verification for User Story 5

- [ ] T047b [US5] [TDD RED PHASE] Verify that tests T044-T047 fail before implementation:
  - Run `ninja tests relaxed_constexpr_tests && ctest -R "unittests" --output-on-failure`
  - Verify US5 boundary tests FAIL (expected: maximal munch and newline termination not yet verified)
  - Document failure output for token boundary cases (`-42`, `42 u8`, newline termination)
  - **DO NOT proceed to T048 until RED phase confirmed**

### Implementation for User Story 5

- [ ] T048 [US5] Verify in `scan_numeric_literal()` in `src/jsav_Lib/lexer/Lexer.cpp` that token termination occurs correctly at the first non-consumable character (spaces, operators, delimiters, EOF, non-ASCII) and that `+`/`-` are not consumed outside G2 context
- [ ] T048b [US5] [FR-028] Verify explicit newline handling in `scan_numeric_literal()` in `src/jsav_Lib/lexer/Lexer.cpp`:
  - The character `\n` (LF) MUST unconditionally terminate the current numeric token
  - The character `\r` (CR) MUST unconditionally terminate the current numeric token
  - The sequence `\r\n` (CRLF) MUST terminate the token after `\r`, leaving `\n` for the next token
  - The newline character MUST NOT be consumed by the `TokenKind::Numeric` token
  - The newline character MUST remain in the input stream for the next token
  - Implement explicit check: if `peek_byte() == '\n'` or `peek_byte() == '\r'`, immediately interrupt literal consumption even if G1→G2→G3 would be continuable
  - Verify that the method comment/doccomment explicitly documents this behavior
- [ ] T049 [US5] Verify in `scan_numeric_literal()` in `src/jsav_Lib/lexer/Lexer.cpp` that the characters `\n`, `\r` unconditionally terminate the numeric token even if G1→G2→G3 would be continuable; the newline character MUST NOT be consumed by the Numeric token but must remain in the stream for the next token as per FR-028
- [ ] T050 [US5] Run `ninja tests relaxed_constexpr_tests && ctest -R "unittests|relaxed_constexpr" --output-on-failure` — all US1–US5 tests must pass

**Checkpoint**: User Story 5 complete — maximal munch and token boundaries correct

---

## Phase 8: Polish & Cross-Cutting Concerns

**Purpose**: Complexity verification, complete regression, final formatting, and documentation

- [ ] T051 [P] Run Lizard analysis (`cmake --build build --target lizard`) and verify that all modified methods respect CCN ≤ 15 and length ≤ 100 lines (Constitution Principle III)
- [ ] T052 [P] Write performance test in `test/tests.cpp` that defines the O(n) criterion BEFORE profiling implementation (Constitution IV: Test-First):
  - Include `#include <jsav/jsav.hpp>` in the test file
    - **Constitution V Note**: `vnd::Timer` is an internal component of the jsav project (`include/jsavCore/timer/Timer.hpp`), exposed via the master header `jsav/jsav.hpp`. **NOT** an external dependency. Approved per Constitution V (Dependency Management).
  - Define TEST_CASE `"Lexer_scanNumericLiteral_scalesLinearly"` that:
    - Generates numeric literals of length 10, 100, 500, 1000 characters (decimal digits only)
    - Measures scan time for each length using `vnd::Timer`
    - Verifies criterion: `time(1000) / time(10) ≤ 150` (scaling factor 150× for O(n) complexity)
  - **This test MUST fail initially** (RED phase) because profiling is not yet implemented
- [ ] T053 Implement actual profiling in `scan_numeric_literal()` to make performance test T052 pass:
  - Use `vnd::Timer` (internal component exposed by `jsav/jsav.hpp`, Constitution V compliant) to measure scan times on inputs of various lengths
  - Document results in `specs/003-numeric-literal-lexer/performance-report.md`:
    - Measured times for each length (10, 100, 500, 1000 characters)
    - Scaling factor calculation `time(1000) / time(10)`
    - Verdict: PASS if scaling ≤ 150×, FAIL otherwise
  - **Acceptance criterion**: Performance test T052 MUST pass (GREEN phase)
- [ ] T054 [P] Run the complete regression test suite (`ctest --output-on-failure`) and verify that ALL pre-existing tests pass without regressions (SC-007)
- [ ] T055 Format all modified files with `clang-format -i src/jsav_Lib/lexer/Lexer.cpp include/jsav/lexer/Lexer.hpp test/tests.cpp test/constexpr_tests.cpp`
- [ ] T056 Run quickstart.md validation: instantiate the quick smoke test with newlines (`jsv::Lexer lex{"3.14e+2f\n.5\r1e 42u8\r\n1i", "test.jsav"}`) and verify that newlines correctly terminate numeric tokens producing separate tokens for numbers on different lines
- [ ] T057 Final code review: verify that comments and doccomments are updated in `include/jsav/lexer/Lexer.hpp` and `src/jsav_Lib/lexer/Lexer.cpp`

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies — can begin immediately
- **Foundational (Phase 2)**: Depends on completion of Phase 1 — **BLOCKS** all user stories
- **User Story 1 (Phase 3)**: Depends on Phase 2 — implements G1, prerequisite for US2/US3/US4
- **User Story 2 (Phase 4)**: Depends on Phase 3 (G1 must exist to attach G2)
- **User Story 3 (Phase 5)**: Depends on Phase 3 (G1 must exist to attach G3); can proceed in parallel with US2
- **User Story 4 (Phase 6)**: Depends on Phase 4 AND Phase 5 (requires G1+G2+G3 all implemented)
- **User Story 5 (Phase 7)**: Depends on Phase 6 (verifies boundaries on entire pattern)
- **Polish (Phase 8)**: Depends on completion of all desired user stories

### User Story Dependencies

```text
Phase 1 (Setup)
    │
    ▼
Phase 2 (Foundational) ──── BLOCKS ALL ────┐
    │                                        │
    ▼                                        │
Phase 3 (US1: G1 base) ◄────────────────────┘
    │
    ├─────────────┐
    ▼             ▼
Phase 4 (US2)  Phase 5 (US3)    ← can proceed in parallel
    │             │
    └──────┬──────┘
           ▼
    Phase 6 (US4: G1+G2+G3)
           │
           ▼
    Phase 7 (US5: boundaries)
           │
           ▼
    Phase 8 (Polish)
```

### Within Each User Story

1. Tests MUST be written and MUST FAIL before implementation (TDD Red-Green-Refactor)
2. Implementation of helper methods before the orchestrator
3. Integration: call to helper in the orchestrator
4. Format → Build → Green test → Checkpoint

### Parallel Opportunities

- **Phase 2**: T004, T005, T006, T010 can be executed in parallel (helper declarations and regex verification in different header sections)
- **Phase 3**: T011–T016 can be executed in parallel (tests in different files or independent sections)
- **Phase 4 and Phase 5**: Can proceed in parallel (US2 modifies `try_scan_exponent`, US3 modifies `try_scan_type_suffix` — separate methods)
- **Phase 6**: T039, T040, T041 can be executed in parallel
- **Phase 7**: T044, T045, T046, T047 can be executed in parallel
- **Phase 8**: T051, T052, T054 can be executed in parallel

---

## Parallel Example: User Story 1

```text
# Tests in parallel (all in different files or independent sections):
T011: TEST_CASE simple integers                    → test/tests.cpp
T012: TEST_CASE decimals with fractional part      → test/tests.cpp
T013: TEST_CASE decimals with trailing dot         → test/tests.cpp
T014: TEST_CASE numbers with only fractional part  → test/tests.cpp
T015: TEST_CASE dot edge cases                     → test/tests.cpp
T016: STATIC_REQUIRE constexpr tests               → test/constexpr_tests.cpp
```

## Parallel Example: User Story 2 and 3 in Parallel

```text
# US2 and US3 can proceed in parallel after US1:
Developer A (US2): T022-T029 → try_scan_exponent() in Lexer.cpp
Developer B (US3): T030-T038 → try_scan_type_suffix() + match_width_suffix() in Lexer.cpp
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Setup
2. Complete Phase 2: Foundational (CRITICAL — blocks all stories)
3. Complete Phase 3: User Story 1 — G1 base
4. **STOP and VALIDATE**: Test US1 independently
5. The lexer already correctly recognizes all base numbers

### Incremental Delivery

1. Setup + Foundational → Infrastructure ready
2. User Story 1 (G1) → Independent tests → **Working MVP** 🎯
3. User Story 2 (G2) → Independent tests → Scientific notation added
4. User Story 3 (G3) → Independent tests → Type suffixes added
5. User Story 4 (G1+G2+G3) → Independent tests → Combined pattern validated
6. User Story 5 → Independent tests → Token boundaries validated
7. Polish → Complexity, regression, documentation

### Parallel Team Strategy

With two developers after US1 completion:

1. Team completes Setup + Foundational + US1 together
2. After US1 completed:
   - **Developer A**: US2 (try_scan_exponent)
   - **Developer B**: US3 (try_scan_type_suffix + match_width_suffix)
3. US4 and US5 sequential after merging US2+US3
4. Final Polish

---

## Notes

- [P] tasks operate on different files or independent sections, without conflicts
- The [Story] label maps each task to the user story for traceability
- Each user story is independently testable at its checkpoint
- **TDD Workflow**: RED tests → implementation → GREEN tests → refactor
- Commit after each task or logical group
- The files involved are 5: `Lexer.hpp`, `Lexer.cpp`, `tests.cpp`, `constexpr_tests.cpp`, `performance-report.md` (generated)
- **Total tasks**: 62 (T001–T057, including 5 new RED phase verification tasks: T016b, T024b, T033b, T041b, T047b)
- **Constitution Principle IV**: Constexpr tests MUST follow the same test-first workflow as runtime tests — write FIRST, verify RED (does not compile), implement constexpr, verify GREEN
- **RED Phase Verification**: Tasks T016b, T024b, T033b, T041b, T047b explicitly verify test failure BEFORE implementation to ensure TDD discipline

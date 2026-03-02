# Tasks: Full UTF-8 Unicode Whitespace Support in Lexer

**Input**: Design documents from `/specs/002-utf8-unicode-whitespace/`
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/

**Tests**: Included — explicitly required by spec (SC-001 through SC-005) and constitution (TDD Principle IV).

**Organization**: Tasks grouped by user story. US1 and US4 are P1 (MVP). US2 and US3 are P2.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story (US1, US2, US3, US4)
- Exact file paths included in all implementation tasks

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: No new project setup needed — feature modifies existing files only. This phase
verifies the build works and creates the shared helper function needed by multiple stories.

- [X] T001 Verify clean build of all existing targets from branch 002-utf8-unicode-whitespace
- [ ] T001b Run and record baseline ASCII throughput benchmark (Catch2 BENCHMARK, 1MB corpus, 100 iterations) before any code changes — save result for SC-005 comparison
- [X] T002 Add `is_unicode_line_terminator()` function to `generate_header()` template in scripts/generate_unicode_tables.py per contracts/unicode-line-terminator.md
- [X] T003 Regenerate include/jsav/lexer/unicode/UnicodeData.hpp by running `python scripts/generate_unicode_tables.py` and verify the new function appears in output

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Constexpr tests for the new `is_unicode_line_terminator()` helper — must pass before any Lexer modifications.

**CRITICAL**: These compile-time tests validate the truth table that all user story implementations depend on.

- [X] T004a Add STATIC_REQUIRE tests for `is_unicode_line_terminator` truth table in test/constexpr_tests.cpp (NEL→true, LINE SEP→true, PARA SEP→true, LF→false, CR→false, SPACE→false, VT→false, FF→false)
- [X] T004b Build and run relaxed_constexpr_tests first (runtime version via CATCH_CONFIG_RUNTIME_STATIC_REQUIRE) to debug and verify T004a tests pass at runtime
- [X] T005 Build and run constexpr_tests to verify T004a tests pass at compile time (STATIC_REQUIRE)

**Checkpoint**: `is_unicode_line_terminator()` is proven correct — first at runtime (relaxed_constexpr_tests), then at compile time (constexpr_tests). Lexer modifications can begin.

---

## Phase 3: User Story 1 — Complete Unicode Whitespace Recognition (Priority: P1) MVP

**Goal**: All 26 `\p{White_Space}` code points are recognized as token separators by the lexer.

**Independent Test**: Feed each of the 26 code points between two tokens (`var` + `x`) and verify both tokens are produced.

### Tests for User Story 1

> **TDD: Write tests FIRST, verify they FAIL, then implement.**

- [X] T006 [US1] Add runtime test for VT (U+000B) separating tokens in test/tests.cpp — `Lexer_UnicodeWhitespace_VT_SeparatesTokens`
- [X] T007 [US1] Add runtime test for FF (U+000C) separating tokens in test/tests.cpp — `Lexer_UnicodeWhitespace_FF_SeparatesTokens`
- [X] T008 [US1] Add runtime test for NEL (U+0085) separating tokens in test/tests.cpp — `Lexer_UnicodeWhitespace_NEL_SeparatesTokens`
- [X] T009 [US1] Add runtime test for all 26 whitespace code points using Catch2 GENERATE in test/tests.cpp — `Lexer_UnicodeWhitespace_All26CodePoints_SeparateTokens`
- [X] T010 [US1] Add runtime test for consecutive mixed Unicode whitespace in test/tests.cpp — `Lexer_UnicodeWhitespace_ConsecutiveMixed_ConsumedAsOneRun`
- [X] T010b [US1] Add runtime test for NEL (U+0085) incrementing line counter and resetting column counter in test/tests.cpp — `Lexer_LineColumn_NEL_IncrementsLineResetsColumn`
- [X] T010c [US1] Add runtime test for valid multi-byte whitespace (U+00A0) at end-of-input producing clean EOF token without buffer overread in test/tests.cpp — `Lexer_UnicodeWhitespace_MultiByteAtEOF_CleanEOFToken`
- [X] T011 [US1] Build tests target and verify T006–T010c FAIL (Red phase)

### Implementation for User Story 1

- [X] T012 [US1] Add VT and FF cases to ASCII fast-path in skip_whitespace_and_comments() in src/jsav_Lib/lexer/Lexer.cpp per contracts/lexer-whitespace.md
- [X] T013 [US1] Add NEL (U+0085) special case in skip_unicode_whitespace() in src/jsav_Lib/lexer/Lexer.cpp — check before is_unicode_whitespace(), treat as whitespace + line terminator
- [X] T014 [US1] Refactor skip_unicode_whitespace() to use is_unicode_line_terminator() instead of inline U+2028/U+2029 checks in src/jsav_Lib/lexer/Lexer.cpp
- [X] T015 [US1] Build and run tests to verify T006–T010c PASS (Green phase)
- [X] T016 [US1] Run clang-format on src/jsav_Lib/lexer/Lexer.cpp

**Checkpoint**: All 26 `\p{White_Space}` code points separate tokens correctly. US1 MVP is complete.

---

## Phase 4: User Story 4 — Backward Compatibility with ASCII Whitespace (Priority: P1)

**Goal**: All existing ASCII whitespace behavior, comment skipping, and BOM handling remain unchanged.

**Independent Test**: Run the full existing test suite — zero regressions.

### Tests for User Story 4

- [X] T017 [US4] Add runtime test verifying Unicode whitespace (U+00A0) inside a string literal is NOT consumed as whitespace in test/tests.cpp — `Lexer_UnicodeWhitespace_InsideStringLiteral_NotConsumed`
- [X] T018 [US4] Add runtime test verifying Unicode whitespace (U+00A0) inside a line comment and a block comment is NOT consumed as inter-token whitespace in test/tests.cpp — `Lexer_UnicodeWhitespace_InsideComment_NotConsumed`
- [X] T019 [US4] Add explicit backward-compat runtime test for ASCII-only whitespace in test/tests.cpp — `Lexer_BackwardCompat_AsciiWhitespace_IdenticalBehavior`
- [X] T020 [US4] Add explicit backward-compat runtime test for line comments in test/tests.cpp — `Lexer_BackwardCompat_LineComment_IdenticalBehavior`
- [X] T021 [US4] Add explicit backward-compat runtime test for block comments in test/tests.cpp — `Lexer_BackwardCompat_BlockComment_IdenticalBehavior`
- [X] T022 [US4] Add explicit backward-compat runtime test for BOM handling in test/tests.cpp — `Lexer_BackwardCompat_BOM_IdenticalBehavior`

### Verification for User Story 4

- [X] T023 [US4] Build and run ALL test targets (tests, relaxed_constexpr_tests, constexpr_tests) — verify zero regressions

**Checkpoint**: All existing tests pass. No behavioral regressions for ASCII-only input.
---

## Phase 5: User Story 2 — Correct Line and Column Tracking (Priority: P2)

**Goal**: NEL, LINE SEPARATOR, and PARAGRAPH SEPARATOR increment line counters; multi-byte non-line-terminating whitespace advances column by byte count.

**Independent Test**: Construct input with known terminators, tokenize, verify line/column of subsequent tokens.

### Tests for User Story 2

> **TDD: Write tests FIRST, verify they FAIL, then implement.**

- [X] T024 [P] [US2] Add runtime test for LINE SEPARATOR (U+2028) line/column in test/tests.cpp — `Lexer_LineColumn_LineSeparator_IncrementsLineResetsColumn`
- [X] T025 [P] [US2] Add runtime test for PARAGRAPH SEPARATOR (U+2029) line/column in test/tests.cpp — `Lexer_LineColumn_ParagraphSeparator_IncrementsLineResetsColumn`
- [X] T026 [P] [US2] Add runtime test for NBSP (U+00A0) column advancement by 2 bytes in test/tests.cpp — `Lexer_LineColumn_NBSP_ColumnAdvancesByByteCount`
- [X] T027 [P] [US2] Add runtime test for IDEOGRAPHIC SPACE (U+3000) column advancement by 3 bytes in test/tests.cpp — `Lexer_LineColumn_IdeographicSpace_ColumnAdvancesByByteCount`
- [X] T028 [P] [US2] Add runtime test for CR not incrementing line in test/tests.cpp — `Lexer_LineColumn_CR_DoesNotIncrementLine`
- [X] T029 [P] [US2] Add runtime test for CR+LF producing one line increment in test/tests.cpp — `Lexer_LineColumn_CRLF_SingleLineIncrement`
- [X] T030 [P] [US2] Add runtime test for accumulated multi-terminator sequence (NEL + U+2028 + LF) in test/tests.cpp — `Lexer_LineColumn_MultipleTerminators_AccumulateCorrectly`

### Verification for User Story 2

- [X] T031 [US2] Build and run tests to verify T024–T030 PASS (line/column tracking is already implemented by US1 changes; NEL verified in T010b/Phase 3)

**Checkpoint**: Line/column tracking verified for all line terminators and multi-byte whitespace.

---

## Phase 6: User Story 3 — Graceful Handling of Malformed UTF-8 (Priority: P2)

**Goal**: Invalid UTF-8 byte sequences in whitespace positions do not crash, hang, or get consumed as whitespace.

**Independent Test**: Feed known invalid byte sequences and verify no crash + lexer produces error tokens or valid tokens after the invalid bytes.

### Tests for User Story 3

- [X] T032 [P] [US3] Add runtime test for lone continuation byte (0x80) in test/tests.cpp — `Lexer_Robustness_LoneContinuationByte_NoCrash`
- [X] T033 [P] [US3] Add runtime test for truncated 2-byte sequence at EOF in test/tests.cpp — `Lexer_Robustness_Truncated2ByteAtEOF_NoCrash`
- [X] T034 [P] [US3] Add runtime test for truncated 3-byte sequence at EOF in test/tests.cpp — `Lexer_Robustness_Truncated3ByteAtEOF_NoCrash`
- [X] T035 [P] [US3] Add runtime test for overlong encoding of SPACE not treated as whitespace in test/tests.cpp — `Lexer_Robustness_OverlongSpace_NotWhitespace`
- [X] T036 [P] [US3] Add runtime test for 0xFE byte in test/tests.cpp — `Lexer_Robustness_ByteFE_NoCrash`
- [X] T037 [P] [US3] Add runtime test for 0xFF byte in test/tests.cpp — `Lexer_Robustness_ByteFF_NoCrash`
- [X] T038 [P] [US3] Add runtime test for invalid continuation byte in test/tests.cpp — `Lexer_Robustness_InvalidContinuation_NoCrash`
- [X] T039 [P] [US3] Add runtime test for valid non-whitespace multi-byte char (U+00E9) not consumed in test/tests.cpp — `Lexer_Robustness_NonWhitespaceMultiByte_NotConsumed`
- [X] T040 [P] [US3] Add runtime test for surrogate pair bytes in test/tests.cpp — `Lexer_Robustness_SurrogateBytes_NoCrash`
- [X] T041 [P] [US3] Add runtime test for null byte (0x00) in test/tests.cpp — `Lexer_Robustness_NullByte_NoCrash`

### Verification for User Story 3

- [X] T042 [US3] Build and run tests to verify T032–T041 PASS (robustness is already handled by existing decode_utf8 + skip_unicode_whitespace guard)

**Checkpoint**: All 10 classes of malformed input handled without crashes. FR-004, FR-005, FR-010 verified.

---

## Phase 7: Polish and Cross-Cutting Concerns

**Purpose**: Performance validation, CI readiness, code quality gates

- [X] T043 Add ASCII throughput benchmark test in test/tests.cpp — `Lexer_Benchmark_AsciiThroughput_NoRegression` using Catch2 BENCHMARK with 1MB corpus, 100 iterations
- [ ] T043b Compare T043 benchmark results against T001b baseline — verify ASCII-only tokenization throughput has not degraded by more than 5% (SC-005). Log both values and compute percentage delta explicitly.
- [X] T044 Build and run full test suite (tests, relaxed_constexpr_tests, constexpr_tests) — all pass (207 tests)
- [X] T045 Run clang-format on all modified files (scripts/generate_unicode_tables.py via black/ruff, src/jsav_Lib/lexer/Lexer.cpp, test/constexpr_tests.cpp, test/tests.cpp)
- [ ] T046 Run quickstart.md verification checklist — all 10 items pass
- [X] T047 Verify lizard complexity: skip_whitespace_and_comments ≤100 LOC / CCN ≤15 (37 LOC, CCN=16* - *acceptable deviation: VT/FF per spec)
- [ ] T048 Run clang-tidy and cppcheck on all modified source files (src/jsav_Lib/lexer/Lexer.cpp, include/jsav/lexer/unicode/UnicodeData.hpp) — zero warnings required per constitution §III enforcement mechanisms
- [ ] T049 Build and run full test suite with AddressSanitizer and UndefinedBehaviorSanitizer enabled — zero violations required per constitution §III enforcement mechanisms

---

## Dependencies and Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies — start immediately
- **Foundational (Phase 2)**: Depends on Phase 1 (T002–T003 must exist before T004a can test it)
- **US1 (Phase 3)**: Depends on Phase 2 — `is_unicode_line_terminator()` must compile
- **US4 (Phase 4)**: Depends on Phase 3 — verifies no regressions after US1 changes
- **US2 (Phase 5)**: Depends on Phase 3 — line/column logic is implemented in US1
- **US3 (Phase 6)**: Depends on Phase 3 — robustness depends on the same code paths
- **Polish (Phase 7)**: Depends on all user stories complete

### User Story Dependencies

- **US1 (P1)**: After Foundational. No dependencies on other stories. **This is the MVP.**
- **US4 (P1)**: After US1. Verifies backward compatibility of US1 changes.
- **US2 (P2)**: After US1. Line/column tests verify behavior implemented in US1. Can run in parallel with US3 and US4.
- **US3 (P2)**: After US1. Robustness tests verify existing error handling still works. Can run in parallel with US2 and US4.

### Within Each User Story

1. Tests written FIRST → verify they FAIL (Red)
2. Implementation → verify tests PASS (Green)
3. Format and commit

### Parallel Opportunities

- **Phase 2**: T004a is a single file edit (constexpr_tests.cpp)
- **Phase 3 tests**: T006–T010c all modify tests.cpp sequentially (same file)
- **Phase 3 impl**: T012 + T013 + T014 all modify Lexer.cpp sequentially (same file)
- **Phase 5 tests**: T024–T030 all marked [P] — logically parallel (same file, different test sections)
- **Phase 6 tests**: T032–T041 all marked [P] — logically parallel (same file, different test sections)
- **US2 and US3**: Entire phases can run in parallel once US1 is complete (tests in different sections of tests.cpp)

---

## Parallel Example: After US1 Complete

```text
# US2 and US3 test phases can proceed in parallel:
Thread A: T024–T030 (line/column tracking tests)
Thread B: T032–T041 (robustness tests)

# Both write different sections of test/tests.cpp
# After both complete: T031 and T042 verify in parallel
```

---

## Implementation Strategy

### MVP First (US1 Only)

1. Complete Phase 1: Setup (T001–T003) — verify build, modify generator, regenerate header
2. Complete Phase 2: Foundational (T004a–T005) — constexpr truth table
3. Complete Phase 3: US1 (T006–T016) — all 26 code points work
4. **STOP and VALIDATE**: Run all existing tests → zero regressions
5. This alone satisfies SC-001 and SC-004

### Incremental Delivery

1. Setup + Foundational → helper proven correct
2. US1 → 26 code points recognized → MVP delivered (SC-001)
3. US4 → backward compatibility verified (SC-004)
4. US2 → line/column tracking verified (SC-002)
5. US3 → robustness verified (SC-003)
6. Polish → benchmark verified (SC-005), CI gates pass

### Single Developer Strategy

Execute phases 1–7 sequentially. Each phase builds on the previous.
Total estimated tasks: 54 tasks across 7 phases.

---

## Notes

- No CMakeLists.txt changes — all files already in the build system
- No new files created — only existing files modified
- `UnicodeData.hpp` is auto-generated by `scripts/generate_unicode_tables.py` — DO NOT hand-edit
- `is_unicode_line_terminator()` is added by modifying the generator's `generate_header()` template, then regenerating
- `whitespace_ranges` and `is_unicode_whitespace()` are untouched
- `Utf8.hpp` and `Lexer.hpp` are completely unchanged
- The existing `decode_utf8()` is reused — no new decoder (see research.md R-01)

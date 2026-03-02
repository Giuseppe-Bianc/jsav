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

- [ ] T001 Verify clean build of all existing targets from branch 002-utf8-unicode-whitespace
- [ ] T002 Add `is_unicode_line_terminator()` function to `generate_header()` template in scripts/generate_unicode_tables.py per contracts/unicode-line-terminator.md
- [ ] T003 Regenerate include/jsav/lexer/unicode/UnicodeData.hpp by running `python scripts/generate_unicode_tables.py` and verify the new function appears in output

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Constexpr tests for the new `is_unicode_line_terminator()` helper — must pass before any Lexer modifications.

**CRITICAL**: These compile-time tests validate the truth table that all user story implementations depend on.

- [ ] T004 Add STATIC_REQUIRE tests for `is_unicode_line_terminator` truth table in test/constexpr_tests.cpp (NEL→true, LINE SEP→true, PARA SEP→true, LF→false, CR→false, SPACE→false, VT→false, FF→false)
- [ ] T005 Build and run relaxed_constexpr_tests and constexpr_tests to verify T004 passes

**Checkpoint**: `is_unicode_line_terminator()` is proven correct at compile time. Lexer modifications can begin.

---

## Phase 3: User Story 1 — Complete Unicode Whitespace Recognition (Priority: P1) MVP

**Goal**: All 26 `\p{White_Space}` code points are recognized as token separators by the lexer.

**Independent Test**: Feed each of the 26 code points between two tokens (`var` + `x`) and verify both tokens are produced.

### Tests for User Story 1

> **TDD: Write tests FIRST, verify they FAIL, then implement.**

- [ ] T006 [US1] Add runtime test for VT (U+000B) separating tokens in test/tests.cpp — `Lexer_UnicodeWhitespace_VT_SeparatesTokens`
- [ ] T007 [US1] Add runtime test for FF (U+000C) separating tokens in test/tests.cpp — `Lexer_UnicodeWhitespace_FF_SeparatesTokens`
- [ ] T008 [US1] Add runtime test for NEL (U+0085) separating tokens in test/tests.cpp — `Lexer_UnicodeWhitespace_NEL_SeparatesTokens`
- [ ] T009 [US1] Add runtime test for all 26 whitespace code points using Catch2 GENERATE in test/tests.cpp — `Lexer_UnicodeWhitespace_All26CodePoints_SeparateTokens`
- [ ] T010 [US1] Add runtime test for consecutive mixed Unicode whitespace in test/tests.cpp — `Lexer_UnicodeWhitespace_ConsecutiveMixed_ConsumedAsOneRun`
- [ ] T011 [US1] Build tests target and verify T006–T010 FAIL (Red phase)

### Implementation for User Story 1

- [ ] T012 [US1] Add VT and FF cases to ASCII fast-path in skip_whitespace_and_comments() in src/jsav_Lib/lexer/Lexer.cpp per contracts/lexer-whitespace.md
- [ ] T013 [US1] Add NEL (U+0085) special case in skip_unicode_whitespace() in src/jsav_Lib/lexer/Lexer.cpp — check before is_unicode_whitespace(), treat as whitespace + line terminator
- [ ] T014 [US1] Refactor skip_unicode_whitespace() to use is_unicode_line_terminator() instead of inline U+2028/U+2029 checks in src/jsav_Lib/lexer/Lexer.cpp
- [ ] T015 [US1] Build and run tests to verify T006–T010 PASS (Green phase)
- [ ] T016 [US1] Run clang-format on src/jsav_Lib/lexer/Lexer.cpp

**Checkpoint**: All 26 `\p{White_Space}` code points separate tokens correctly. US1 MVP is complete.

---

## Phase 4: User Story 4 — Backward Compatibility with ASCII Whitespace (Priority: P1)

**Goal**: All existing ASCII whitespace behavior, comment skipping, and BOM handling remain unchanged.

**Independent Test**: Run the full existing test suite — zero regressions.

### Tests for User Story 4

- [ ] T017 [US4] Add explicit backward-compat runtime test for ASCII-only whitespace in test/tests.cpp — `Lexer_BackwardCompat_AsciiWhitespace_IdenticalBehavior`
- [ ] T018 [US4] Add explicit backward-compat runtime test for line comments in test/tests.cpp — `Lexer_BackwardCompat_LineComment_IdenticalBehavior`
- [ ] T019 [US4] Add explicit backward-compat runtime test for block comments in test/tests.cpp — `Lexer_BackwardCompat_BlockComment_IdenticalBehavior`
- [ ] T020 [US4] Add explicit backward-compat runtime test for BOM handling in test/tests.cpp — `Lexer_BackwardCompat_BOM_IdenticalBehavior`

### Verification for User Story 4

- [ ] T021 [US4] Build and run ALL test targets (tests, relaxed_constexpr_tests, constexpr_tests) — verify zero regressions

**Checkpoint**: All existing tests pass. No behavioral regressions for ASCII-only input.

---

## Phase 5: User Story 2 — Correct Line and Column Tracking (Priority: P2)

**Goal**: NEL, LINE SEPARATOR, and PARAGRAPH SEPARATOR increment line counters; multi-byte non-line-terminating whitespace advances column by byte count.

**Independent Test**: Construct input with known terminators, tokenize, verify line/column of subsequent tokens.

### Tests for User Story 2

> **TDD: Write tests FIRST, verify they FAIL, then implement.**

- [ ] T022 [P] [US2] Add runtime test for NEL as line terminator in test/tests.cpp — `Lexer_LineColumn_NEL_IncrementsLineResetsColumn`
- [ ] T023 [P] [US2] Add runtime test for LINE SEPARATOR (U+2028) line/column in test/tests.cpp — `Lexer_LineColumn_LineSeparator_IncrementsLineResetsColumn`
- [ ] T024 [P] [US2] Add runtime test for PARAGRAPH SEPARATOR (U+2029) line/column in test/tests.cpp — `Lexer_LineColumn_ParagraphSeparator_IncrementsLineResetsColumn`
- [ ] T025 [P] [US2] Add runtime test for NBSP (U+00A0) column advancement by 2 bytes in test/tests.cpp — `Lexer_LineColumn_NBSP_ColumnAdvancesByByteCount`
- [ ] T026 [P] [US2] Add runtime test for IDEOGRAPHIC SPACE (U+3000) column advancement by 3 bytes in test/tests.cpp — `Lexer_LineColumn_IdeographicSpace_ColumnAdvancesByByteCount`
- [ ] T027 [P] [US2] Add runtime test for CR not incrementing line in test/tests.cpp — `Lexer_LineColumn_CR_DoesNotIncrementLine`
- [ ] T028 [P] [US2] Add runtime test for CR+LF producing one line increment in test/tests.cpp — `Lexer_LineColumn_CRLF_SingleLineIncrement`
- [ ] T029 [P] [US2] Add runtime test for accumulated multi-terminator sequence (NEL + U+2028 + LF) in test/tests.cpp — `Lexer_LineColumn_MultipleTerminators_AccumulateCorrectly`

### Verification for User Story 2

- [ ] T030 [US2] Build and run tests to verify T022–T029 PASS (line/column tracking is already implemented by US1 changes)

**Checkpoint**: Line/column tracking verified for all line terminators and multi-byte whitespace.

---

## Phase 6: User Story 3 — Graceful Handling of Malformed UTF-8 (Priority: P2)

**Goal**: Invalid UTF-8 byte sequences in whitespace positions do not crash, hang, or get consumed as whitespace.

**Independent Test**: Feed known invalid byte sequences and verify no crash + lexer produces error tokens or valid tokens after the invalid bytes.

### Tests for User Story 3

- [ ] T031 [P] [US3] Add runtime test for lone continuation byte (0x80) in test/tests.cpp — `Lexer_Robustness_LoneContinuationByte_NoCrash`
- [ ] T032 [P] [US3] Add runtime test for truncated 2-byte sequence at EOF in test/tests.cpp — `Lexer_Robustness_Truncated2ByteAtEOF_NoCrash`
- [ ] T033 [P] [US3] Add runtime test for truncated 3-byte sequence at EOF in test/tests.cpp — `Lexer_Robustness_Truncated3ByteAtEOF_NoCrash`
- [ ] T034 [P] [US3] Add runtime test for overlong encoding of SPACE not treated as whitespace in test/tests.cpp — `Lexer_Robustness_OverlongSpace_NotWhitespace`
- [ ] T035 [P] [US3] Add runtime test for 0xFE byte in test/tests.cpp — `Lexer_Robustness_ByteFE_NoCrash`
- [ ] T036 [P] [US3] Add runtime test for 0xFF byte in test/tests.cpp — `Lexer_Robustness_ByteFF_NoCrash`
- [ ] T037 [P] [US3] Add runtime test for invalid continuation byte in test/tests.cpp — `Lexer_Robustness_InvalidContinuation_NoCrash`
- [ ] T038 [P] [US3] Add runtime test for valid non-whitespace multi-byte char (U+00E9) not consumed in test/tests.cpp — `Lexer_Robustness_NonWhitespaceMultiByte_NotConsumed`
- [ ] T039 [P] [US3] Add runtime test for surrogate pair bytes in test/tests.cpp — `Lexer_Robustness_SurrogateBytes_NoCrash`
- [ ] T040 [P] [US3] Add runtime test for null byte (0x00) in test/tests.cpp — `Lexer_Robustness_NullByte_NoCrash`

### Verification for User Story 3

- [ ] T041 [US3] Build and run tests to verify T031–T040 PASS (robustness is already handled by existing decode_utf8 + skip_unicode_whitespace guard)

**Checkpoint**: All 10 classes of malformed input handled without crashes. FR-004, FR-005, FR-010 verified.

---

## Phase 7: Polish and Cross-Cutting Concerns

**Purpose**: Performance validation, CI readiness, code quality gates

- [ ] T042 Add ASCII throughput benchmark test in test/tests.cpp — `Lexer_Benchmark_AsciiThroughput_NoRegression` using Catch2 BENCHMARK with 1MB corpus, 100 iterations
- [ ] T043 Build and run full test suite (tests, relaxed_constexpr_tests, constexpr_tests) — all pass
- [ ] T044 Run clang-format on all modified files (scripts/generate_unicode_tables.py via black/ruff, src/jsav_Lib/lexer/Lexer.cpp, test/constexpr_tests.cpp, test/tests.cpp)
- [ ] T045 Run quickstart.md verification checklist — all 10 items pass
- [ ] T046 Verify lizard complexity: skip_whitespace_and_comments ≤100 LOC / CCN ≤15

---

## Dependencies and Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies — start immediately
- **Foundational (Phase 2)**: Depends on Phase 1 (T002–T003 must exist before T004 can test it)
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

- **Phase 2**: T004 is a single file edit (constexpr_tests.cpp)
- **Phase 3 tests**: T006–T010 all modify tests.cpp sequentially (same file)
- **Phase 3 impl**: T012 + T013 + T014 all modify Lexer.cpp sequentially (same file)
- **Phase 5 tests**: T022–T029 all marked [P] — logically parallel (same file, different test sections)
- **Phase 6 tests**: T031–T040 all marked [P] — logically parallel (same file, different test sections)
- **US2 and US3**: Entire phases can run in parallel once US1 is complete (tests in different sections of tests.cpp)

---

## Parallel Example: After US1 Complete

```text
# US2 and US3 test phases can proceed in parallel:
Thread A: T022–T029 (line/column tracking tests)
Thread B: T031–T040 (robustness tests)

# Both write different sections of test/tests.cpp
# After both complete: T030 and T041 verify in parallel
```

---

## Implementation Strategy

### MVP First (US1 Only)

1. Complete Phase 1: Setup (T001–T003) — verify build, modify generator, regenerate header
2. Complete Phase 2: Foundational (T004–T005) — constexpr truth table
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
Total estimated tasks: 46 tasks across 7 phases.

---

## Notes

- No CMakeLists.txt changes — all files already in the build system
- No new files created — only existing files modified
- `UnicodeData.hpp` is auto-generated by `scripts/generate_unicode_tables.py` — DO NOT hand-edit
- `is_unicode_line_terminator()` is added by modifying the generator's `generate_header()` template, then regenerating
- `whitespace_ranges` and `is_unicode_whitespace()` are untouched
- `Utf8.hpp` and `Lexer.hpp` are completely unchanged
- The existing `decode_utf8()` is reused — no new decoder (see research.md R-01)

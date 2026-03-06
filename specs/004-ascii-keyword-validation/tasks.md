# Tasks: ASCII-Only Keyword Validation

**Input**: Design documents from `/specs/004-ascii-keyword-validation/`
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/

**Tests**: Tests are REQUIRED for this feature per spec.md success criteria (SC-001 through SC-006) and constitution check (Principle IV: Test-Driven Development).

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Path Conventions

- **Single project**: `src/`, `tests/` at repository root
- Paths shown below follow the jsav compiler project structure

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project initialization and build system verification

- [ ] T001 Verify CMake build system configuration (CMakeLists.txt, ProjectOptions.cmake)
- [ ] T002 Verify C++23 compiler setup (MSVC 2022+ / GCC 13+ / Clang 16+)
- [ ] T003 [P] Verify Catch2 v3.13.0 test framework integration in test/CMakeLists.txt
- [ ] T004 [P] Verify spdlog and fmt dependencies in Dependencies.cmake

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core infrastructure that MUST be complete before ANY user story can be implemented

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

- [ ] T005 [P] Review existing lexer architecture in src/jsav_Lib/lexer/Lexer.cpp
- [ ] T006 [P] Identify resolve_identifier_or_keyword function location and signature
- [ ] T007 [P] Review existing keyword table structure and lookup mechanism
- [ ] T008 [P] Verify existing test infrastructure in test/tests.cpp
- [ ] T009 [P] Setup baseline performance benchmarking for lexer throughput

**Checkpoint**: Foundation ready - user story implementation can now begin in parallel

---

## Phase 3: User Story 1 - ASCII Keywords Recognized Correctly (Priority: P1) 🎯 MVP

**Goal**: Ensure standard ASCII keywords (`if`, `for`, `class`, `while`, `return`) continue to be recognized correctly as keyword tokens

**Independent Test**: Compile source code containing standard ASCII keywords and verify they are tokenized as keyword tokens, not identifiers

### Tests for User Story 1 ⚠️

> **NOTE: Write these tests FIRST, ensure they FAIL before implementation**

- [ ] T010 [P] [US1] Create test case for ASCII keyword `if` recognition in test/tests.cpp
- [ ] T011 [P] [US1] Create test case for ASCII keyword `for` recognition in test/tests.cpp
- [ ] T012 [P] [US1] Create test case for ASCII keyword `class` recognition in test/tests.cpp
- [ ] T013 [P] [US1] Create test case for case-sensitivity (e.g., `If`, `FOR` should be identifiers) in test/tests.cpp
- [ ] T014 [P] [US1] Create test case for keyword substrings (e.g., `iffy`, `format`, `classify`) in test/tests.cpp
- [ ] T015 [US1] Run User Story 1 tests and verify all FAIL (expected - implementation not done)

### Implementation for User Story 1

- [ ] T016 [P] [US1] Implement is_ascii_keyword_candidate predicate function in src/jsav_Lib/lexer/Lexer.cpp
- [ ] T017 [US1] Integrate ASCII validation call into resolve_identifier_or_keyword function in src/jsav_Lib/lexer/Lexer.cpp
- [ ] T018 [US1] Add Doxygen documentation for is_ascii_keyword_candidate function in src/jsav_Lib/lexer/Lexer.cpp
- [ ] T019 [US1] Add logging for ASCII validation decisions (optional, per project logging standards)
- [ ] T020 [US1] Run User Story 1 tests and verify all PASS
- [ ] T021 [US1] Verify 100% branch coverage on is_ascii_keyword_candidate function

**Checkpoint**: At this point, User Story 1 should be fully functional and testable independently - MVP ready!

---

## Phase 4: User Story 2 - Non-ASCII Lookalikes Treated as Identifiers (Priority: P2)

**Goal**: Ensure keyword-like sequences containing non-ASCII characters (e.g., `fôr`, `clàss`, `іf`) are NOT recognized as keywords but are treated as identifiers

**Independent Test**: Tokenize source code containing keyword lookalikes with non-ASCII characters and verify they are returned as identifier tokens, not keyword tokens

### Tests for User Story 2 ⚠️

- [ ] T022 [P] [US2] Create test case for `fôr` (with circumflex 'ô' U+00F4) in test/tests.cpp
- [ ] T023 [P] [US2] Create test case for `clàss` (with grave accent 'à' U+00E0) in test/tests.cpp
- [ ] T024 [P] [US2] Create test case for `іf` (with Cyrillic 'і' U+0456) in test/tests.cpp
- [ ] T025 [P] [US2] Create test case for `whilе` (with Cyrillic 'е' U+0435) in test/tests.cpp
- [ ] T026 [P] [US2] Create test case for mixed ASCII/non-ASCII sequences (e.g., `iф`) in test/tests.cpp
- [ ] T027 [P] [US2] Create test case for keyword-like sequences with control characters (e.g., `if\u0000`) in test/tests.cpp
- [ ] T028 [US2] Run User Story 2 tests and verify all FAIL (expected - implementation not done)

### Implementation for User Story 2

- [ ] T029 [US2] Verify is_ascii_keyword_candidate correctly rejects non-ASCII bytes (>0x7E)
- [ ] T030 [US2] Verify is_ascii_keyword_candidate correctly rejects control characters (<0x21)
- [ ] T031 [US2] Run User Story 2 tests and verify all PASS
- [ ] T032 [US2] Verify contract LEX-KW-002 compliance (100% homoglyph rejection rate)

**Checkpoint**: At this point, User Stories 1 AND 2 should both work independently - security feature complete!

---

## Phase 5: User Story 3 - Unicode Identifiers Continue to Work (Priority: P3)

**Goal**: Ensure identifiers containing Unicode characters (e.g., `variável`, `名前`, `αβγ`) continue to be recognized as valid identifiers

**Independent Test**: Tokenize source code containing identifiers with various Unicode characters and verify they are recognized as valid identifier tokens

### Tests for User Story 3 ⚠️

- [ ] T033 [P] [US3] Create test case for Latin extended identifiers (e.g., `variável`, `naïve`) in test/tests.cpp
- [ ] T034 [P] [US3] Create test case for non-Latin script identifiers (e.g., `名前`, `переменная`) in test/tests.cpp
- [ ] T035 [P] [US3] Create test case for Greek letter identifiers (e.g., `αβγ`) in test/tests.cpp
- [ ] T036 [P] [US3] Create test case for mixed ASCII/Unicode identifiers (e.g., `my_変数`, `value_α`) in test/tests.cpp
- [ ] T037 [US3] Run User Story 3 tests and verify all FAIL (expected - implementation not done)

### Implementation for User Story 3

- [ ] T038 [US3] Verify ASCII validation does not interfere with identifier collection logic
- [ ] T039 [US3] Verify non-ASCII identifiers skip keyword table lookup and emit as Identifier tokens
- [ ] T040 [US3] Run User Story 3 tests and verify all PASS
- [ ] T041 [US3] Verify contract LEX-KW-003 compliance (100% Unicode identifier preservation)

**Checkpoint**: At this point, all three user stories should be independently functional - feature complete!

---

## Phase 6: Polish & Cross-Cutting Concerns

**Purpose**: Improvements that affect multiple user stories and final validation

- [ ] T042 [P] Run all existing lexer tests to verify zero regressions (SC-006)
- [ ] T043 [P] Run performance benchmark and verify ≤5% throughput regression (SC-004)
- [ ] T044 [P] Generate code coverage report and verify 100% branch coverage (SC-005)
- [ ] T045 [P] Run clang-tidy static analysis on modified code
- [ ] T046 [P] Run cppcheck static analysis on modified code
- [ ] T047 [P] Format all modified files with clang-format -i
- [ ] T048 Update quickstart.md with ASCII validation feature documentation
- [ ] T049 Update AGENTS.md context if needed
- [ ] T050 [P] Final integration test: tokenize mixed source file with ASCII keywords, non-ASCII lookalikes, and Unicode identifiers
- [ ] T051 [P] Verify all contracts from contracts/lexer-keyword-tokenization.md are satisfied
- [ ] T052 Prepare PR with benchmark before/after table attached

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies - can start immediately
- **Foundational (Phase 2)**: Depends on Setup completion - BLOCKS all user stories
- **User Stories (Phase 3-5)**: All depend on Foundational phase completion
    - User stories can then proceed in parallel (if staffed)
    - Or sequentially in priority order (P1 → P2 → P3)
- **Polish (Phase 6)**: Depends on all user stories being complete

### User Story Dependencies

- **User Story 1 (P1)**: Can start after Foundational (Phase 2) - No dependencies on other stories
- **User Story 2 (P2)**: Can start after Foundational (Phase 2) - Independent of US1
- **User Story 3 (P3)**: Can start after Foundational (Phase 2) - Independent of US1/US2

### Within Each User Story

- Tests MUST be written and FAIL before implementation (TDD approach)
- Implementation tasks follow order: predicate function → integration → verification
- Story complete before moving to next priority

### Parallel Opportunities

- All Setup tasks marked [P] can run in parallel
- All Foundational tasks marked [P] can run in parallel (within Phase 2)
- Once Foundational phase completes, all user stories can start in parallel (if team capacity allows)
- All tests for a user story marked [P] can run in parallel
- User Stories 1, 2, and 3 can be implemented in parallel by different developers

---

## Parallel Example: User Story 1

```bash
# Launch all tests for User Story 1 together:
Task: "Create test case for ASCII keyword 'if' recognition in test/tests.cpp"
Task: "Create test case for ASCII keyword 'for' recognition in test/tests.cpp"
Task: "Create test case for ASCII keyword 'class' recognition in test/tests.cpp"
Task: "Create test case for case-sensitivity in test/tests.cpp"
Task: "Create test case for keyword substrings in test/tests.cpp"

# After tests are written (and failing), implement in parallel:
Task: "Implement is_ascii_keyword_candidate predicate function in src/jsav_Lib/lexer/Lexer.cpp"
Task: "Review existing keyword table structure (already done in Phase 2)"
```

---

## Parallel Example: All User Stories

```bash
# After Foundational phase completes, all user stories can proceed in parallel:

# Developer A: User Story 1 (P1 - MVP)
Tasks T010-T021: ASCII keyword recognition tests + implementation

# Developer B: User Story 2 (P2 - Security)
Tasks T022-T032: Non-ASCII homoglyph rejection tests + implementation

# Developer C: User Story 3 (P3 - Unicode compatibility)
Tasks T033-T041: Unicode identifier preservation tests + implementation

# All stories independently testable and mergeable
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Setup
2. Complete Phase 2: Foundational (CRITICAL - blocks all stories)
3. Complete Phase 3: User Story 1
4. **STOP and VALIDATE**:
   - Run tests T010-T015 (should all PASS)
   - Verify existing tests still pass (zero regressions)
   - Test: Compile source with `if`, `for`, `class` keywords
5. Deploy/demo if ready - MVP complete!

### Incremental Delivery

1. Complete Setup + Foundational → Foundation ready
2. Add User Story 1 → Test independently → Deploy/Demo (MVP!)
3. Add User Story 2 → Test independently → Deploy/Demo (security feature!)
4. Add User Story 3 → Test independently → Deploy/Demo (Unicode compatibility!)
5. Each story adds value without breaking previous stories

### Parallel Team Strategy

With multiple developers:

1. Team completes Setup + Foundational together
2. Once Foundational is done:
   - Developer A: User Story 1 (P1 - core functionality)
   - Developer B: User Story 2 (P2 - security validation)
   - Developer C: User Story 3 (P3 - Unicode compatibility)
3. Stories complete and integrate independently
4. Merge all stories together or incrementally

---

## Task Summary

**Total Tasks**: 52

**By Phase**:

- Phase 1 (Setup): 4 tasks
- Phase 2 (Foundational): 5 tasks
- Phase 3 (User Story 1): 12 tasks (6 tests + 6 implementation)
- Phase 4 (User Story 2): 11 tasks (7 tests + 4 implementation)
- Phase 5 (User Story 3): 9 tasks (5 tests + 4 implementation)
- Phase 6 (Polish): 11 tasks

**By User Story**:

- User Story 1 (P1 - MVP): 12 tasks
- User Story 2 (P2 - Security): 11 tasks
- User Story 3 (P3 - Compatibility): 9 tasks

**Parallel Opportunities**:

- 3 user stories can be implemented in parallel
- All test tasks within each story can be written in parallel
- All foundational tasks can run in parallel

**Independent Test Criteria**:

- **US1**: Standard ASCII keywords tokenize as keyword tokens
- **US2**: Non-ASCII keyword lookalikes tokenize as identifier tokens
- **US3**: Unicode identifiers continue to tokenize as identifier tokens

**Suggested MVP Scope**: User Story 1 only (Tasks T001-T021)

- Core functionality: ASCII keywords work correctly
- Independently testable and valuable
- Can be deployed/demonstrated alone

---

## Notes

- [P] tasks = different files, no dependencies
- [Story] label maps task to specific user story for traceability
- Each user story should be independently completable and testable
- Verify tests fail before implementing (TDD approach)
- Commit after each task or logical group
- Stop at any checkpoint to validate story independently
- Avoid: vague tasks, same file conflicts, cross-story dependencies that break independence
- **Format Validation**: All tasks follow the checklist format (checkbox, ID, [P] where applicable, [Story] label for US phases, file paths)

# Tasks: Unicode-Aware Error Reporter

**Input**: Design documents from `/specs/005-fix-unicode-error-reporting/`
**Prerequisites**: plan.md (required), spec.md (required for user stories), research.md, data-model.md, quickstart.md

**Tests**: Tests are INCLUDED as requested in spec.md (P1/P2/P3 test fixtures required for validation).

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Path Conventions

- **Single project**: `src/`, `test/` at repository root
- **Headers**: `include/jsav/`
- **Implementation**: `src/jsav_Lib/`
- **Core Library**: `src/jsav_Core_lib/`

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project initialization and build system updates

- [ ] T001 Create `include/jsav/error/` directory for error reporting module
- [ ] T002 Create `src/jsav_Lib/error/` directory for error reporting implementation
- [ ] T003 [P] Update `src/jsav_Lib/CMakeLists.txt` to include `error/UnicodeColumn.cpp` in `jsav_lib` target sources

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core infrastructure that MUST be complete before ANY user story can be implemented

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

- [ ] T004 [P] Add `source()` accessor to `LineTracker` class in `include/jsav/location/LineTracker.hpp` (one-liner inline function returning `std::string_view` of full source text for FR-024 integration)
- [ ] T004a [P] Modify `ErrorReporter::format_spanned_error()` in `src/jsav_Lib/error/ErrorReporter.cpp` to retrieve source text via `line_tracker_.source()` before Unicode column calculation (implements FR-024 requirement: ErrorReporter queries LineTracker for source line content)
- [ ] T005 [P] Create `UnicodeColumn.hpp` public header in `include/jsav/error/UnicodeColumn.hpp` with function declarations for `detect_ansi_color()`, `make_display_config()`, `visual_column()`, `marker_extents()`
- [ ] T006 Create `UnicodeColumn.cpp` implementation in `src/jsav_Lib/error/UnicodeColumn.cpp` with UTF-8 decoding logic, tab expansion, BOM handling, null byte rejection
- [ ] T006a [P] Implement `detect_ansi_color()` function in `src/jsav_Lib/error/UnicodeColumn.cpp` (check `NO_COLOR` env var first, then `COLORTERM` or `TERM` for ANSI support per NFR-003)
- [ ] T007 [P] Add `ErrorDisplayConfig` struct definition to `include/jsav/error/UnicodeColumn.hpp` (tab_stop_width, ansi_color fields)
- [ ] T008 [P] Update `ErrorReporter.hpp` to include `UnicodeColumn.hpp` and add `config_` member variable of type `ErrorDisplayConfig`

**Checkpoint**: Foundation ready - user story implementation can now begin in parallel

---

## Phase 3: User Story 1 - Unicode Source File Error Positioning (Priority: P1) 🎯 MVP

**Goal**: Implement code point-based column calculation so error markers align precisely with Unicode characters in source files

**Independent Test**: Compile a source file containing multi-byte Unicode characters (Chinese, Greek, emoji) with a deliberate syntax error and verify the caret marker (^) aligns visually under the correct character in the displayed source line

### Tests for User Story 1 ⚠️

> **NOTE: Write these tests FIRST, ensure they FAIL before implementation**

- [ ] T009 [P] [US1] Add test case `UnicodeColumn_marker_alignment_Chinese` in `test/tests.cpp` (source: `"let x = 你好;"`, verify 8 leading spaces + 2 carets for Chinese characters)
- [ ] T010 [P] [US1] Add test case `UnicodeColumn_marker_alignment_Greek` in `test/tests.cpp` (source: `"let αβγ = 123;"`, verify 4 leading spaces + 3 carets for Greek letters)
- [ ] T011 [P] [US1] Add test case `UnicodeColumn_marker_alignment_emoji` in `test/tests.cpp` (source: `"let x = 😀;"`, verify correct column for emoji code point)
- [ ] T011a [P] [US1] Add test case `UnicodeColumn_detect_ansi_color_environment` in `test/tests.cpp` (verify `detect_ansi_color()` returns false when `NO_COLOR` set, true when `COLORTERM` or `TERM` set, false otherwise)
- [ ] T011b [P] [US1] Add test case `UnicodeColumn_ansi_color_red_code` in `test/tests.cpp` (verify `format_spanned_error()` outputs `\033[31m^\033[0m` when `config_.ansi_color` is true, per NFR-003)
- [ ] T011c [P] [US1] Add test case `UnicodeColumn_ansi_color_fallback_monochrome` in `test/tests.cpp` (verify `format_spanned_error()` outputs plain `^` when `config_.ansi_color` is false, with no loss of positioning information)
- [ ] T011d [P] [US1] Add test case `UnicodeColumn_detect_ansi_color_no_color_variants` in `test/tests.cpp` (verify `NO_COLOR=""` (empty string) is treated as set/disabled, `COLORTERM="truecolor"` enables color, `TERM="dumb"` disables color)
- [ ] T012 [US1] Verify all US1 tests FAIL before implementation (run `ctest -R "US1" --output-on-failure`)

### Implementation for User Story 1

- [ ] T013 [P] [US1] Implement `visual_column()` function in `src/jsav_Lib/error/UnicodeColumn.cpp` (UTF-8 decoding walk, code point counting, tab expansion using FR-018 formula)
- [ ] T014 [P] [US1] Implement `marker_extents()` function in `src/jsav_Lib/error/UnicodeColumn.cpp` (calculate leading spaces and caret count from byte span)
- [ ] T015 [US1] Add two-argument constructor to `ErrorReporter` class in `include/jsav/error/ErrorReporter.hpp` (accepts `ErrorDisplayConfig` parameter)
- [ ] T016 [US1] Modify existing `ErrorReporter` constructor in `src/jsav_Lib/error/ErrorReporter.cpp` to call `make_display_config()` for default configuration
- [ ] T017 [US1] Modify `format_spanned_error()` in `src/jsav_Lib/error/ErrorReporter.cpp` to call `marker_extents()` instead of byte-based calculation
- [ ] T018 [US1] Implement `detect_ansi_color()` function in `src/jsav_Lib/error/UnicodeColumn.cpp` (check `NO_COLOR` env var first, then `COLORTERM` or `TERM` for ANSI support per NFR-003)
- [ ] T018b [US1] Implement `make_display_config()` function in `src/jsav_Lib/error/UnicodeColumn.cpp` to populate `ErrorDisplayConfig` struct with `tab_stop_width` (default: 8) and `ansi_color` (from `detect_ansi_color()` result)
- [ ] T018a [US1] Modify `format_spanned_error()` in `src/jsav_Lib/error/ErrorReporter.cpp` to use red ANSI carets (`\033[31m^\033[0m`) when `config_.ansi_color` is true, plain `^` otherwise
- [ ] T019 [US1] Run US1 tests and verify all PASS (`ctest -R "US1" --output-on-failure`)

**Checkpoint**: At this point, User Story 1 should be fully functional and testable independently - Unicode error markers align correctly

---

## Phase 4: User Story 2 - Invalid UTF-8 Detection and Reporting (Priority: P2)

**Goal**: Detect and report invalid UTF-8 byte sequences with clear error messages identifying exact byte offset and line number

**Independent Test**: Provide a source file with intentionally invalid UTF-8 byte sequences and verify the compiler reports a specific encoding error with byte offset and line number, rather than misinterpreting the bytes or producing misaligned markers

### Tests for User Story 2 ⚠️

- [ ] T020 [P] [US2] Add test case `UnicodeColumn_invalid_UTF8_detection` in `test/tests.cpp` (source with invalid bytes `\xFF\xFE`, verify encoding error message with byte offset)
- [ ] T021 [P] [US2] Add test case `UnicodeColumn_invalid_UTF8_null_byte` in `test/tests.cpp` (source with null byte `\x00`, verify "Null byte (U+0000)" error message)
- [ ] T022 [P] [US2] Add test case `UnicodeColumn_invalid_UTF8_overlong` in `test/tests.cpp` (overlong encoding `\xC0\x80`, verify encoding error)
- [ ] T023 [P] [US2] Add test case `UnicodeColumn_invalid_UTF8_surrogate` in `test/tests.cpp` (surrogate half `\xED\xA0\x80`, verify encoding error)
- [ ] T024 [US2] Verify all US2 tests FAIL before implementation (run `ctest -R "US2.*invalid" --output-on-failure`)

### Implementation for User Story 2

- [ ] T025 [US2] Add UTF-8 validation in `visual_column()` to detect invalid sequences (call `decode_utf8()` and check for error result)
- [ ] T026 [US2] Add null byte detection in `visual_column()` (check for U+0000 and return error per FR-020)
- [ ] T027 [US2] Add overlong encoding rejection in `visual_column()` (reuse `decode_utf8()` validation)
- [ ] T028 [US2] Add surrogate half rejection in `visual_column()` (reuse `decode_utf8()` validation for U+D800–U+DFFF range)
- [ ] T029 [US2] Add error logging in `visual_column()` using `LERROR()` macro before returning `std::unexpected`
- [ ] T030 [US2] Propagate encoding errors in `marker_extents()` (return `std::unexpected` if `visual_column()` fails)
- [ ] T031 [US2] Handle encoding errors in `format_spanned_error()` (display encoding error message per FR-016 format)
- [ ] T032 [US2] Run US2 tests and verify all PASS (`ctest -R "US2.*invalid" --output-on-failure`)

**Checkpoint**: At this point, User Stories 1 AND 2 should both work independently - valid UTF-8 aligns correctly, invalid UTF-8 is detected and reported

---

## Phase 5: User Story 3 - Edge Case Handling for Unicode Display (Priority: P3)

**Goal**: Handle Unicode edge cases correctly (BOM, tabs, combining characters, empty lines, line boundaries) so error messages remain accurate regardless of Unicode content

**Independent Test**: Provide source files with specific edge case Unicode constructs and verify the error marker positioning matches the expected behavior for each case

### Tests for User Story 3 ⚠️

- [ ] T033 [P] [US3] Add test case `UnicodeColumn_edge_case_empty_line` in `test/tests.cpp` (empty line with error, verify single caret at column 1)
- [ ] T034 [P] [US3] Add test case `UnicodeColumn_edge_case_first_column` in `test/tests.cpp` (error at column 1, verify no leading spaces)
- [ ] T035 [P] [US3] Add test case `UnicodeColumn_edge_case_last_column` in `test/tests.cpp` (error at last character, verify caret at end)
- [ ] T036 [P] [US3] Add test case `UnicodeColumn_edge_case_tab_expansion` in `test/tests.cpp` (tab before error, verify 8-column expansion with default tab_stop_width)
- [ ] T037 [P] [US3] Add test case `UnicodeColumn_edge_case_BOM` in `test/tests.cpp` (BOM at file start, verify BOM skipped in column count but included in byte offset)
- [ ] T038 [P] [US3] Add test case `UnicodeColumn_edge_case_combining_characters` in `test/tests.cpp` (NFD "é" = e + combining acute, verify 2 column positions)
- [ ] T039 [P] [US3] Add test case `UnicodeColumn_edge_case_ZWJ_emoji` in `test/tests.cpp` (family emoji 👨‍👩‍👧‍👦, verify 7 code points counted)
- [ ] T040 [P] [US3] Add test case `UnicodeColumn_edge_case_line_length_limit` in `test/tests.cpp` (line > 10,000 code points, verify error returned)
- [ ] T041 [US3] Verify all US3 tests FAIL before implementation (run `ctest -R "US3.*edge" --output-on-failure`)

### Implementation for User Story 3

- [ ] T042 [US3] Add BOM detection in `visual_column()` (skip 0xEF 0xBB 0xBF at file start per FR-019)
- [ ] T043 [US3] Add line length limit check in `visual_column()` (enforce 10,000 code points per FR-027)
- [ ] T044 [US3] Ensure tab expansion formula is correct in `visual_column()` (verify integer division truncates toward zero per C++23 standard; test with non-integer results from formula `((col - 1) / tab_stop_width + 1) * tab_stop_width + 1`)
- [ ] T045 [US3] Ensure combining characters are counted individually (no grapheme cluster logic - each code point = 1 column)
- [ ] T046 [US3] Ensure ZWJ emoji sequences are counted as separate code points (no special handling - each code point including ZWJ = 1 column)
- [ ] T047 [US3] Add minimum 1 caret guarantee in `marker_extents()` (if `caret_count == 0`, set to 1 per FR-013)
- [ ] T048 [US3] Run US3 tests and verify all PASS (`ctest -R "US3.*edge" --output-on-failure`)

**Checkpoint**: All user stories should now be independently functional - core Unicode handling, invalid UTF-8 detection, and edge cases all work correctly

---

## Phase 6: Polish & Cross-Cutting Concerns

**Purpose**: Improvements that affect multiple user stories

- [ ] T049 [P] Add backward compatibility test in `test/tests.cpp` (ASCII-only source produces byte-for-byte identical output per SC-002)
- [ ] T050 [P] Run all existing `ErrorReporter` tests to verify no regressions (`ctest -R "ErrorReporter" --output-on-failure`)
- [ ] T051 [P] Generate code coverage report for UnicodeColumn module (`gcovr -r .. --filter "src/jsav_Lib/error/*" --filter "include/jsav/error/*"`)
- [ ] T052 [P] Verify coverage ≥80% for `UnicodeColumn.cpp` and `ErrorReporter.cpp`
- [ ] T053 Run full test suite to ensure all tests pass (`ctest --output-on-failure`)
- [ ] T054 [P] Update quickstart.md with usage examples and troubleshooting guide (verify examples work)
- [ ] T055 [P] Add Doxygen comments to all public functions in `include/jsav/error/UnicodeColumn.hpp`
- [ ] T056 [P] Run clang-format on all modified files (`clang-format -i src/jsav_Lib/error/*.cpp include/jsav/error/*.hpp`)
- [ ] T057 [P] Run clang-tidy on all modified files (verify zero issues with `jsav_ENABLE_CLANG_TIDY=ON`)
- [ ] T058 [P] Run cppcheck on all modified files (verify zero issues with `jsav_ENABLE_CPPCHECK=ON`)
- [ ] T059 [P] Run AddressSanitizer test suite (verify zero memory leaks with `jsav_ENABLE_SANITIZER_ADDRESS=ON`)
- [ ] T060 [P] Run UndefinedBehaviorSanitizer test suite (verify zero UB violations with `jsav_ENABLE_SANITIZER_UNDEFINED=ON`)

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies - can start immediately
- **Foundational (Phase 2)**: Depends on Setup completion - BLOCKS all user stories
- **User Stories (Phase 3+)**: All depend on Foundational phase completion
  - User stories can then proceed in parallel (if staffed)
  - Or sequentially in priority order (P1 → P2 → P3)
- **Polish (Phase 6)**: Depends on all user stories being complete

### User Story Dependencies

- **User Story 1 (P1)**: Can start after Foundational (Phase 2) - No dependencies on other stories
- **User Story 2 (P2)**: Can start after Foundational (Phase 2) - Builds on US1 infrastructure but independently testable
- **User Story 3 (P3)**: Can start after Foundational (Phase 2) - Builds on US1/US2 infrastructure but independently testable

### Within Each User Story

- Tests MUST be written and FAIL before implementation (TDD approach)
- `visual_column()` and `marker_extents()` before `ErrorReporter` modifications
- Core UnicodeColumn functions before ErrorReporter integration
- Story complete before moving to next priority

### Parallel Opportunities

- All Setup tasks marked [P] can run in parallel (T003)
- All Foundational tasks marked [P] can run in parallel (T004, T005, T007, T008)
- Once Foundational phase completes, all user stories can start in parallel (if team capacity allows)
- All tests for a user story marked [P] can run in parallel
- `visual_column()` and `marker_extents()` implementation can run in parallel (different functions)
- Different user stories can be worked on in parallel by different team members

---

## Parallel Example: User Story 1

```bash
# Launch all tests for User Story 1 together:
Task: "Add test case UnicodeColumn_marker_alignment_Chinese in test/tests.cpp"
Task: "Add test case UnicodeColumn_marker_alignment_Greek in test/tests.cpp"
Task: "Add test case UnicodeColumn_marker_alignment_emoji in test/tests.cpp"
Task: "Add test case UnicodeColumn_detect_ansi_color_environment in test/tests.cpp"
Task: "Add test case UnicodeColumn_ansi_color_red_code in test/tests.cpp"
Task: "Add test case UnicodeColumn_ansi_color_fallback_monochrome in test/tests.cpp"
Task: "Add test case UnicodeColumn_detect_ansi_color_no_color_variants in test/tests.cpp"

# Launch all UnicodeColumn implementations for User Story 1 together:
Task: "Implement visual_column() function in src/jsav_Lib/error/UnicodeColumn.cpp"
Task: "Implement marker_extents() function in src/jsav_Lib/error/UnicodeColumn.cpp"
```

---

## Parallel Example: User Story 2

```bash
# Launch all invalid UTF-8 tests together:
Task: "Add test case UnicodeColumn_invalid_UTF8_detection in test/tests.cpp"
Task: "Add test case UnicodeColumn_invalid_UTF8_null_byte in test/tests.cpp"
Task: "Add test case UnicodeColumn_invalid_UTF8_overlong in test/tests.cpp"
Task: "Add test case UnicodeColumn_invalid_UTF8_surrogate in test/tests.cpp"
```

---

## Parallel Example: User Story 3

```bash
# Launch all edge case tests together:
Task: "Add test case UnicodeColumn_edge_case_empty_line in test/tests.cpp"
Task: "Add test case UnicodeColumn_edge_case_first_column in test/tests.cpp"
Task: "Add test case UnicodeColumn_edge_case_last_column in test/tests.cpp"
Task: "Add test case UnicodeColumn_edge_case_tab_expansion in test/tests.cpp"
Task: "Add test case UnicodeColumn_edge_case_BOM in test/tests.cpp"
Task: "Add test case UnicodeColumn_edge_case_combining_characters in test/tests.cpp"
Task: "Add test case UnicodeColumn_edge_case_ZWJ_emoji in test/tests.cpp"
Task: "Add test case UnicodeColumn_edge_case_line_length_limit in test/tests.cpp"
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Setup
2. Complete Phase 2: Foundational (CRITICAL - blocks all stories)
3. Complete Phase 3: User Story 1
4. **STOP and VALIDATE**: 
   - Run US1 tests (`ctest -R "US1" --output-on-failure`)
   - Manually test with Unicode source file (Chinese, Greek, emoji)
   - Verify caret alignment is correct
5. Deploy/demo if ready

### Incremental Delivery

1. Complete Setup + Foundational → Foundation ready
2. Add User Story 1 → Test independently → Deploy/Demo (MVP!)
3. Add User Story 2 → Test independently → Deploy/Demo
4. Add User Story 3 → Test independently → Deploy/Demo
5. Each story adds value without breaking previous stories

### Parallel Team Strategy

With multiple developers:

1. Team completes Setup + Foundational together
2. Once Foundational is done:
   - Developer A: User Story 1 (core Unicode handling)
   - Developer B: User Story 2 (invalid UTF-8 detection)
   - Developer C: User Story 3 (edge cases)
3. Stories complete and integrate independently
4. Team reconvenes for Phase 6 (Polish & Cross-Cutting)

---

## Task Summary

| Phase | Task Count | Description |
|-------|------------|-------------|
| **Phase 1: Setup** | 3 tasks | Project structure and build system |
| **Phase 2: Foundational** | 7 tasks | Core UnicodeColumn module and ErrorReporter extensions (includes NFR-003 ANSI color detection, FR-024 LineTracker integration) |
| **Phase 3: US1** | 16 tasks | Unicode marker alignment (MVP) + NFR-003 ANSI color tests |
| **Phase 4: US2** | 13 tasks | Invalid UTF-8 detection and reporting |
| **Phase 5: US3** | 17 tasks | Edge case handling |
| **Phase 6: Polish** | 12 tasks | Testing, coverage, documentation, static analysis |
| **TOTAL** | **68 tasks** | Complete feature implementation |

### Task Count per User Story

- **User Story 1 (P1)**: 16 tasks (T009–T019, T011a–T011d, T018, T018a) — **includes NFR-003 ANSI color support with comprehensive tests**
- **User Story 2 (P2)**: 13 tasks (T020–T032)
- **User Story 3 (P3)**: 17 tasks (T033–T048, T049–T052 in Polish)
- **Setup + Foundational**: 10 tasks (T001–T008, T004a, T006a)
- **Polish**: 12 tasks (T049–T060)

### Parallel Opportunities Identified

- **Phase 1**: T003 can run in parallel with T001, T002
- **Phase 2**: T004, T004a, T005, T006a, T007, T008 can all run in parallel (different files)
- **Phase 3**: T009, T010, T011, T011a, T011b, T011c, T011d (tests) can run in parallel; T013, T014 (implementations) can run in parallel; T018, T018a can run in parallel
- **Phase 4**: T020–T024 (tests) can run in parallel
- **Phase 5**: T033–T040 (tests) can run in parallel
- **Phase 6**: T049, T050, T051, T052 (coverage tests) can run in parallel; T056–T060 (static analysis) can run in parallel

### Independent Test Criteria for Each Story

- **User Story 1**: Compile source with Chinese/Greek/emoji characters, verify caret alignment under correct code points
- **User Story 2**: Provide invalid UTF-8 bytes, verify encoding error message with byte offset and line number
- **User Story 3**: Test each edge case (empty line, BOM, tabs, combining chars, ZWJ emoji, line length limit), verify expected behavior

### Suggested MVP Scope

**MVP = User Story 1 Only (Phase 3)**

- Core functionality: Unicode-aware error marker alignment
- Delivers immediate value: Correct positioning for multi-byte Unicode characters
- Independently testable: Can verify with Chinese/Greek/emoji source files
- Backward compatible: ASCII output unchanged
- Can be deployed/demoed without waiting for P2/P3

**MVP Test Criteria**:
```bash
# Create test file with Unicode
echo "let x = 你好;" > test_unicode.vn
# Compile and observe
./build/jsav test_unicode.vn
# Verify: Carets align under Chinese characters (not byte offsets)
```

---

## Notes

- [P] tasks = different files, no dependencies on incomplete tasks
- [Story] label maps task to specific user story for traceability
- Each user story should be independently completable and testable
- Verify tests fail before implementing (TDD approach)
- Commit after each task or logical group of 2-3 tasks
- Stop at any checkpoint to validate story independently
- Avoid: vague tasks, same file conflicts, cross-story dependencies that break independence
- **Constitution Check**: All tasks comply with AGENTS.md constitution (platform independence, VS 2026 compatibility, C++ Core Guidelines, zero new dependencies)

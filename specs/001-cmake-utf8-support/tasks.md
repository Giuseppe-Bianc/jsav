# Tasks: CMake UTF-8 Support

**Input**: Design documents from `/specs/001-cmake-utf8-support/`
**Prerequisites**: plan.md (required), spec.md (required for user stories), research.md, data-model.md, contracts/

**Tests**: Tests are INCLUDED in this feature as explicitly requested in spec.md (FR-009 diagnostic logging verification, SC-003a/b/c success criteria, and quickstart.md test integration steps).

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Path Conventions

- **Single project**: `src/`, `test/`, `cmake/`, `configured_files/` at repository root
- Paths follow existing jsav project structure per plan.md

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project initialization and UTF-8 infrastructure preparation

**Note**: This phase is minimal since UTF-8 support integrates into existing jsav build system.

- [ ] T001 Verify project structure exists per plan.md (cmake/, configured_files/include/jsav/, src/, test/)
- [ ] T002 [P] Create cmake/ directory if not exists (repository root)
- [ ] T003 [P] Create configured_files/include/jsav/ directory if not exists (repository root)

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core UTF-8 infrastructure that MUST be complete before ANY user story can be implemented

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

- [ ] T004 Create cmake/Utf8Compiler.cmake with jsav_enable_utf8_compiler_flags() function per quickstart.md Step 1.1
- [ ] T005 Create cmake/Utf8Console.cmake with jsav_enable_utf8_console() function per quickstart.md Step 1.2
- [ ] T006 Create cmake/Utf8BomStrip.cmake with jsav_check_utf8_bom() function per quickstart.md Step 1.3
- [ ] T007 Create configured_files/include/jsav/utf8_console.hpp.in template per quickstart.md Step 2 and data-model.md Section 2.1
- [ ] T008 Modify ProjectOptions.cmake: Add jsav_ENABLE_UTF8 option in jsav_setup_options() macro per quickstart.md Step 3.1
- [ ] T009 Modify ProjectOptions.cmake: Wire UTF-8 modules into jsav_local_options() macro per quickstart.md Step 3.2

**Checkpoint**: Foundation ready - user story implementation can now begin in parallel

---

## Phase 3: User Story 1 - Build Project with UTF-8 Source Files (Priority: P1) 🎯 MVP

**Goal**: Enable compilation of source files containing UTF-8 encoded characters (comments, string literals) with zero encoding-related build errors.

**Independent Test**: Can build project containing UTF-8 characters in source files on Windows, Linux, macOS with successful compilation (exit code 0, no encoding warnings).

**Mapped Requirements**: FR-001 (compiler flags), FR-005 (byte preservation), FR-007 (BOM handling), FR-008 (UTF-8 requirement), SC-001 (build success)

### Tests for User Story 1 ⚠️

> **NOTE: Write these tests FIRST, ensure they FAIL before implementation**

- [ ] T011 [P] [US1] Add UTF-8 string literal preservation test in test/tests.cpp with `[utf8]` tag per quickstart.md Step 5 (verify strlen > expected for multi-byte, verify substring search works)
- [ ] T012 [P] [US1] Add BOM absence verification test in test/tests.cpp that builds file with BOM and checks preprocessed output per data-model.md Section 3.2. Use compiler flag `-E` (GCC/Clang) or `/EP` (MSVC) to generate preprocessed output; verify BOM bytes (EF BB BF) are absent from output
- [ ] T013 [US1] Add test verifying build succeeds without encoding warnings for UTF-8 source files in test/tests.cpp with `[utf8]` tag

### Implementation for User Story 1

- [ ] T014 [US1] Verify Utf8Compiler.cmake sets `/utf-8` for MSVC per research.md Decision 1 and contracts/cmake-api.md Section 2.1
- [ ] T015 [US1] Verify Utf8Compiler.cmake sets `-finput-charset=UTF-8 -fexec-charset=UTF-8` for GCC/Clang per research.md Decision 1 and contracts/cmake-api.md Section 2.1
- [ ] T016 [US1] Verify Utf8Compiler.cmake applies flags to jsav, jsav_lib, jsav_core_lib targets per quickstart.md Step 3.2
- [ ] T017 [US1] Verify Utf8BomStrip.cmake detects EF BB BF BOM pattern and logs warning per contracts/cmake-api.md Section 2.3
- [ ] T018 [US1] Add FR-009 diagnostic logging: CMake message() calls that log UTF-8 initialization status during build per spec.md FR-009. Log message format: `[UTF-8] Compiler flags: <flags> applied to target <target>` and `[UTF-8] BOM detected in <filename>: stripping` (if BOM found)
- [ ] T019 [US1] Update test/tests.cpp: Add test case for locale independence (UTF-8 works regardless of system locale) per quickstart.md Step 5

**Checkpoint**: User Story 1 complete - UTF-8 source files compile successfully, BOM handled, byte sequences preserved

---

## Phase 4: User Story 2 - Display UTF-8 in Build Output and Logs (Priority: P2)

**Goal**: Enable UTF-8 characters to display correctly in compiler output, build logs, and runtime console output without mojibake or replacement characters.

**Independent Test**: Build project that outputs UTF-8 strings and verify console/build output displays characters correctly on Windows, Linux, macOS.

**Mapped Requirements**: FR-002 (compiler output), FR-003 (console initialization), FR-004 (file paths), FR-006 (CMake files), FR-009 (diagnostics), SC-002 (build output), SC-003a/b/c (runtime output)

### Tests for User Story 2 ⚠️

- [ ] T020 [P] [US2] Add console initialization test in test/tests.cpp: Verify jsav::utf8::init_console() compiles and can be called without throwing per quickstart.md Step 5 and contracts/runtime-header-api.md Section 3
- [ ] T021 [P] [US2] Add UTF-8 console output test in test/tests.cpp: Output test strings (Latin-1, CJK, emoji, mixed) and verify no replacement characters per data-model.md Section 3.1
- [ ] T022 [US2] Add Windows-specific test: Verify SetConsoleOutputCP(CP_UTF8) is called when init_console() invoked on Windows per contracts/runtime-header-api.md Section 5.1
- [ ] T023 [US2] Add Linux/macOS test: Verify init_console() is no-op (returns immediately, no side effects) per contracts/runtime-header-api.md Section 5.2
- [ ] T024 [US2] Add fail-fast test: Mock SetConsoleOutputCP failure and verify error message contains "UTF-8 console initialization failed" per spec.md SC-003c and contracts/runtime-header-api.md Section 5.1

### Implementation for User Story 2

- [ ] T010 [US2] Modify src/jsav/main.cpp: Add #include <jsav/utf8/console.hpp> and call jsav::utf8::init_console() at start of main() per quickstart.md Step 4
- [ ] T025 [US2] Verify Utf8Console.cmake configures utf8_console.hpp from template using configure_file() per quickstart.md Step 1.2 and contracts/cmake-api.md Section 2.2
- [ ] T026 [US2] Verify Utf8Console.cmake adds configured_files/include to include directories per quickstart.md Step 1.2
- [ ] T027 [US2] Verify generated header uses correct include guard (JSAV_UTF8_CONSOLE_HPP) per data-model.md Section 2.1
- [ ] T028 [US2] Verify generated header includes <windows.h> only on _WIN32, <cstdlib>, <fmt/core.h> per data-model.md Section 2.1
- [ ] T029 [US2] Verify init_console() calls SetConsoleOutputCP(CP_UTF8) on Windows per research.md Decision 2 and contracts/runtime-header-api.md Section 3
- [ ] T030 [US2] Verify init_console() prints error to stderr and calls std::exit(EXIT_FAILURE) on Windows failure per spec.md FR-003 and contracts/runtime-header-api.md Section 5.1
- [ ] T031 [US2] Verify init_console() is empty (no-op) on Linux/macOS per research.md Decision 8 and contracts/runtime-header-api.md Section 5.2
- [ ] T032 [US2] Add FR-009 runtime diagnostic logging: Verify build logs show UTF-8 initialization status per spec.md FR-009. Runtime log message format (via fmt::print to stderr): `[UTF-8] Console initialized: CP=<codepage>` on success, or `[UTF-8] ERROR: Console initialization failed: <reason>` on failure
- [ ] T033 [US2] Verify CMakePresets.json files are saved as UTF-8 (check existing presets, convert if needed) per spec.md FR-006
- [ ] T034 [US2] Test UTF-8 in file paths: Create test with UTF-8 characters in file paths and verify build succeeds per spec.md FR-004

**Checkpoint**: User Story 2 complete - UTF-8 displays correctly in build output and runtime console output

---

## Phase 5: User Story 3 - Cross-Platform UTF-8 Consistency (Priority: P3)

**Goal**: Ensure UTF-8 handling works consistently on Windows (MSVC, GCC, Clang), Linux, and macOS with equivalent behavior and output.

**Independent Test**: Build same project on Windows, Linux, macOS and verify UTF-8 handling produces equivalent results on all platforms.

**Mapped Requirements**: FR-001 (cross-compiler flags), FR-003 (cross-platform console), SC-003a/b (cross-platform output), SC-004 (cross-platform paths), SC-005 (new developer experience), SC-006 (CI/CD)

### Tests for User Story 3 ⚠️

- [ ] T035 [P] [US3] Add cross-platform compiler flag test: Verify correct flags set for MSVC vs GCC vs Clang in test/tests.cpp per data-model.md Section 3.2
- [ ] T036 [P] [US3] Add platform detection test: Verify init_console() behavior differs correctly by platform (Windows vs Linux/macOS) in test/tests.cpp
- [ ] T037 [US3] Add CI/CD test scenario: Document and test UTF-8 build in CI environment per quickstart.md Troubleshooting section

### Implementation for User Story 3

- [ ] T038 [US3] Verify compiler detection logic in Utf8Compiler.cmake uses CMAKE_CXX_COMPILER_ID correctly per quickstart.md Step 1.1
- [ ] T039 [US3] Verify minimum compiler versions documented (GCC 13+, Clang 18+, MSVC 2022+) per spec.md Clarifications and research.md Summary table
- [ ] T040 [US3] Add CMake warning/error if unsupported compiler detected per contracts/cmake-api.md Section 6
- [ ] T041 [US3] Verify all 10 non-hidden CMakePresets.json presets automatically pick up UTF-8 flags: For each preset (windows-msvc-debug-developer-mode, windows-msvc-release-developer-mode, windows-msvc-debug-user-mode, windows-msvc-release-user-mode, windows-clang-debug, windows-clang-release, unixlike-gcc-debug, unixlike-gcc-release, unixlike-clang-debug, unixlike-clang-release), configure build, compile successfully, and verify UTF-8 output displays correctly. Document test results per preset in a markdown table.
- [ ] T042 [US3] Document cross-platform behavior in quickstart.md: Windows requires init_console(), Linux/macOS are no-op per quickstart.md "For Application Developers" section
- [ ] T043 [US3] Add troubleshooting section to quickstart.md for platform-specific issues per quickstart.md Troubleshooting section
- [ ] T044 [US3] Verify JSAV_UTF8_COMPILER_FLAGS, JSAV_UTF8_CONSOLE_HEADER, JSAV_UTF8_BOM_DETECTED cache variables set correctly per data-model.md Section 1.2

**Checkpoint**: User Story 3 complete - UTF-8 works consistently across all supported platforms and compilers

---

## Phase 6: Polish & Cross-Cutting Concerns

**Purpose**: Final validation, documentation, and integration verification

- [ ] T045 [P] Run full test suite: ctest --output-on-failure (verify all tests pass including [utf8] tests)
- [ ] T046 [P] Run static analysis: cmake --build build --target clang-tidy (verify zero UTF-8 related warnings)
- [ ] T047 [P] Run code coverage: gcovr -r . --config=gcovr.cfg (verify UTF-8 code paths covered)
- [ ] T048 [P] Verify quickstart.md validation: Follow quickstart.md Step 6 end-to-end and document any gaps
- [ ] T049 Update contracts/cmake-api.md: Verify all function signatures match implementation per contracts/cmake-api.md
- [ ] T050 Update contracts/runtime-header-api.md: Verify API documentation matches generated header per contracts/runtime-header-api.md
- [ ] T051 [P] Documentation updates: Verify all markdown files follow markdownlint rules per spec.md Constitution Check
- [ ] T052 [P] Clean rebuild verification: rm -rf build/ && cmake -S . -B build && cmake --build build (verify zero errors/warnings)
- [ ] T053 [P] UTF-8 output manual test: Run ./build/jsav --help and verify UTF-8 characters display correctly in terminal
- [ ] T054 [P] Commit all changes to feature branch 001-cmake-utf8-support per quickstart.md "Next Steps"

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies - can start immediately
- **Foundational (Phase 2)**: Depends on Setup completion - **BLOCKS all user stories**
- **User Story 1 (Phase 3)**: Depends on Foundational phase completion only
- **User Story 2 (Phase 4)**: Depends on Foundational phase completion only (can run parallel to US1)
- **User Story 3 (Phase 5)**: Depends on Foundational phase completion only (can run parallel to US1/US2)
- **Polish (Phase 6)**: Depends on all user stories being complete

### User Story Dependencies

- **User Story 1 (P1)**: Can start after Foundational (Phase 2) - No dependencies on other stories
- **User Story 2 (P2)**: Can start after Foundational (Phase 2) - Independent of US1, can run in parallel
- **User Story 3 (P3)**: Can start after Foundational (Phase 2) - Independent of US1/US2, can run parallel

### Within Each User Story

- Tests MUST be written and FAIL before implementation tasks
- CMake modules before header template (T004-T006 before T007)
- Header template before main.cpp integration (T007 before T010, but T010 is in User Story 2)
- Core implementation before verification tests
- Story complete before moving to next priority (if sequential)

### Parallel Opportunities

- **Phase 1**: T001, T002, T003 can all run in parallel (different directories)
- **Phase 2**: T004, T005, T006 can run in parallel (different CMake modules); T007 can run parallel to T004-T006
- **Phase 2**: T008, T009 must be sequential (same file: ProjectOptions.cmake)
- **Phase 3**: T011, T012, T013 can run in parallel (different test sections)
- **Phase 3**: T014, T015, T016, T017 can run in parallel (different CMake modules/verification)
- **Phase 4**: T010, T020, T021, T022, T023, T024 can run in parallel (different files/test sections)
- **Phase 4**: T025-T034 can run in parallel (different verification tasks)
- **Phase 5**: T035, T036, T037 can run in parallel (different test scenarios)
- **Phase 5**: T038-T044 can run in parallel (different verification tasks)
- **Phase 6**: T045, T046, T047, T048, T052, T053, T054 can all run in parallel (independent verification)

---

## Parallel Example: User Story 1

```bash
# Launch all tests for User Story 1 together:
# (Can be done by different developers or in parallel CI jobs)
Task T011: "Add UTF-8 string literal preservation test in test/tests.cpp"
Task T012: "Add BOM absence verification test in test/tests.cpp"
Task T013: "Add test verifying build succeeds without encoding warnings"

# Launch all CMake module verification for User Story 1 together:
Task T014: "Verify Utf8Compiler.cmake sets /utf-8 for MSVC"
Task T015: "Verify Utf8Compiler.cmake sets -finput-charset=UTF-8 -fexec-charset=UTF-8 for GCC/Clang"
Task T016: "Verify Utf8Compiler.cmake applies flags to all targets"
Task T017: "Verify Utf8BomStrip.cmake detects BOM pattern"
```

---

## Parallel Example: User Story 2

```bash
# Launch all runtime header tests together:
Task T020: "Add console initialization test"
Task T021: "Add UTF-8 console output test"
Task T022: "Add Windows-specific SetConsoleOutputCP test"
Task T023: "Add Linux/macOS no-op test"
Task T024: "Add fail-fast error message test"

# Launch all header implementation verification together:
Task T025: "Verify Utf8Console.cmake configures header from template"
Task T026: "Verify include directories added"
Task T027: "Verify include guard"
Task T028: "Verify header includes"
Task T029: "Verify SetConsoleOutputCP call on Windows"
Task T030: "Verify fail-fast behavior"
Task T031: "Verify no-op on Linux/macOS"
```

---

## Parallel Example: User Story 3

```bash
# Launch all cross-platform tests together:
Task T035: "Add cross-platform compiler flag test"
Task T036: "Add platform detection test"
Task T037: "Add CI/CD test scenario"

# Launch all cross-platform verification together:
Task T038: "Verify compiler detection logic"
Task T039: "Verify minimum compiler versions documented"
Task T040: "Add unsupported compiler warning"
Task T041: "Verify all CMakePresets.json presets"
Task T042: "Document cross-platform behavior"
Task T043: "Add troubleshooting section"
Task T044: "Verify cache variables set correctly"
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Setup (T001-T003)
2. Complete Phase 2: Foundational (T004-T009) - **CRITICAL BLOCKER**
3. Complete Phase 3: User Story 1 (T011-T019)
4. **STOP and VALIDATE**:
   - Run: `ctest -R "\[utf8\]" --output-on-failure`
   - Verify: Build succeeds with UTF-8 source files
   - Verify: No encoding warnings
   - Verify: BOM handling works
5. Deploy/demo MVP if validation passes

### Incremental Delivery

1. **Foundation**: Complete Setup + Foundational (T001-T009) → UTF-8 infrastructure ready
2. **MVP (US1)**: Add User Story 1 (T011-T019) → Test independently → Deploy/Demo (UTF-8 compilation works!)
3. **Enhancement (US2)**: Add User Story 2 (T010, T020-T034) → Test independently → Deploy/Demo (UTF-8 output works!)
4. **Polish (US3)**: Add User Story 3 (T035-T044) → Test independently → Deploy/Demo (Cross-platform consistency!)
5. Each phase adds value without breaking previous phases

### Parallel Team Strategy

With multiple developers:

1. **Team completes Setup + Foundational together** (T001-T009)
   - Developer A: T004-T006 (CMake modules)
   - Developer B: T007 (header template)
   - Developer C: T008-T009 (ProjectOptions.cmake)
2. **Once Foundational is done, split by user story**:
   - Developer A: User Story 1 (T011-T019) - Compilation support
   - Developer B: User Story 2 (T020-T034) - Runtime console support
   - Developer C: User Story 2 (T010) + User Story 3 (T035-T044) - Main.cpp integration and cross-platform verification
3. **Stories complete and integrate independently** - no merge conflicts expected
4. **Reunite for Polish phase** (T045-T054)

---

## Task Summary

| Phase     | Description             | Task Count |
|-----------|-------------------------|------------|
| Phase 1   | Setup                   | 3          |
| Phase 2   | Foundational            | 6          |
| Phase 3   | User Story 1 (P1 - MVP) | 9          |
| Phase 4   | User Story 2 (P2)       | 16         |
| Phase 5   | User Story 3 (P3)       | 10         |
| Phase 6   | Polish & Cross-Cutting  | 10         |
| **Total** | **All phases**          | **54**     |

### Task Count per User Story

- **User Story 1**: 9 tasks (T011-T019) - UTF-8 compilation support
- **User Story 2**: 16 tasks (T010, T020-T034) - UTF-8 console output support (includes T010 main.cpp integration)
- **User Story 3**: 10 tasks (T035-T044) - Cross-platform consistency

### Parallel Opportunities Identified

- **Phase 1**: 3/3 tasks (100%) parallelizable
- **Phase 2**: 4/6 tasks (67%) parallelizable (T008-T009 must be sequential)
- **Phase 3**: 7/9 tasks (78%) parallelizable (T018-T019 depend on T014-T017)
- **Phase 4**: 15/16 tasks (94%) parallelizable (T010 depends on T007; T032 depends on T029-T031)
- **Phase 5**: 10/10 tasks (100%) parallelizable
- **Phase 6**: 10/10 tasks (100%) parallelizable

### Independent Test Criteria

- **User Story 1**: Build succeeds with UTF-8 source files, zero encoding warnings, BOM handled correctly
- **User Story 2**: Console output displays UTF-8 correctly (no mojibake), fail-fast on Windows initialization failure
- **User Story 3**: Equivalent UTF-8 behavior on Windows/Linux/macOS, all CMakePresets work

### Suggested MVP Scope

**MVP = User Story 1 Only** (T001-T019)

- UTF-8 source files compile successfully
- BOM detection and handling works
- Byte sequences preserved in compiled output
- Build logs show UTF-8 initialization status

**Why US1 is MVP**: Core functionality without which no other UTF-8 features matter. Developers can work with UTF-8 source code even if console output isn't perfect yet.

---

## Notes

- [P] tasks = different files, no dependencies on incomplete tasks
- [Story] label maps task to specific user story for traceability
- Each user story is independently completable and testable
- Verify tests fail before implementing (TDD approach for UTF-8 features)
- Commit after each task or logical group of parallel tasks
- Stop at each checkpoint (after US1, US2, US3) to validate independently
- **Avoid**: vague tasks, same-file conflicts, cross-story dependencies that break independence
- **Remember**: Linux/macOS init_console() is no-op by design - do not add unnecessary code
- **Remember**: Fail-fast on Windows is intentional - do not add fallback logic
- **Remember**: Legacy encodings are out of scope - rely on compiler errors only

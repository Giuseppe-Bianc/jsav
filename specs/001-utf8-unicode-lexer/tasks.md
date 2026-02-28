# Tasks: UTF-8 Unicode Lexer Support

**Input**: Design documents from `/specs/001-utf8-unicode-lexer/`
**Prerequisites**: plan.md (required), spec.md (required for user stories), research.md, data-model.md, contracts/lexer-interface.md

**Tests**: Tests are INCLUDED as this is a compiler component requiring comprehensive test coverage (≥95% branch coverage per spec.md success criteria).

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Path Conventions

- **Public headers**: `include/jsav/lexer/`
- **Implementation**: `src/jsav_Lib/lexer/`
- **Tests**: `test/constexpr_tests.cpp` (compile-time), `test/tests.cpp` (runtime)

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project initialization and basic structure

- [ ] T001 Create `include/jsav/lexer/` directory for public lexer headers
- [ ] T002 Create `src/jsav_Lib/lexer/` directory for lexer implementation
- [ ] T003 [P] Add `include/jsav/lexer/` to CMakeLists.txt header installation list
- [ ] T004 [P] Add `src/jsav_Lib/lexer/` sources to src/jsav_Lib/CMakeLists.txt

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core infrastructure that MUST be complete before ANY user story can be implemented

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

- [ ] T005 [P] Create `include/jsav/lexer/Token.hpp` with TokenType enum class (uint8_t underlying type)
- [ ] T006 [P] Create `include/jsav/lexer/Token.hpp` with LexicalError enum class (all 16 error types from data-model.md)
- [ ] T007 [P] Create `include/jsav/lexer/Token.hpp` with SourceLocation struct (byteOffset, line, column)
- [ ] T008 [P] Create `include/jsav/lexer/Token.hpp` with Token class (constexpr accessors, std::string_view text_)
- [ ] T009 [P] Create `include/jsav/lexer/Utf8Decoder.hpp` with constexpr decode() function signature
- [ ] T010 [P] Create `include/jsav/lexer/XidClassification.hpp` with XID_Start/XID_Continue range table structures
- [ ] T011 Create `src/jsav_Lib/lexer/Utf8Decoder.cpp` with UTF-8 state machine implementation (1-4 byte sequences)
- [ ] T012 Create `src/jsav_Lib/lexer/XidClassification.cpp` with XID range tables for 10 scripts (Latin, Greek, Cyrillic, Arabic, Hebrew, Devanagari, Bengali, Thai, CJK, Hangul)
- [ ] T013 [P] Add UTF-8 decoder and XID classifier headers to `include/jsav/headers.hpp` master include
- [ ] T014 [P] Add lexer sources to src/jsav_Lib/CMakeLists.txt build list

**Checkpoint**: Foundation ready - user story implementation can now begin in parallel

---

## Phase 3: User Story 1 - Write Source Code with Unicode Identifiers (Priority: P1) 🎯 MVP

**Goal**: Enable developers to write variable names, function names, and type names using characters from their native languages (Greek, Cyrillic, Arabic, Hebrew, Devanagari, CJK, etc.)

**Independent Test**: Can be fully tested by writing source files with non-ASCII identifiers and verifying the lexer correctly tokenizes them as single identifier tokens.

### Tests for User Story 1 ⚠️

> **NOTE: Write these tests FIRST, ensure they FAIL before implementation**

- [ ] T015 [P] [US1] Add constexpr test for UTF-8 decoder with valid 1-4 byte sequences in test/constexpr_tests.cpp
- [ ] T016 [P] [US1] Add constexpr test for XID_Start classification (Latin, Greek, Cyrillic) in test/constexpr_tests.cpp
- [ ] T017 [P] [US1] Add constexpr test for XID_Continue classification (including combining marks) in test/constexpr_tests.cpp
- [ ] T018 [US1] Add runtime test for Greek identifiers (αβγ, Συνάρτηση) in test/tests.cpp
- [ ] T019 [US1] Add runtime test for Cyrillic identifiers (переменная, функция) in test/tests.cpp
- [ ] T020 [US1] Add runtime test for CJK identifiers (变量，函数) in test/tests.cpp
- [ ] T021 [US1] Add runtime test for invalid identifier start (digit at start) in test/tests.cpp

### Implementation for User Story 1

- [ ] T022 [P] [US1] Implement XidClassifier::isXidStart() with binary search over range tables in src/jsav_Lib/lexer/XidClassification.cpp
- [ ] T023 [P] [US1] Implement XidClassifier::isXidContinue() with binary search over range tables in src/jsav_Lib/lexer/XidClassification.cpp
- [ ] T024 [P] [US1] Implement XidClassifier::classifyIdentifier() to distinguish IdentifierAscii vs IdentifierUnicode in src/jsav_Lib/lexer/XidClassification.cpp
- [ ] T025 [US1] Create `include/jsav/lexer/Lexer.hpp` with Lexer class declaration (owning and non-owning constructors)
- [ ] T026 [US1] Implement Lexer::lexIdentifier() to recognize IdentifierAscii and IdentifierUnicode in src/jsav_Lib/lexer/Lexer.cpp
- [ ] T027 [US1] Implement Lexer::skipWhitespaceAndComments() to handle whitespace and prepare for tokenization in src/jsav_Lib/lexer/Lexer.cpp
- [ ] T028 [US1] Implement Lexer::tokenize() main loop with identifier tokenization in src/jsav_Lib/lexer/Lexer.cpp
- [ ] T029 [US1] Add error token insertion for invalid identifier start in src/jsav_Lib/lexer/Lexer.cpp
- [ ] T030 [US1] Add logging for User Story 1 identifier lexing operations in src/jsav_Lib/lexer/Lexer.cpp

**Checkpoint**: At this point, User Story 1 should be fully functional and testable independently

---

## Phase 4: User Story 2 - Use Unicode Characters in String and Character Literals (Priority: P2)

**Goal**: Enable developers to include any Unicode character in string and character literals, either directly or via escape sequences (\uXXXX, \UXXXXXXXX), for international text, emojis, and special symbols.

**Independent Test**: Can be fully tested by writing string/character literals with various Unicode content and verifying correct tokenization and value representation.

### Tests for User Story 2 ⚠️

- [ ] T031 [P] [US2] Add constexpr test for \uXXXX escape sequence decoding (valid BMP code points) in test/constexpr_tests.cpp
- [ ] T032 [P] [US2] Add constexpr test for \UXXXXXXXX escape sequence decoding (supplementary code points) in test/constexpr_tests.cpp
- [ ] T033 [P] [US2] Add constexpr test for invalid escape sequences (non-hex digits, surrogate halves) in test/constexpr_tests.cpp
- [ ] T034 [US2] Add runtime test for multi-byte UTF-8 string literals ("Hello 世界", "Привет") in test/tests.cpp
- [ ] T035 [US2] Add runtime test for \uXXXX escapes in string literals in test/tests.cpp
- [ ] T036 [US2] Add runtime test for \UXXXXXXXX escapes (emoji) in string literals in test/tests.cpp
- [ ] T037 [US2] Add runtime test for Unicode character literals ('α', '中') in test/tests.cpp
- [ ] T038 [US2] Add runtime test for mixed direct UTF-8 and escape sequences in test/tests.cpp
- [ ] T039 [US2] Add runtime test for invalid escape sequences (\uGGGG, \uDC00) in test/tests.cpp

### Implementation for User Story 2

- [ ] T040 [P] [US2] Create `include/jsav/lexer/EscapeDecoder.hpp` with constexpr decodeUnicode() function signature in include/jsav/lexer/EscapeDecoder.hpp
- [ ] T041 [P] [US2] Implement EscapeDecoder::decodeUnicode() for \uXXXX (4 hex digits) in src/jsav_Lib/lexer/EscapeDecoder.cpp
- [ ] T042 [P] [US2] Implement EscapeDecoder::decodeUnicode() for \UXXXXXXXX (8 hex digits) in src/jsav_Lib/lexer/EscapeDecoder.cpp
- [ ] T043 [P] [US2] Implement EscapeDecoder::decodeSimple() for standard escapes (\n, \t, \r, \\, \", \') in src/jsav_Lib/lexer/EscapeDecoder.cpp
- [ ] T044 [US2] Implement Lexer::lexStringLiteral() with UTF-8 and escape sequence handling in src/jsav_Lib/lexer/Lexer.cpp
- [ ] T045 [US2] Implement Lexer::lexCharacterLiteral() with UTF-8 and escape sequence handling in src/jsav_Lib/lexer/Lexer.cpp
- [ ] T046 [US2] Add error token insertion for unterminated string/character literals in src/jsav_Lib/lexer/Lexer.cpp
- [ ] T047 [US2] Add error token insertion for invalid escape sequences in src/jsav_Lib/lexer/Lexer.cpp
- [ ] T048 [US2] Integrate escape decoder into string/character literal lexing in src/jsav_Lib/lexer/Lexer.cpp
- [ ] T049 [US2] Add logging for User Story 2 literal lexing operations in src/jsav_Lib/lexer/Lexer.cpp

**Checkpoint**: At this point, User Stories 1 AND 2 should both work independently

---

## Phase 5: User Story 3 - Write Comments with Unicode Content (Priority: P3)

**Goal**: Enable developers to write comments using their native language characters for natural code documentation.

**Independent Test**: Can be fully tested by writing source files with Unicode content in both line (`//`) and block (`/* */`) comments and verifying correct comment recognition.

### Tests for User Story 3 ⚠️

- [ ] T050 [P] [US3] Add runtime test for line comments with multi-byte UTF-8 (// Это комментарий) in test/tests.cpp
- [ ] T051 [P] [US3] Add runtime test for block comments with multi-byte UTF-8 (/* Комментарий */) in test/tests.cpp
- [ ] T052 [US3] Add runtime test for multi-line block comments with Unicode and newlines in test/tests.cpp

### Implementation for User Story 3

- [ ] T053 [P] [US3] Implement Lexer::skipLineComment() to handle // comments with UTF-8 content in src/jsav_Lib/lexer/Lexer.cpp
- [ ] T054 [P] [US3] Implement Lexer::skipBlockComment() to handle /* */ comments with UTF-8 content in src/jsav_Lib/lexer/Lexer.cpp
- [ ] T055 [US3] Add error token insertion for unterminated block comments in src/jsav_Lib/lexer/Lexer.cpp
- [ ] T056 [US3] Integrate comment skipping into main tokenization loop in src/jsav_Lib/lexer/Lexer.cpp
- [ ] T057 [US3] Add logging for User Story 3 comment processing in src/jsav_Lib/lexer/Lexer.cpp

**Checkpoint**: At this point, User Stories 1, 2, AND 3 should all work independently

---

## Phase 6: User Story 4 - Receive Accurate Error Reports for Invalid UTF-8 (Priority: P4)

**Goal**: Provide clear error messages with accurate source positions when the lexer encounters invalid UTF-8 sequences (malformed bytes, overlong encodings, surrogate halves).

**Independent Test**: Can be fully tested by providing source files with various invalid UTF-8 sequences and verifying error messages include correct byte positions.

### Tests for User Story 4 ⚠️

- [ ] T058 [P] [US4] Add constexpr test for invalid UTF-8 start bytes (0xC0, 0xC1, 0xF5-0xFF) in test/constexpr_tests.cpp
- [ ] T059 [P] [US4] Add constexpr test for incomplete UTF-8 sequences at EOF in test/constexpr_tests.cpp
- [ ] T060 [P] [US4] Add constexpr test for overlong UTF-8 encodings in test/constexpr_tests.cpp
- [ ] T061 [P] [US4] Add constexpr test for surrogate halves (U+D800-U+DFFF) in test/constexpr_tests.cpp
- [ ] T062 [US4] Add runtime test for malformed UTF-8 (continuation byte without start byte) in test/tests.cpp
- [ ] T063 [US4] Add runtime test for overlong UTF-8 encoding (2-byte sequence for ASCII) in test/tests.cpp
- [ ] T064 [US4] Add runtime test for unpaired surrogate half in source in test/tests.cpp
- [ ] T065 [US4] Add runtime test for invalid UTF-8 followed by valid code (error recovery) in test/tests.cpp

### Implementation for User Story 4

- [ ] T066 [P] [US4] Enhance Utf8Decoder::decode() to detect and report invalid start bytes in src/jsav_Lib/lexer/Utf8Decoder.cpp
- [ ] T067 [P] [US4] Enhance Utf8Decoder::decode() to detect and report incomplete sequences in src/jsav_Lib/lexer/Utf8Decoder.cpp
- [ ] T068 [P] [US4] Enhance Utf8Decoder::decode() to detect and report overlong encodings in src/jsav_Lib/lexer/Utf8Decoder.cpp
- [ ] T069 [P] [US4] Enhance Utf8Decoder::decode() to detect and report surrogate halves in src/jsav_Lib/lexer/Utf8Decoder.cpp
- [ ] T070 [US4] Implement Lexer::advancePastError() for error recovery in src/jsav_Lib/lexer/Lexer.cpp
- [ ] T071 [US4] Integrate UTF-8 error detection into identifier lexing in src/jsav_Lib/lexer/Lexer.cpp
- [ ] T072 [US4] Integrate UTF-8 error detection into literal lexing in src/jsav_Lib/lexer/Lexer.cpp
- [ ] T073 [US4] Add accurate byte offset tracking for error reporting in src/jsav_Lib/lexer/Lexer.cpp
- [ ] T074 [US4] Add logging for User Story 4 error detection in src/jsav_Lib/lexer/Lexer.cpp

**Checkpoint**: At this point, User Stories 1-4 should all work independently with comprehensive error reporting

---

## Phase 7: User Story 5 - Consistent Line Ending Handling Across Platforms (Priority: P5)

**Goal**: Handle source files with Unix (LF), Windows (CRLF), or legacy Mac (CR) line endings correctly for cross-platform compatibility.

**Independent Test**: Can be fully tested by providing identical source files with different line ending conventions and verifying consistent tokenization.

### Tests for User Story 5 ⚠️

- [ ] T075 [P] [US5] Add runtime test for Unix LF line endings in test/tests.cpp
- [ ] T076 [P] [US5] Add runtime test for Windows CRLF line endings in test/tests.cpp
- [ ] T077 [P] [US5] Add runtime test for legacy Mac CR line endings in test/tests.cpp
- [ ] T078 [US5] Add runtime test for mixed line ending styles in test/tests.cpp

### Implementation for User Story 5

- [ ] T079 [P] [US5] Implement Lexer::advanceLine() to handle LF, CRLF, and CR in src/jsav_Lib/lexer/Lexer.cpp
- [ ] T080 [P] [US5] Implement Lexer::locationFromOffset() for line/column calculation in src/jsav_Lib/lexer/Lexer.cpp
- [ ] T081 [US5] Integrate line ending handling into main tokenization loop in src/jsav_Lib/lexer/Lexer.cpp
- [ ] T082 [US5] Add line number tracking to Lexer internal state in src/jsav_Lib/lexer/Lexer.cpp
- [ ] T083 [US5] Add logging for User Story 5 line ending handling in src/jsav_Lib/lexer/Lexer.cpp

**Checkpoint**: All user stories should now be independently functional

---

## Phase 8: Polish & Cross-Cutting Concerns

**Purpose**: Improvements that affect multiple user stories

- [ ] T084 [P] Add comprehensive documentation comments to all public headers in include/jsav/lexer/
- [ ] T085 [P] Run clang-tidy on all lexer code and fix all warnings
- [ ] T086 [P] Run cppcheck on all lexer code and fix all warnings
- [ ] T087 [P] Run gcovr to verify ≥95% branch coverage
- [ ] T088 [P] Add performance benchmark for UTF-8 slowdown ratio (target ≤1.10) in test/tests.cpp
- [ ] T089 [P] Update quickstart.md with working examples from all user stories
- [ ] T090 [P] Verify all contract requirements from contracts/lexer-interface.md are met
- [ ] T091 [P] Code cleanup and refactoring for ≤15 cyclomatic complexity per function
- [ ] T092 [P] Code cleanup and refactoring for ≤100 lines per function
- [ ] T093 [P] Add integration test for full file tokenization with mixed scripts in test/tests.cpp
- [ ] T094 [P] Add large file performance test (≥10k lines) in test/tests.cpp

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies - can start immediately
- **Foundational (Phase 2)**: Depends on Setup completion - BLOCKS all user stories
- **User Stories (Phase 3-7)**: All depend on Foundational phase completion
    - User stories can then proceed in parallel (if staffed)
    - Or sequentially in priority order (P1 → P2 → P3 → P4 → P5)
- **Polish (Phase 8)**: Depends on all desired user stories being complete

### User Story Dependencies

- **User Story 1 (P1)**: Can start after Foundational (Phase 2) - No dependencies on other stories
- **User Story 2 (P2)**: Can start after Foundational (Phase 2) - Independent of US1
- **User Story 3 (P3)**: Can start after Foundational (Phase 2) - Independent of US1/US2
- **User Story 4 (P4)**: Can start after Foundational (Phase 2) - Independent but enhances all stories
- **User Story 5 (P5)**: Can start after Foundational (Phase 2) - Independent infrastructure

### Within Each User Story

- Tests MUST be written and FAIL before implementation (TDD approach)
- Utility implementations (XidClassifier, Utf8Decoder, EscapeDecoder) before Lexer integration
- Core lexing (identifiers, literals, comments) before error handling
- Error handling before line ending infrastructure
- Story complete before moving to next priority

### Parallel Opportunities

- **Setup phase**: T003, T004 can run in parallel
- **Foundational phase**: T005-T010, T013-T014 can run in parallel (different headers)
- **User Story 1**: T015-T017 (constexpr tests) can run in parallel; T022-T024 (XID classifier) can run in parallel
- **User Story 2**: T031-T033 (constexpr tests) can run in parallel; T040-T043 (EscapeDecoder) can run in parallel
- **User Story 3**: T050-T051 (tests) can run in parallel; T053-T054 (comment handling) can run in parallel
- **User Story 4**: T058-T061 (constexpr tests) can run in parallel; T066-T069 (Utf8Decoder enhancements) can run in parallel
- **User Story 5**: T075-T077 (tests) can run in parallel; T079-T080 (line ending utilities) can run in parallel
- **Polish phase**: T084-T094 can all run in parallel (different files)

---

## Parallel Example: User Story 1

```bash
# Launch all constexpr tests for User Story 1 together:
Task: "Add constexpr test for UTF-8 decoder with valid 1-4 byte sequences in test/constexpr_tests.cpp"
Task: "Add constexpr test for XID_Start classification (Latin, Greek, Cyrillic) in test/constexpr_tests.cpp"
Task: "Add constexpr test for XID_Continue classification (including combining marks) in test/constexpr_tests.cpp"

# Launch all XID classifier implementations together:
Task: "Implement XidClassifier::isXidStart() with binary search over range tables in src/jsav_Lib/lexer/XidClassification.cpp"
Task: "Implement XidClassifier::isXidContinue() with binary search over range tables in src/jsav_Lib/lexer/XidClassification.cpp"
Task: "Implement XidClassifier::classifyIdentifier() to distinguish IdentifierAscii vs IdentifierUnicode in src/jsav_Lib/lexer/XidClassification.cpp"
```

---

## Parallel Example: User Story 2

```bash
# Launch all constexpr tests for User Story 2 together:
Task: "Add constexpr test for \uXXXX escape sequence decoding (valid BMP code points) in test/constexpr_tests.cpp"
Task: "Add constexpr test for \UXXXXXXXX escape sequence decoding (supplementary code points) in test/constexpr_tests.cpp"
Task: "Add constexpr test for invalid escape sequences (non-hex digits, surrogate halves) in test/constexpr_tests.cpp"

# Launch all EscapeDecoder implementations together:
Task: "Implement EscapeDecoder::decodeUnicode() for \uXXXX (4 hex digits) in src/jsav_Lib/lexer/EscapeDecoder.cpp"
Task: "Implement EscapeDecoder::decodeUnicode() for \UXXXXXXXX (8 hex digits) in src/jsav_Lib/lexer/EscapeDecoder.cpp"
Task: "Implement EscapeDecoder::decodeSimple() for standard escapes (\n, \t, \r, \\, \", \') in src/jsav_Lib/lexer/EscapeDecoder.cpp"
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Setup
2. Complete Phase 2: Foundational (CRITICAL - blocks all stories)
3. Complete Phase 3: User Story 1
4. **STOP and VALIDATE**:
   - Run constexpr_tests (T015-T017) - must compile
   - Run tests (T018-T021) - must pass
   - Verify Greek, Cyrillic, CJK identifiers tokenize correctly
5. Deploy/demo if ready

### Incremental Delivery

1. Complete Setup + Foundational → Foundation ready (TokenType, Token, Utf8Decoder, XidClassification)
2. Add User Story 1 → Test independently → Deploy/Demo (MVP: Unicode identifiers!)
3. Add User Story 2 → Test independently → Deploy/Demo (Unicode literals + escapes)
4. Add User Story 3 → Test independently → Deploy/Demo (Unicode comments)
5. Add User Story 4 → Test independently → Deploy/Demo (comprehensive error reporting)
6. Add User Story 5 → Test independently → Deploy/Demo (cross-platform line endings)
7. Each story adds value without breaking previous stories

### Parallel Team Strategy

With multiple developers:

1. Team completes Setup + Foundational together
2. Once Foundational is done:
   - Developer A: User Story 1 (identifiers)
   - Developer B: User Story 2 (literals + escapes)
   - Developer C: User Story 3 (comments)
3. After US1-3 complete:
   - Developer A: User Story 4 (error reporting)
   - Developer B: User Story 5 (line endings)
4. All developers: Polish phase tasks

---

## Task Summary

| Phase | Description | Task Count |
|-------|-------------|------------|
| Phase 1 | Setup | 4 |
| Phase 2 | Foundational | 10 |
| Phase 3 | User Story 1 (P1) - Unicode Identifiers | 16 |
| Phase 4 | User Story 2 (P2) - Unicode Literals + Escapes | 19 |
| Phase 5 | User Story 3 (P3) - Unicode Comments | 8 |
| Phase 6 | User Story 4 (P4) - Error Reporting | 17 |
| Phase 7 | User Story 5 (P5) - Line Endings | 9 |
| Phase 8 | Polish & Cross-Cutting | 11 |
| **Total** | | **94** |

### Tasks per User Story

| User Story | Priority | Task IDs | Count |
|------------|----------|----------|-------|
| US1 | P1 | T015-T030 | 16 |
| US2 | P2 | T031-T049 | 19 |
| US3 | P3 | T050-T057 | 8 |
| US4 | P4 | T058-T074 | 17 |
| US5 | P5 | T075-T083 | 9 |

### Parallel Opportunities Identified

- **Setup**: 2 tasks can run in parallel (T003, T004)
- **Foundational**: 8 tasks can run in parallel (T005-T010, T013-T014)
- **US1**: 6 tasks can run in parallel (T015-T017, T022-T024)
- **US2**: 6 tasks can run in parallel (T031-T033, T040-T043)
- **US3**: 4 tasks can run in parallel (T050-T051, T053-T054)
- **US4**: 8 tasks can run in parallel (T058-T061, T066-T069)
- **US5**: 4 tasks can run in parallel (T075-T077, T079-T080)
- **Polish**: All 11 tasks can run in parallel (T084-T094)

### Independent Test Criteria

| User Story | Independent Test Criteria |
|------------|---------------------------|
| US1 | Source files with Greek (αβγ), Cyrillic (переменная), CJK (变量) identifiers tokenize as single tokens |
| US2 | String/character literals with direct UTF-8 and \uXXXX/\UXXXXXXXX escapes tokenize correctly |
| US3 | Line comments (//) and block comments (/* */) with Unicode content are recognized correctly |
| US4 | Invalid UTF-8 (malformed, overlong, surrogates) produce error tokens with accurate byte positions |
| US5 | Files with LF, CRLF, CR line endings produce consistent tokenization with correct line numbers |

### Suggested MVP Scope

**MVP = User Story 1 Only (P1)**

- Unicode identifiers from 10 scripts (Latin, Greek, Cyrillic, Arabic, Hebrew, Devanagari, Bengali, Thai, CJK, Hangul)
- IdentifierAscii and IdentifierUnicode token types
- Basic error token insertion for invalid identifier start
- ≥95% test coverage for identifier lexing

**MVP Deliverable**: Developers can write variable names, function names, and type names using their native scripts.

---

## Notes

- [P] tasks = different files, no dependencies on incomplete tasks
- [Story] label maps task to specific user story for traceability
- Each user story is independently completable and testable
- Tests follow TDD: write first, ensure failure, then implement
- Commit after each task or logical group of parallel tasks
- Stop at each checkpoint to validate story independently
- All tasks include exact file paths for immediate actionability
- Function complexity enforced: ≤15 cyclomatic, ≤100 lines (via lizard)
- Zero static analysis warnings target (clang-tidy, cppcheck)

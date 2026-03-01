# Tasks: Lexer Full UTF-8 Support

**Input**: Design documents from `/specs/001-lexer-utf8-support/`
**Prerequisites**: plan.md, spec.md, research.md, contracts/unicode-api.md, quickstart.md

**Tests**: Included — constitution mandates TDD (Red-Green-Refactor) for all code.

**Organization**: Tasks grouped by user story for independent implementation and testing.

## Format: `[ID] [P?] [Story?] Description`

- **[P]**: Can run in parallel (different files, no dependencies on incomplete tasks)
- **[Story]**: Which user story (US1–US5) — only in user story phases
- Exact file paths included in every implementation task

## Path Conventions

- **Headers**: `include/jsav/lexer/` and `include/jsav/lexer/unicode/` (new unicode module)
- **Sources**: `src/jsav_Lib/lexer/` (existing lexer), `src/jsav_Lib/lexer/unicode/` (new if needed)
- **Tests**: `test/constexpr_tests.cpp` (STATIC_REQUIRE), `test/tests.cpp` (runtime REQUIRE)
- **Scripts**: `scripts/` (offline generation tools)

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Create directory structure, generate Unicode tables, define types

- [ ] T001 Create directory `include/jsav/lexer/unicode/` for new Unicode headers
- [ ] T002 Create directory `src/jsav_Lib/lexer/unicode/` for new Unicode source files
- [ ] T003 Create directory `scripts/` for offline generation tools
- [ ] T004 Create Python generation script in `scripts/generate_unicode_tables.py` per research.md R-06 (downloads UnicodeData.txt for Unicode 16.0.0, parses General Categories, merges adjacent ranges for id_start/id_continue/whitespace, outputs formatted constexpr C++ header with static_assert size guards)
- [ ] T005 Run `scripts/generate_unicode_tables.py` and commit generated file to `include/jsav/lexer/unicode/UnicodeData.hpp`
- [ ] T006 Create `CodepointRange` struct and `Utf8Status` enum and `Utf8DecodeResult` struct in `include/jsav/lexer/unicode/Utf8.hpp` per contracts/unicode-api.md (types only, no function bodies yet)
- [ ] T007 Update `src/jsav_Lib/CMakeLists.txt` to include new `unicode/` source files in the jsav_Lib target
- [ ] T008 Verify project compiles with new empty headers and updated CMakeLists — run `ninja` in build directory

**Checkpoint**: Project compiles with new types and generated Unicode tables available. No behavior changes yet.

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Implement core constexpr functions that ALL user stories depend on

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

- [ ] T009 [P] Implement constexpr `decode_utf8()` dispatcher function in `include/jsav/lexer/unicode/Utf8.hpp` — ASCII fast-path (`if (first < 0x80) [[likely]]`), then delegate to sub-functions per sequence length per research.md R-01
- [ ] T010 [P] Implement constexpr `decode_2byte()` sub-function in `include/jsav/lexer/unicode/Utf8.hpp` — validates continuation byte, rejects overlong (0xC0–0xC1)
- [ ] T011 [P] Implement constexpr `decode_3byte()` sub-function in `include/jsav/lexer/unicode/Utf8.hpp` — validates continuation bytes, rejects overlong (E0 80–9F) and surrogates (ED A0–BF)
- [ ] T012 [P] Implement constexpr `decode_4byte()` sub-function in `include/jsav/lexer/unicode/Utf8.hpp` — validates continuation bytes, rejects overlong (F0 80–8F) and out-of-range (F4 90+)
- [ ] T013 Implement constexpr `is_valid_utf8_at()` convenience function in `include/jsav/lexer/unicode/Utf8.hpp` — delegates to decode_utf8 and checks status == Ok
- [ ] T014 [P] Implement constexpr `is_letter()` function in `include/jsav/lexer/unicode/UnicodeData.hpp` — ASCII fast-path then binary search via `std::upper_bound` on `id_start_ranges` per research.md R-04
- [ ] T015 [P] Implement constexpr `is_id_start()` function in `include/jsav/lexer/unicode/UnicodeData.hpp` — delegates to `is_letter(cp) || cp == U'_'`
- [ ] T016 [P] Implement constexpr `is_id_continue()` function in `include/jsav/lexer/unicode/UnicodeData.hpp` — ASCII fast-path then binary search on `id_continue_ranges` per research.md R-04
- [ ] T017 [P] Implement constexpr `is_unicode_whitespace()` function in `include/jsav/lexer/unicode/UnicodeData.hpp` — ASCII fast-path (U+0020 is Zs) then binary search on `whitespace_ranges`
- [ ] T018 Verify all foundational functions compile — run `ninja` in build directory

**Checkpoint**: Foundation ready — all constexpr decode and classify functions implemented. User story work can begin.

---

## Phase 3: User Story 1 — Correct Multi-Byte UTF-8 Decoding (Priority: P1) 🎯 MVP

**Goal**: The lexer correctly decodes all valid UTF-8 sequences (1–4 bytes) into Unicode code points

**Independent Test**: Feed source strings with known characters from all four byte-length categories; verify each code point is decoded correctly

### Tests for User Story 1

> **Write these tests FIRST — ensure they FAIL before implementation (TDD Red phase)**

- [ ] T019 [P] [US1] Add constexpr test `Utf8Decoder_AsciiChar_ReturnsOkWithByteLength1` in `test/constexpr_tests.cpp` — verify decode_utf8 returns {cp, 1, Ok} for ASCII byte (e.g., 'A' = 0x41)
- [ ] T020 [P] [US1] Add constexpr test `Utf8Decoder_TwoByteSequence_ReturnsCorrectCodepoint` in `test/constexpr_tests.cpp` — verify decode_utf8 for é (U+00E9 = 0xC3 0xA9) returns {0x00E9, 2, Ok}
- [ ] T021 [P] [US1] Add constexpr test `Utf8Decoder_ThreeByteSequence_ReturnsCorrectCodepoint` in `test/constexpr_tests.cpp` — verify decode_utf8 for 変 (U+5909 = 0xE5 0xA4 0x89) returns {0x5909, 3, Ok}
- [ ] T022 [P] [US1] Add constexpr test `Utf8Decoder_FourByteSequence_ReturnsCorrectCodepoint` in `test/constexpr_tests.cpp` — verify decode_utf8 for 𐍈 (U+10348 = 0xF0 0x90 0x8D 0x88) returns {0x10348, 4, Ok}
- [ ] T023 [P] [US1] Add constexpr test `Utf8Decoder_NullByte_ReturnsOkCodepointZero` in `test/constexpr_tests.cpp` — verify decode_utf8 for U+0000 returns {0, 1, Ok}
- [ ] T024 [P] [US1] Add constexpr test `Utf8Decoder_InterleavedAsciiAndMultibyte_AllDecodeCorrectly` in `test/constexpr_tests.cpp` — verify sequential decode over mixed ASCII + 2/3/4-byte string
- [ ] T025 [P] [US1] Add runtime test `Lexer_AsciiOnlySource_TokenizeCorrectly` in `test/tests.cpp` — verify tokenize produces correct tokens for pure ASCII input (baseline sanity)
- [ ] T026 [P] [US1] Add runtime test `Lexer_TwoByteIdentifier_ReturnsIdentifierUnicode` in `test/tests.cpp` — verify lexer tokenizes `Ω` (U+03A9) as IdentifierUnicode
- [ ] T027 [P] [US1] Add runtime test `Lexer_ThreeByteIdentifier_ReturnsIdentifierUnicode` in `test/tests.cpp` — verify lexer tokenizes `変量` as single IdentifierUnicode token
- [ ] T028 [P] [US1] Add runtime test `Lexer_FourByteIdentifier_ReturnsIdentifierUnicode` in `test/tests.cpp` — verify lexer tokenizes mathematical 𝑥 (U+1D465) as IdentifierUnicode

### Implementation for User Story 1

- [ ] T029 [US1] Update `Lexer::peek_codepoint()` in `src/jsav_Lib/lexer/Lexer.cpp` to use `jsv::unicode::decode_utf8()` — replace current unvalidated decode with validated version, return codepoint from result
- [ ] T030 [US1] Update `Lexer::advance_codepoint()` in `src/jsav_Lib/lexer/Lexer.cpp` to use `jsv::unicode::decode_utf8()` — advance m_pos by result.byte_length, handle newline, update m_column by byte count
- [ ] T031 [US1] Update `Lexer::utf8_byte_len()` in `src/jsav_Lib/lexer/Lexer.cpp` to delegate to `jsv::unicode::decode_utf8()` or remove in favor of direct decode result usage
- [ ] T032 [US1] Update `include/jsav/lexer/Lexer.hpp` — add `#include "unicode/Utf8.hpp"` and `#include "unicode/UnicodeData.hpp"`, remove `is_xid_start`/`is_xid_continue` declarations (will be replaced in US3)
- [ ] T033 [US1] Run `clang-format -i include/jsav/lexer/unicode/Utf8.hpp include/jsav/lexer/Lexer.hpp src/jsav_Lib/lexer/Lexer.cpp` and verify all tests pass with `ninja tests relaxed_constexpr_tests && ctest -R "unittests|relaxed_constexpr" --output-on-failure`

**Checkpoint**: UTF-8 decoding is validated. All 1/2/3/4-byte valid sequences produce correct code points. Tests green.

---

## Phase 4: User Story 2 — Malformed UTF-8 Sequence Handling (Priority: P1)

**Goal**: The lexer detects all malformed UTF-8 patterns, emits error tokens with diagnostics, and recovers via maximal subpart strategy

**Independent Test**: Feed deliberately malformed byte sequences; verify error tokens emitted and subsequent valid tokens unaffected

### Tests for User Story 2

> **Write these tests FIRST — ensure they FAIL before implementation (TDD Red phase)**

- [ ] T034 [P] [US2] Add constexpr test `Utf8Decoder_OverlongTwoByte_ReturnsOverlongError` in `test/constexpr_tests.cpp` — verify decode_utf8 for 0xC0 0xAF returns {U+FFFD, 1, Overlong}
- [ ] T035 [P] [US2] Add constexpr test `Utf8Decoder_OverlongThreeByte_ReturnsOverlongError` in `test/constexpr_tests.cpp` — verify decode_utf8 for 0xE0 0x80 0xAF returns {U+FFFD, 1, Overlong}
- [ ] T036 [P] [US2] Add constexpr test `Utf8Decoder_SurrogateHalf_ReturnsSurrogateError` in `test/constexpr_tests.cpp` — verify decode_utf8 for 0xED 0xA0 0x80 (U+D800) returns {U+FFFD, 1, Surrogate}
- [ ] T037 [P] [US2] Add constexpr test `Utf8Decoder_OutOfRange_ReturnsOutOfRangeError` in `test/constexpr_tests.cpp` — verify decode_utf8 for 0xF4 0x90 0x80 0x80 (U+110000) returns {U+FFFD, 1, OutOfRange}
- [ ] T038 [P] [US2] Add constexpr test `Utf8Decoder_OrphanedContinuation_ReturnsOrphanedError` in `test/constexpr_tests.cpp` — verify decode_utf8 for 0x80 returns {U+FFFD, 1, OrphanedContinuation}
- [ ] T039 [P] [US2] Add constexpr test `Utf8Decoder_TruncatedTwoByte_ReturnsTruncatedError` in `test/constexpr_tests.cpp` — verify decode_utf8 for 0xC3 at end of input returns {U+FFFD, 1, TruncatedSequence}
- [ ] T040 [P] [US2] Add constexpr test `Utf8Decoder_TruncatedThreeByte_ReturnsCorrectMaximalSubpart` in `test/constexpr_tests.cpp` — verify decode_utf8 for 0xE5 0xA4 at end of input returns {U+FFFD, 2, TruncatedSequence}
- [ ] T041 [P] [US2] Add constexpr test `Utf8Decoder_InvalidLeadByte_ReturnsInvalidLeadError` in `test/constexpr_tests.cpp` — verify decode_utf8 for 0xFF returns {U+FFFD, 1, InvalidLeadByte}
- [ ] T042 [P] [US2] Add constexpr test `Utf8Decoder_AllContinuationBytes_EachProducesOrphanedError` in `test/constexpr_tests.cpp` — verify sequential decode over 0x80 0x80 0x80 produces 3 separate OrphanedContinuation results
- [ ] T043 [P] [US2] Add runtime test `Lexer_MalformedOrphanedContinuation_EmitsErrorToken` in `test/tests.cpp` — verify lexer produces Error token for orphaned 0x80 byte
- [ ] T044 [P] [US2] Add runtime test `Lexer_MalformedOverlong_EmitsErrorToken` in `test/tests.cpp` — verify lexer produces Error token for overlong 0xC0 0xAF
- [ ] T045 [P] [US2] Add runtime test `Lexer_MalformedMidFile_ContinuesTokenizing` in `test/tests.cpp` — verify tokens after malformed sequence are correctly parsed (e.g., `\x80 var x` produces Error + KeywordVar + IdentifierAscii)
- [ ] T046 [P] [US2] Add runtime test `Lexer_MalformedInsideStringLiteral_EntireLiteralBecomesError` in `test/tests.cpp` — verify string literal containing 0xC0 0xAF produces single Error token per FR-021
- [ ] T047 [P] [US2] Add runtime test `Lexer_MalformedInsideCharLiteral_EntireLiteralBecomesError` in `test/tests.cpp` — verify char literal containing orphaned continuation produces Error token per FR-021

### Implementation for User Story 2

- [ ] T048 [US2] Add malformed-UTF-8 error handling in `Lexer::next_token()` in `src/jsav_Lib/lexer/Lexer.cpp` — when `decode_utf8` returns status != Ok for non-ASCII lead byte, emit error_token covering maximal subpart bytes with diagnostic message per FR-022
- [ ] T049 [US2] Add UTF-8 validation in `Lexer::scan_string_literal()` in `src/jsav_Lib/lexer/Lexer.cpp` — for non-ASCII bytes inside string, call decode_utf8; if status != Ok, mark entire literal as error token per FR-021
- [ ] T050 [US2] Add UTF-8 validation in `Lexer::scan_char_literal()` in `src/jsav_Lib/lexer/Lexer.cpp` — for non-ASCII bytes inside char literal, call decode_utf8; if status != Ok, mark entire literal as error token per FR-021
- [ ] T051 [US2] Run `clang-format -i src/jsav_Lib/lexer/Lexer.cpp` and verify all tests pass with `ninja tests relaxed_constexpr_tests && ctest -R "unittests|relaxed_constexpr" --output-on-failure`

**Checkpoint**: All malformed UTF-8 patterns produce appropriate error tokens. Error recovery works — tokens after errors parse correctly. Literals with malformed UTF-8 become error tokens.

---

## Phase 5: User Story 3 — Unicode Identifier Recognition (Priority: P2)

**Goal**: The lexer classifies characters by Unicode General Category (L for start, L+M+N for continue) enabling identifiers in all Unicode scripts

**Independent Test**: Provide identifiers from CJK, Cyrillic, Devanagari, Arabic, mathematical symbols; verify correct IdentifierUnicode tokens

### Tests for User Story 3

> **Write these tests FIRST — ensure they FAIL before implementation (TDD Red phase)**

- [ ] T052 [P] [US3] Add constexpr test `UnicodeClassifier_AsciiLetter_IsLetter` in `test/constexpr_tests.cpp` — verify is_letter returns true for 'A', 'z'
- [ ] T053 [P] [US3] Add constexpr test `UnicodeClassifier_CJKIdeograph_IsLetter` in `test/constexpr_tests.cpp` — verify is_letter returns true for U+5909 (変)
- [ ] T054 [P] [US3] Add constexpr test `UnicodeClassifier_CyrillicLetter_IsIdStart` in `test/constexpr_tests.cpp` — verify is_id_start returns true for U+0438 (и)
- [ ] T055 [P] [US3] Add constexpr test `UnicodeClassifier_DevanagariLetter_IsIdStart` in `test/constexpr_tests.cpp` — verify is_id_start returns true for U+0917 (ग)
- [ ] T056 [P] [US3] Add constexpr test `UnicodeClassifier_Underscore_IsIdStart` in `test/constexpr_tests.cpp` — verify is_id_start returns true for U+005F (_)
- [ ] T057 [P] [US3] Add constexpr test `UnicodeClassifier_CombiningMark_IsIdContinueNotStart` in `test/constexpr_tests.cpp` — verify is_id_continue returns true and is_id_start returns false for U+0303 (combining tilde)
- [ ] T058 [P] [US3] Add constexpr test `UnicodeClassifier_ArabicDigit_IsIdContinueNotStart` in `test/constexpr_tests.cpp` — verify is_id_continue returns true and is_id_start returns false for U+0660 (Arabic-Indic digit zero)
- [ ] T059 [P] [US3] Add constexpr test `UnicodeClassifier_Emoji_NotIdStartNotContinue` in `test/constexpr_tests.cpp` — verify is_id_start and is_id_continue both return false for U+1F600 (😀)
- [ ] T060 [P] [US3] Add constexpr test `UnicodeClassifier_MathItalic_IsLetter` in `test/constexpr_tests.cpp` — verify is_letter returns true for U+1D465 (𝑥, Mathematical Italic Small X, General Category Ll)
- [ ] T061 [P] [US3] Add runtime test `Lexer_CJKIdentifier_ReturnsIdentifierUnicode` in `test/tests.cpp` — verify lexer tokenizes `变量名` as single IdentifierUnicode
- [ ] T062 [P] [US3] Add runtime test `Lexer_CyrillicWithCombiningMark_ReturnsSingleIdentifier` in `test/tests.cpp` — verify lexer tokenizes `и̃мя` (Cyrillic + combining tilde) as single IdentifierUnicode
- [ ] T063 [P] [US3] Add runtime test `Lexer_DevanagariIdentifier_ReturnsIdentifierUnicode` in `test/tests.cpp` — verify lexer tokenizes `गणना` as single IdentifierUnicode
- [ ] T064 [P] [US3] Add runtime test `Lexer_UnderscoreUnicode_ReturnsIdentifierUnicode` in `test/tests.cpp` — verify lexer tokenizes `_变量` as single IdentifierUnicode per FR-018
- [ ] T065 [P] [US3] Add runtime test `Lexer_EmojiOutsideLiteral_ReturnsErrorToken` in `test/tests.cpp` — verify lexer emits error token with diagnostic "unexpected Unicode character U+1F600" per FR-022
- [ ] T066 [P] [US3] Add runtime test `Lexer_MarkAtIdentifierStart_NotRecognizedAsIdentifier` in `test/tests.cpp` — verify combining mark alone does not start an identifier per FR-012
- [ ] T067 [P] [US3] Add runtime test `Lexer_NumberAtIdentifierStart_NotRecognizedAsIdentifier` in `test/tests.cpp` — verify Unicode Number category character alone at start does not form identifier per FR-012

### Implementation for User Story 3

- [ ] T068 [US3] Replace `Lexer::is_xid_start()` body in `src/jsav_Lib/lexer/Lexer.cpp` with call to `jsv::unicode::is_id_start(static_cast<char32_t>(cp))` per research.md R-07
- [ ] T069 [US3] Replace `Lexer::is_xid_continue()` body in `src/jsav_Lib/lexer/Lexer.cpp` with call to `jsv::unicode::is_id_continue(static_cast<char32_t>(cp))` per research.md R-07
- [ ] T070 [US3] Update `Lexer::next_token()` non-ASCII branch in `src/jsav_Lib/lexer/Lexer.cpp` — for valid decoded codepoint that is NOT is_id_start and NOT is_unicode_whitespace, emit error token with diagnostic "unexpected Unicode character U+XXXX" per FR-022
- [ ] T071 [US3] Run `clang-format -i src/jsav_Lib/lexer/Lexer.cpp include/jsav/lexer/Lexer.hpp` and verify all tests pass with `ninja tests relaxed_constexpr_tests && ctest -R "unittests|relaxed_constexpr" --output-on-failure`

**Checkpoint**: Identifiers from all Unicode scripts recognized. Emoji and non-identifier characters produce error tokens. Combining marks work as continue but not start.

---

## Phase 6: User Story 4 — ASCII Compatibility Preservation (Priority: P2)

**Goal**: All existing ASCII-only source files continue to tokenize identically after the UTF-8 changes

**Independent Test**: Run the complete existing test suite; verify zero regressions

### Tests for User Story 4

> **Write these tests FIRST — ensure they FAIL before implementation (TDD Red phase)**

- [ ] T072 [P] [US4] Add runtime test `Lexer_BOMAtStart_SkippedTransparently` in `test/tests.cpp` — verify tokenize with BOM prefix 0xEF 0xBB 0xBF followed by `var x` produces KeywordVar + IdentifierAscii + Eof (no BOM token) per FR-019
- [ ] T073 [P] [US4] Add runtime test `Lexer_UnicodeWhitespace_NoBreakSpace_ConsumedSilently` in `test/tests.cpp` — verify U+00A0 between tokens is consumed as whitespace per FR-023
- [ ] T074 [P] [US4] Add runtime test `Lexer_UnicodeWhitespace_EmSpace_ConsumedSilently` in `test/tests.cpp` — verify U+2003 EM SPACE between tokens is consumed as whitespace per FR-023
- [ ] T075 [P] [US4] Add runtime test `Lexer_UnicodeWhitespace_LineSeparator_ConsumedSilently` in `test/tests.cpp` — verify U+2028 LINE SEPARATOR is consumed as whitespace per FR-023
- [ ] T076 [P] [US4] Add runtime test `Lexer_AsciiOperators_UnchangedAfterUtf8` in `test/tests.cpp` — verify all ASCII operator sequences produce identical tokens as before (regression guard)
- [ ] T077 [P] [US4] Add runtime test `Lexer_AsciiKeywords_UnchangedAfterUtf8` in `test/tests.cpp` — verify all ASCII keywords produce identical TokenKind values (regression guard)
- [ ] T078 [P] [US4] Add runtime test `Lexer_AsciiStringLiteral_UnchangedAfterUtf8` in `test/tests.cpp` — verify ASCII string literals produce identical content (regression guard)

### Implementation for User Story 4

- [ ] T079 [US4] Add BOM detection at start of `Lexer::tokenize()` in `src/jsav_Lib/lexer/Lexer.cpp` — check if source starts with 0xEF 0xBB 0xBF and skip 3 bytes (advance m_pos and m_column by 3) per FR-019
- [ ] T080 [US4] Add Unicode whitespace recognition in `Lexer::skip_whitespace_and_comments()` in `src/jsav_Lib/lexer/Lexer.cpp` — for non-ASCII bytes, decode codepoint and check `jsv::unicode::is_unicode_whitespace()`, consume if true per FR-023
- [ ] T081 [US4] Run full existing test suite to verify zero regressions — `ninja tests relaxed_constexpr_tests && ctest -R "unittests|relaxed_constexpr" --output-on-failure` — all pre-existing tests MUST pass unchanged per FR-017

**Checkpoint**: BOM handling, Unicode whitespace, and complete ASCII backward compatibility verified. All pre-existing and new tests pass.

---

## Phase 7: User Story 5 — Efficient Text Processing (Priority: P3)

**Goal**: Tokenization performance remains within 10% of pre-change baseline for ASCII files; Unicode lookup is sub-linear

**Independent Test**: Measure tokenization time on large ASCII and mixed-Unicode files; compare against baseline

### Tests for User Story 5

- [ ] T082 [P] [US5] Add constexpr test `UnicodeClassifier_AsciiLetterFastPath_NoTableAccess` in `test/constexpr_tests.cpp` — verify is_id_start and is_id_continue return correct results for full ASCII range without relying on table (ensure ASCII fast-path exists)
- [ ] T083 [P] [US5] Add constexpr test `UnicodeClassifier_WhitespaceFastPath_SpaceIsZs` in `test/constexpr_tests.cpp` — verify is_unicode_whitespace returns true for U+0020 (Space, Zs) via fast-path
- [ ] T084 [P] [US5] Add runtime test `Lexer_LargeAsciiFile_TokenizesWithinBaseline` in `test/tests.cpp` — generate large ASCII input (~10K tokens), tokenize, and verify completion (functional correctness; timing is manual/CI)
- [ ] T085 [P] [US5] Add runtime test `Lexer_MixedUnicodeFile_TokenizesCompletely` in `test/tests.cpp` — generate input mixing ASCII + CJK + Cyrillic + emoji (in strings), tokenize, verify all tokens produced correctly

### Implementation for User Story 5

- [ ] T086 [US5] Review and verify ASCII fast-path in `decode_utf8()` in `include/jsav/lexer/unicode/Utf8.hpp` — confirm `[[likely]]` attribute on `if (first < 0x80)` branch per research.md R-01
- [ ] T087 [US5] Review and verify ASCII fast-path in `is_id_start()`, `is_id_continue()`, `is_unicode_whitespace()` in `include/jsav/lexer/unicode/UnicodeData.hpp` — confirm ASCII-range checks precede binary search
- [ ] T088 [US5] Verify `id_start_ranges` and `id_continue_ranges` arrays in `include/jsav/lexer/unicode/UnicodeData.hpp` are sorted and non-overlapping via static_assert or generator validation per research.md R-06
- [ ] T089 [US5] Run `clang-format -i include/jsav/lexer/unicode/Utf8.hpp include/jsav/lexer/unicode/UnicodeData.hpp` and verify all tests pass

**Checkpoint**: Performance characteristics verified. ASCII fast-paths confirmed. Lookup tables validated as sorted for O(log n) binary search.

---

## Phase 8: Polish & Cross-Cutting Concerns

**Purpose**: Final cleanup, formatting, documentation, and coverage verification

- [ ] T090 Remove old `Lexer::is_xid_start()` and `Lexer::is_xid_continue()` method declarations from `include/jsav/lexer/Lexer.hpp` and their definitions from `src/jsav_Lib/lexer/Lexer.cpp` (replaced by jsv::unicode functions)
- [ ] T091 Remove old `Lexer::utf8_byte_len()` static method from `include/jsav/lexer/Lexer.hpp` and `src/jsav_Lib/lexer/Lexer.cpp` if fully replaced by decode_utf8
- [ ] T092 [P] Update `Lexer::peek_codepoint()` return type from `std::uint32_t` to `char32_t` in `include/jsav/lexer/Lexer.hpp` and `src/jsav_Lib/lexer/Lexer.cpp` for type consistency with jsv::unicode API
- [ ] T093 [P] Update `Lexer::advance_codepoint()` return type from `std::uint32_t` to `char32_t` in `include/jsav/lexer/Lexer.hpp` and `src/jsav_Lib/lexer/Lexer.cpp`
- [ ] T094 Run `clang-format -i include/jsav/lexer/*.hpp include/jsav/lexer/unicode/*.hpp src/jsav_Lib/lexer/*.cpp` on all modified files
- [ ] T095 Run full test suite — `ninja tests relaxed_constexpr_tests && ctest -R "unittests|relaxed_constexpr" --output-on-failure` — all tests green
- [ ] T096 Run quickstart.md validation — follow build and verify steps from `specs/001-lexer-utf8-support/quickstart.md`

---

## Dependencies & Execution Order

### Phase Dependencies

```text
Phase 1: Setup ──────────────────────► no dependencies, start immediately
Phase 2: Foundational ───────────────► depends on Phase 1 (types + tables exist)
Phase 3: US1 (UTF-8 Decoding) ──────► depends on Phase 2 (decode functions)
Phase 4: US2 (Malformed Handling) ──► depends on Phase 3 (valid decode in place)
Phase 5: US3 (Unicode Identifiers) ─► depends on Phase 2 (classify functions)
Phase 6: US4 (ASCII Compat) ────────► depends on Phase 3 (decode integrated)
Phase 7: US5 (Performance) ─────────► depends on Phases 3–6 (all features complete)
Phase 8: Polish ─────────────────────► depends on all user stories complete
```

### User Story Dependencies

- **US1 (P1)**: Depends on Foundational only — no other story dependencies
- **US2 (P1)**: Depends on US1 (validated decode must be integrated before error paths make sense)
- **US3 (P2)**: Depends on Foundational only — can run in parallel with US1/US2 (different files for tests; Lexer.cpp changes are in different functions)
- **US4 (P2)**: Depends on US1 (BOM and whitespace handling requires decoded codepoints)
- **US5 (P3)**: Depends on US1–US4 (measures performance of final implementation)

### Within Each User Story

1. Tests MUST be written and FAIL before implementation (TDD Red phase)
2. Implementation follows test structure
3. Format and verify after each story completes

### Parallel Opportunities

**After Phase 2 completes:**

- US1 tests (T019–T028) and US3 tests (T052–T067) can be written in parallel — different test sections, different concerns
- US1 tests and US2 tests can be written in parallel — all are constexpr or runtime tests in separate sections

**After US1 implementation completes:**

- US2 implementation (T048–T051) and US4 implementation (T079–T081) can proceed in parallel — different functions in Lexer.cpp
- US3 implementation (T068–T071) can proceed in parallel with US2 — different functions (is_xid_start/continue vs malformed handling)

---

## Parallel Example: After Phase 2

```text
# Write all US1 + US3 constexpr tests in parallel (different test sections):
T019–T024 (US1 constexpr)  ║  T052–T060 (US3 constexpr)

# Write US1 + US2 + US3 runtime tests in parallel (different test sections):
T025–T028 (US1 runtime)  ║  T043–T047 (US2 runtime)  ║  T061–T067 (US3 runtime)

# After US1 impl: US2 + US3 + US4 implementations in parallel:
T048–T051 (US2 impl)  ║  T068–T071 (US3 impl)  ║  T079–T081 (US4 impl)
```

---

## Implementation Strategy

### MVP First (User Stories 1 + 2 Only)

1. Complete Phase 1: Setup (T001–T008)
2. Complete Phase 2: Foundational (T009–T018)
3. Complete Phase 3: US1 — UTF-8 Decoding (T019–T033)
4. Complete Phase 4: US2 — Malformed Handling (T034–T051)
5. **STOP and VALIDATE**: Core UTF-8 decoding + error recovery working

### Incremental Delivery

1. Setup + Foundational → types and functions ready
2. US1 → Valid UTF-8 decoding works → Test independently
3. US2 → Malformed input handled gracefully → Test independently
4. US3 → Unicode identifiers from all scripts → Test independently
5. US4 → BOM, Unicode whitespace, ASCII regression-free → Test independently
6. US5 → Performance verified → Test independently
7. Polish → Cleanup old code, final formatting

### Single Developer Strategy (This Project)

Execute sequentially in priority order:

1. Phase 1 + 2: Foundation (T001–T018)
2. Phase 3: US1 P1 (T019–T033) — MVP decoding
3. Phase 4: US2 P1 (T034–T051) — MVP error handling
4. Phase 5: US3 P2 (T052–T071) — Unicode identifiers
5. Phase 6: US4 P2 (T072–T081) — Compatibility
6. Phase 7: US5 P3 (T082–T089) — Performance
7. Phase 8: Polish (T090–T096) — Cleanup

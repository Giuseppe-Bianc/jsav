# Quickstart: Lexer Full UTF-8 Support

**Branch**: `001-lexer-utf8-support` | **Date**: 2026-03-01

## Prerequisites

- C++23 compiler: MSVC 19.44+ (VS 2026), GCC 13.2+, or Clang 17.0+
- CMake 3.28+, Ninja
- Python 3.10+ (only for regenerating Unicode tables, not for building)
- Existing project builds and all tests pass on `main`

## Build and Test

```bash
# Standard debug build (from repository root)
cd cmake-build-debug-visual-studio
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug \
      -Dmyproject_PACKAGING_MAINTAINER_MODE=OFF \
      -Dmyproject_ENABLE_COVERAGE=ON ..
ninja tests relaxed_constexpr_tests
ctest -R "unittests|relaxed_constexpr" --output-on-failure
```

## Implementation Order

### Step 1: Unicode tables generation

```bash
# Generate lookup tables from UnicodeData.txt (Unicode 16.0.0)
python scripts/generate_unicode_tables.py

# Output: include/jsav/lexer/unicode/UnicodeData.hpp
# Commit the generated file
```

### Step 2: New headers (TDD — write tests first)

1. Create `include/jsav/lexer/unicode/Utf8.hpp` — types + `decode_utf8()`
2. Create `include/jsav/lexer/unicode/UnicodeData.hpp` — generated tables + classifiers
3. Write constexpr tests in `test/constexpr_tests.cpp`:
   - `Utf8Decoder_AsciiChar_ReturnsOkWithByteLength1`
   - `Utf8Decoder_TwoByteSequence_ReturnsCorrectCodepoint`
   - `Utf8Decoder_OverlongTwoByte_ReturnsOverlongError`
   - `UnicodeClassifier_AsciiLetter_IsIdStart`
   - `UnicodeClassifier_CJKIdeograph_IsLetter`
4. Write runtime tests in `test/tests.cpp`:
   - `Lexer_Utf8BOM_SkippedTransparently`
   - `Lexer_MalformedOrphanedContinuation_EmitsErrorToken`
   - `Lexer_UnicodeIdentifier_CJK_ReturnsIdentifierUnicode`
   - `Lexer_UnicodeWhitespace_NoBreakSpace_ConsumedSilently`

### Step 3: Integrate into Lexer

1. Replace `Lexer::is_xid_start()` → `jsv::unicode::is_id_start()`
2. Replace `Lexer::is_xid_continue()` → `jsv::unicode::is_id_continue()`
3. Upgrade `peek_codepoint()` / `advance_codepoint()` to use `decode_utf8()`
4. Add BOM detection in `tokenize()`
5. Add Unicode whitespace in `skip_whitespace_and_comments()`
6. Add UTF-8 validation in `scan_string_literal()` / `scan_char_literal()`

### Step 4: Verify

```bash
# Run full test suite (must be green)
ninja tests relaxed_constexpr_tests
ctest -R "unittests|relaxed_constexpr" --output-on-failure

# Format changed files
clang-format -i include/jsav/lexer/unicode/*.hpp src/jsav_Lib/lexer/*.cpp

# Verify coverage
gcovr -r . --config=gcovr.cfg
```

## Key Files

| File | Role |
|------|------|
| `include/jsav/lexer/unicode/Utf8.hpp` | UTF-8 decode types and functions |
| `include/jsav/lexer/unicode/UnicodeData.hpp` | Generated constexpr lookup tables |
| `include/jsav/lexer/Lexer.hpp` | Updated method signatures |
| `src/jsav_Lib/lexer/Lexer.cpp` | Updated implementation |
| `scripts/generate_unicode_tables.py` | Offline table generator |
| `test/constexpr_tests.cpp` | Constexpr tests for decode + classify |
| `test/tests.cpp` | Runtime tests for full lexer UTF-8 behavior |

## Useful References

- Unicode Standard Chapter 3, Section 3.9 (Table 3-7: Well-Formed UTF-8 Byte Sequences)
- Unicode Standard Annex #44 (UnicodeData.txt format)
- cppreference: `std::upper_bound` (constexpr since C++20)
- cppreference: `std::expected` (C++23)

# Quickstart: Full UTF-8 Unicode Whitespace Support in Lexer

**Feature Branch**: `002-utf8-unicode-whitespace`
**Date**: 2026-03-02

## Prerequisites

- Visual Studio 2026 or GCC 13.x / Clang 17.x with C++23 support
- CMake 3.21+, Ninja build system
- Catch2 3.13.0 (fetched automatically by CPM)
- clang-format, clang-tidy, cppcheck (for CI gates)

## Build

```bash
# From repository root
git checkout 002-utf8-unicode-whitespace

mkdir -p build && cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -Djsav_PACKAGING_MAINTAINER_MODE=OFF \
  -Djsav_ENABLE_COVERAGE=ON ..
ninja
```

## Test

```bash
cd build
ninja tests relaxed_constexpr_tests constexpr_tests
ctest -R "unittests|relaxed_constexpr|constexpr" --output-on-failure
```

### Expected new test sections

**In `constexpr_tests.cpp`** (~6 new static_assert tests):

- `is_unicode_line_terminator` truth table: NEL → true, LINE SEP → true, PARA SEP → true,
  LF → false, CR → false, SPACE → false

**In `tests.cpp`** (~28 new runtime tests):

- **Token separation** (5 tests): Each of the 26 `\p{White_Space}` code points separates tokens
- **Line/column tracking** (8 tests): NEL, LINE SEP, PARA SEP as line terminators; multi-byte
  column advancement; CR not incrementing line; CR+LF → 1 increment; accumulated terminators
- **Robustness** (10 tests): Lone continuation bytes, truncated sequences, overlong, 0xFE, 0xFF,
  invalid continuation, surrogates, null bytes, valid non-whitespace multi-byte
- **Backward compatibility** (4 tests): ASCII whitespace, line comments, block comments, BOM
- **Benchmark** (1 test): 1 MB ASCII corpus, 100 iterations, ≤5% regression

## Files Changed

| File | Change | Net Lines |
|------|--------|-----------|
| `src/jsav_Lib/lexer/Lexer.cpp` | Add VT/FF to ASCII switch; add NEL handling in `skip_unicode_whitespace()`; use `is_unicode_line_terminator()` | ~15 |
| `include/jsav/lexer/unicode/UnicodeData.hpp` | Add `is_unicode_line_terminator()` function | ~10 |
| `test/constexpr_tests.cpp` | Add `is_unicode_line_terminator` truth table tests | ~20 |
| `test/tests.cpp` | Add 28 runtime tests + 1 benchmark | ~350 |

## Verification Checklist

1. All 26 `\p{White_Space}` code points separate tokens correctly
2. NEL, U+2028, U+2029 increment line counter; column resets to 1
3. VT and FF consumed as plain whitespace (no line increment)
4. CR does NOT increment line — CR+LF produces one increment via LF
5. Multi-byte whitespace advances column by byte count (2 or 3)
6. Invalid UTF-8 in whitespace position: no crash, no hang, not consumed
7. All existing tests pass unchanged
8. ASCII throughput regresses ≤5%
9. Zero ASan/UBSan findings
10. lizard: all functions ≤100 LOC, CCN ≤15, ≤6 params

## Architecture Decision: No New Decoder

The existing `unicode::decode_utf8()` in `Utf8.hpp` is reused. No new `decode_utf8()` is added
to `Lexer.cpp`. See [research.md](research.md#r-01-reuse-existing-decode_utf8-vs-new-decoder-in-lexercpp)
for rationale.

# Quickstart: Numeric Literal Lexer Feature

**Feature**: 003-numeric-literal-lexer
**Branch**: `003-numeric-literal-lexer`

## Prerequisites

- MSVC 2026 (Windows) or GCC/Clang with C++23 support
- CMake 4.2+, Ninja
- clang-format installed

## Environment setup

```powershell
# Clone and checkout branch
git checkout 003-numeric-literal-lexer

# Configure build (from project root)
cmake -S . -B ./build -Djsav_ENABLE_CLANG_TIDY:BOOL=OFF -Djsav_ENABLE_IPO:BOOL=OFF -Djsav_ENABLE_CPPCHECK:BOOL=OFF -DFMT_PEDANTIC:BOOL=ON -Djsav_ENABLE_SANITIZER_ADDRESS:BOOL=OFF
```

## Files involved

| File | Change type |
|------|-------------|
| `src/jsav_Lib/lexer/Lexer.cpp` | Rewrite `scan_numeric_literal()` + modify `next_token()` |
| `include/jsav/lexer/Lexer.hpp` | Add private helper declarations |
| `test/tests.cpp` | New TEST_CASE for all scenarios |
| `test/constexpr_tests.cpp` | Possible STATIC_REQUIRE tests for constexpr helpers |

## Development workflow (TDD)

### 1. Write failing tests

Add tests to `test/tests.cpp` following the structure:

```cpp
TEST_CASE("Lexer_NumericBaseFormats_TokenizeCorrectly", "[lexer][numeric][phase7]") {
    SECTION("integer 42 produces single Numeric token") {
        jsv::Lexer lex{"42", "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 2);  // Numeric + Eof
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "42");
    }
}
```

### 2. Build and verify tests red

```powershell
cd build
ninja tests relaxed_constexpr_tests
ctest -R "unittests|relaxed_constexpr" --output-on-failure
```

### 3. Implement minimum code

Modify `Lexer.cpp` to make tests pass.

### 4. Format

```powershell
clang-format -i src/jsav_Lib/lexer/Lexer.cpp include/jsav/lexer/Lexer.hpp test/tests.cpp test/constexpr_tests.cpp
```

### 5. Verify regression

```powershell
cd build
ninja tests relaxed_constexpr_tests
ctest -R "unittests|relaxed_constexpr" --output-on-failure
```

### 6. Verify complexity

```powershell
cmake --build build --target lizard
```

## Task priorities

| Priority | Task | Description |
|----------|------|-------------|
| P0 | Task 1 | Modify `next_token()` for leading-dot |
| P0 | Task 2 | Rewrite `scan_numeric_literal()` |
| P0 | Task 3 | Helper declarations in header |
| P1 | Task 4 | Runtime tests (5 user stories) |
| P1 | Task 5 | Compile-time tests |
| P2 | Task 6 | Complete regression suite |

## Quick verification

After full implementation, a quick smoke test:

```cpp
jsv::Lexer lex{"3.14e+2f .5 1e 42u8 1i", "test.jsav"};
auto tokens = lex.tokenize();
// Expected: Numeric("3.14e+2f"), Numeric(".5"), Numeric("1"), Identifier("e"),
//           Numeric("42u8"), Numeric("1"), Identifier("i"), Eof
```

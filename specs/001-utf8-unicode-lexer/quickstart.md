# Quickstart: UTF-8 Unicode Lexer

**Feature**: UTF-8 Unicode Lexer Support  
**Branch**: `001-utf8-unicode-lexer`  
**Date**: 2026-02-28  
**Status**: Ready for Implementation

---

## Overview

This quickstart guide provides step-by-step instructions for building, testing, and using the UTF-8 Unicode lexer component. Follow this guide to get started with the lexer implementation.

---

## Prerequisites

### Required Tools

- **C++23 Compiler**: GCC 13+, Clang 16+, or MSVC 2022+ (17.8+)
- **CMake**: 3.21+
- **Ninja**: Recommended build system

### Recommended Tools

- **clang-tidy**: Static analysis
- **cppcheck**: Static analysis
- **ccache**: Compilation caching
- **gcovr**: Coverage reporting

### Verify Installation

```bash
# Check compiler version
g++ --version    # GCC 13+
clang++ --version # Clang 16+
cl --version      # MSVC 19.38+

# Check CMake version
cmake --version   # 3.21+

# Check Ninja version
ninja --version
```

---

## Building the Lexer

### Step 1: Configure Build

From the repository root:

```bash
# Windows (MSVC)
cmake -S . --preset windows-msvc-debug-developer-mode

# Linux/macOS (GCC/Clang)
cmake -S . --preset unixlike-gcc-debug
```

### Step 2: Build

```bash
# Windows
cmake --build --preset windows-msvc-debug-developer-mode

# Linux/macOS
cmake --build --preset unixlike-gcc-debug
```

### Step 3: Run Tests

```bash
# Run all tests
ctest --preset test-windows-msvc-debug-developer-mode  # Windows
ctest --preset test-unixlike-gcc-debug                  # Linux/macOS

# Or from build directory
cd build
ctest --output-on-failure
```

---

## Using the Lexer

### Basic Example

```cpp
#include <jsav/lexer/Lexer.hpp>
#include <jsav/lexer/Token.hpp>
#include <fmt/format.h>
#include <iostream>

int main() {
    // Create lexer with source code
    std::string source = R"(
        let x = 42;
        let αβγ = 100;  // Greek identifier
        let имя = "Привет";  // Cyrillic with string
    )";
    
    jsav::lexer::Lexer lexer{std::move(source)};
    
    // Tokenize
    auto tokens = lexer.tokenize();
    
    // Print tokens
    for (const auto& token : tokens) {
        if (token.isError()) {
            std::cerr << fmt::format("Error at line {}: {}\n",
                                     token.location().line,
                                     std::string(token.text()));
        } else {
            std::cout << fmt::format("Token: {} Text: '{}' at L{}C{}\n",
                                     token.type(),
                                     std::string(token.text()),
                                     token.location().line,
                                     token.location().column);
        }
    }
    
    return 0;
}
```

### Expected Output

```text
Token: Whitespace Text: '
        ' at L1C0
Token: Keyword Text: 'let' at L2C8
Token: Whitespace Text: ' ' at L2C11
Token: IdentifierAscii Text: 'x' at L2C12
Token: Whitespace Text: ' ' at L2C13
Token: Assign Text: '=' at L2C14
Token: Whitespace Text: ' ' at L2C15
Token: IntegerLiteral Text: '42' at L2C16
Token: Semicolon Text: ';' at L2C18
Token: LineComment Text: '// Greek identifier' at L2C20
...
Token: IdentifierUnicode Text: 'αβγ' at L3C12
...
Token: IdentifierUnicode Text: 'имя' at L4C12
Token: StringLiteral Text: '"Привет"' at L4C16
...
Token: EndOfFile Text: '' at L5C0
```

---

## Unicode Identifier Examples

### Greek Identifiers

```cpp
std::string source = R"(
    let α = 1;
    let β = 2;
    let γ = α + β;
    let Συνάρτηση = 42;  // "function" in Greek
)";
```

### Cyrillic Identifiers

```cpp
std::string source = R"(
    let переменная = 10;  // "variable"
    let функция = 20;     // "function"
    let результат = переменная + функция;
)";
```

### CJK Identifiers

```cpp
std::string source = R"(
    let 变量 = 100;  // "variable" in Chinese
    let 函数 = 200;  // "function" in Chinese
    let 结果 = 变量 + 函数;
)";
```

### Mixed Script Identifiers

```cpp
std::string source = R"(
    let variable_α = 1;  // Latin + Greek
    let x_переменная = 2;  // Latin + Cyrillic
    let 变量_x = 3;  // CJK + Latin
)";
```

---

## Escape Sequence Examples

### Unicode Escapes in Strings

```cpp
std::string source = R"(
    let s1 = "\u0041\u0042\u0043";  // "ABC"
    let s2 = "\u03B1\u03B2\u03B3";  // "αβγ" (Greek)
    let s3 = "\U0001F600";  // "😀" (emoji)
    let s4 = "Hello\nWorld\t!";  // Standard escapes
)";
```

### Unicode Escapes in Characters

```cpp
std::string source = R"(
    let c1 = '\u0041';  // 'A'
    let c2 = '\u03B1';  // 'α'
    let c3 = '\U0001F600';  // '😀'
)";
```

---

## Error Handling Examples

### Invalid UTF-8

```cpp
std::string source = "let x = \xFF\xFE;";  // Invalid UTF-8 bytes

jsav::lexer::Lexer lexer{std::move(source)};
auto tokens = lexer.tokenize();

for (const auto& token : tokens) {
    if (token.isError()) {
        auto error = token.error().value();
        // error == LexicalError::InvalidUtf8StartByte
        std::cerr << fmt::format("UTF-8 error at byte {}\n", 
                                 token.location().byteOffset);
    }
}
```

### Invalid Escape Sequence

```cpp
std::string source = R"(let s = "\uGGGG";)";  // Invalid hex digits

jsav::lexer::Lexer lexer{std::move(source)};
auto tokens = lexer.tokenize();

for (const auto& token : tokens) {
    if (token.isError()) {
        auto error = token.error().value();
        // error == LexicalError::InvalidHexDigit
        std::cerr << fmt::format("Escape error at line {}\n",
                                 token.location().line);
    }
}
```

### Surrogate Half in Escape

```cpp
std::string source = R"(let s = "\uD800";)";  // Surrogate half

jsav::lexer::Lexer lexer{std::move(source)};
auto tokens = lexer.tokenize();

for (const auto& token : tokens) {
    if (token.isError()) {
        auto error = token.error().value();
        // error == LexicalError::SurrogateInEscape
    }
}
```

---

## Testing the Lexer

### Run Unit Tests

```bash
# From build directory
ctest -R "unittests" --output-on-failure

# Or run specific test executable
./build/test/tests
```

### Run Compile-Time Tests

```bash
# Compile-time tests (if it compiles, tests pass)
./build/test/constexpr_tests
```

### Generate Coverage Report

```bash
# Configure with coverage enabled
cmake -S . -B build -Djsav_ENABLE_COVERAGE=ON

# Build and run tests
cmake --build build
ctest --test-dir build

# Generate coverage report
gcovr -r . --config=gcovr.cfg

# View HTML report
# Open build/coverage/index.html in browser
```

### Check Coverage Thresholds

```bash
# Verify ≥95% branch coverage
gcovr -r . --branches --txt-metric branch
```

---

## Debugging the Lexer

### Enable Debug Logging

```cpp
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

int main() {
    // Set up debug logging
    auto console = spdlog::stdout_color_mt("lexer");
    console->set_level(spdlog::level::debug);
    
    std::string source = "let x = 42;";
    jsav::lexer::Lexer lexer{std::move(source)};
    
    // Tokenize with debug output
    auto tokens = lexer.tokenize();
    
    return 0;
}
```

### Inspect Token Vector

```cpp
void printTokenDetails(const jsav::lexer::Token& token) {
    fmt::print("Token:\n");
    fmt::print("  Type: {}\n", token.type());
    fmt::print("  Text: '{}'\n", std::string(token.text()));
    fmt::print("  Location: byte={}, line={}, column={}\n",
               token.location().byteOffset,
               token.location().line,
               token.location().column);
    
    if (token.isError()) {
        fmt::print("  Error: {}\n", token.error().value());
    }
}
```

---

## Performance Benchmarking

### Benchmark ASCII vs UTF-8

```cpp
#include <chrono>
#include <jsav/lexer/Lexer.hpp>

void benchmark() {
    // ASCII source
    std::string asciiSource = R"(
        let x = 1;
        let y = 2;
        let z = x + y;
    )";
    // ... repeat 1000x for measurable time
    
    // UTF-8 source (same structure, Unicode identifiers)
    std::string utf8Source = R"(
        let α = 1;
        let β = 2;
        let γ = α + β;
    )";
    // ... repeat 1000x
    
    auto start = std::chrono::high_resolution_clock::now();
    jsav::lexer::Lexer asciiLexer{asciiSource};
    auto asciiTokens = asciiLexer.tokenize();
    auto asciiEnd = std::chrono::high_resolution_clock::now();
    
    jsav::lexer::Lexer utf8Lexer{utf8Source};
    auto utf8Tokens = utf8Lexer.tokenize();
    auto utf8End = std::chrono::high_resolution_clock::now();
    
    auto asciiTime = std::chrono::duration_cast<std::chrono::microseconds>(
        asciiEnd - start).count();
    auto utf8Time = std::chrono::duration_cast<std::chrono::microseconds>(
        utf8End - asciiEnd).count();
    
    double slowdown = static_cast<double>(utf8Time) / asciiTime;
    fmt::print("ASCII time: {} µs\n", asciiTime);
    fmt::print("UTF-8 time: {} µs\n", utf8Time);
    fmt::print("Slowdown ratio: {:.2f}x\n", slowdown);
    
    // Verify slowdown ≤ 1.10 (10% slowdown target)
    if (slowdown > 1.10) {
        fmt::print("WARNING: Slowdown exceeds 10% target!\n");
    }
}
```

---

## Common Issues

### Issue 1: Source File Encoding

**Problem**: Source file with Unicode string literals causes compile errors.

**Solution**: Ensure source file is saved as UTF-8 without BOM:
```bash
# Check file encoding
file -I myfile.cpp

# Convert to UTF-8 if needed
iconv -f ISO-8859-1 -t UTF-8 myfile.cpp > myfile_utf8.cpp
```

### Issue 2: Console Output Encoding

**Problem**: Unicode tokens display incorrectly in console.

**Solution**: Set console to UTF-8 mode:
```bash
# Windows PowerShell
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8

# Linux/macOS
export LANG=en_US.UTF-8
```

### Issue 3: Linker Errors

**Problem**: Undefined reference to `jsav::lexer::Lexer` methods.

**Solution**: Link against jsav library:
```cmake
target_link_libraries(my_app PRIVATE jsav::jsav_lib)
```

### Issue 4: Static Analysis Warnings

**Problem**: clang-tidy reports warnings in lexer code.

**Solution**: Run clang-tidy with project configuration:
```bash
cmake --build build --target clang-tidy
```

---

## Next Steps

1. **Implement Lexer**: Follow tasks in `tasks.md` for implementation order
2. **Write Tests**: Add tests to `test/constexpr_tests.cpp` and `test/tests.cpp`
3. **Run Coverage**: Verify ≥95% branch coverage
4. **Static Analysis**: Ensure zero clang-tidy/cppcheck warnings
5. **Performance**: Benchmark UTF-8 slowdown ≤10%

---

## References

- Feature Specification: `/specs/001-utf8-unicode-lexer/spec.md`
- Data Model: `/specs/001-utf8-unicode-lexer/data-model.md`
- Interface Contract: `/specs/001-utf8-unicode-lexer/contracts/lexer-interface.md`
- Research Report: `/specs/001-utf8-unicode-lexer/research.md`
- Project README: `/README.md`
- Building Instructions: `/README_building.md`

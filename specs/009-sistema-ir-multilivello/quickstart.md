# Quickstart - Verifiable Multi-Level IR System

Terminology reference: `GLOSSARY.md`.

## Prerequisites
- Visual Studio 2026 with C++23 MSVC toolset.
- CMake 4.2+, Ninja.
- Project dependencies are already managed via CPM (no manual additions).

## 1) Configure (MSVC Debug)

```powershell
cmake -S . --preset windows-msvc-debug-developer-mode
```

## 2) Build

```powershell
cmake --build --preset windows-msvc-debug-developer-mode
```

## 3) Test Pyramid

### 3.1 Compile-time constexpr

```powershell
cmake --build --preset windows-msvc-debug-developer-mode --target constexpr_tests
ctest --preset test-windows-msvc-debug-developer-mode -R "constexpr"
```

### 3.2 Constexpr debug runtime

```powershell
cmake --build --preset windows-msvc-debug-developer-mode --target relaxed_constexpr_tests
ctest --preset test-windows-msvc-debug-developer-mode -R "relaxed_constexpr"
```

### 3.3 Full runtime tests

```powershell
cmake --build --preset windows-msvc-debug-developer-mode --target tests
ctest --preset test-windows-msvc-debug-developer-mode -R "unittests" --output-on-failure
```

## 4) Quality Gates (local mirror of CI)

```powershell
# Build zero-warning
cmake --build --preset windows-msvc-debug-developer-mode

# Static analysis (if enabled by preset)
# clang-tidy / cppcheck via CMake options

# Sanitizers (preset/toolchain support)
ctest --preset test-windows-msvc-debug-developer-mode --output-on-failure

# Complexity
lizard src/**/*.cpp include/**/*.hpp --CCN 15 --length 100 --arguments 6

# Coverage
gcovr -r . --config=gcovr.cfg
```

## 5) Implementation Notes
- Uniform error handling: use CompileError and std::expected<T, std::vector<CompileError>>.
- No new dependencies beyond the four approved ones.
- All transformations go through a transactional working copy with atomic commit.
- For may-alias, strict no-reorder unless a valid formal proof exists.

## 6) CI Pipeline Reference
GitHub Actions stages: `lint -> static-analysis -> build -> test -> sanitizers -> complexity -> coverage`.
Gate coverage target: `gcovr >= 95%`.

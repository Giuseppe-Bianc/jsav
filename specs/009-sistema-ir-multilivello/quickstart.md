# Quickstart - Sistema IR Multi-Livello Verificabile

## Prerequisites
- Visual Studio 2026 con toolset C++23 MSVC.
- CMake 4.2+, Ninja.
- Dipendenze progetto gia gestite via CPM (nessuna aggiunta manuale).

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

### 3.2 Debug constexpr runtime

```powershell
cmake --build --preset windows-msvc-debug-developer-mode --target relaxed_constexpr_tests
ctest --preset test-windows-msvc-debug-developer-mode -R "relaxed_constexpr"
```

### 3.3 Runtime tests completi

```powershell
cmake --build --preset windows-msvc-debug-developer-mode --target tests
ctest --preset test-windows-msvc-debug-developer-mode -R "unittests" --output-on-failure
```

## 4) Quality Gates (local mirror of CI)

```powershell
# Build zero-warning
cmake --build --preset windows-msvc-debug-developer-mode

# Static analysis (se abilitata da preset)
# clang-tidy / cppcheck via CMake options

# Sanitizers (preset/toolchain support)
ctest --preset test-windows-msvc-debug-developer-mode --output-on-failure

# Complexity
lizard src/**/*.cpp include/**/*.hpp --CCN 15 --length 100 --arguments 6

# Coverage
gcovr -r . --config=gcovr.cfg
```

## 5) Implementation Notes
- Error handling uniforme: usare CompileError e std::expected<T, std::vector<CompileError>>.
- Nessuna nuova dipendenza oltre le quattro approvate.
- Tutte le trasformazioni passano da working copy transazionale con commit atomico.
- Per may-alias, strict no-reorder salvo prova formale valida.

## 6) CI Pipeline Reference
GitHub Actions stages: `lint -> static-analysis -> build -> test -> sanitizers -> complexity -> coverage`.
Gate coverage target: `gcovr >= 95%`.

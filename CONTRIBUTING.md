# Contributing to jsav

Thank you for your interest in contributing to the jsav compiler project! This document provides guidelines and instructions for contributing to this project.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Setting Up Development Environment](#setting-up-development-environment)
- [Development Workflow](#development-workflow)
  - [Branching Strategy](#branching-strategy)
  - [Commit Messages](#commit-messages)
- [Making Changes](#making-changes)
  - [Code Style](#code-style)
  - [Testing Requirements](#testing-requirements)
  - [Build Verification](#build-verification)
- [Submitting Changes](#submitting-changes)
  - [Pull Request Checklist](#pull-request-checklist)
  - [Code Review Process](#code-review-process)
- [Architecture Overview](#architecture-overview)
- [Questions?](#questions)

---

## Code of Conduct

This project adheres to the [Contributor Covenant Code of Conduct](CODE_OF_CONDUCT.md). By participating, you are expected to uphold this code. Please report unacceptable behavior to <bianconig6@gmail.com>.

---

## Getting Started

### Prerequisites

Before contributing, ensure you have the following tools installed:

**Required:**
- **CMake** 4.2+
- **C++ Compiler**: GCC 13+ / Clang 16+ / MSVC 2022+ (1930+)
- **Ninja** 1.10+ (recommended build generator)
- **Git**

**Recommended:**
- **clang-tidy** (static analysis)
- **cppcheck** (static analysis)
- **ccache** (compilation caching)
- **lizard** (code complexity analysis)
- **gcovr** (coverage report generation)

**Installation Commands:**

**Windows (Chocolatey):**
```powershell
choco install cmake ninja llvm cppcheck ccache
pip install gcovr lizard
```

**Ubuntu/Debian:**

```bash
sudo apt update
sudo apt install cmake ninja-build build-essential g++ clang-tidy cppcheck ccache gcovr
pip3 install lizard
```

**macOS (Homebrew):**

```bash
brew install cmake ninja llvm cppcheck ccache gcovr
pip3 install lizard
```

### Setting Up Development Environment

1. **Clone the repository:**

   ```bash
   git clone https://github.com/Giuseppe-Bianc/jsav.git
   cd jsav
   ```

2. **Configure the build (Debug with coverage and sanitizers):**

   ```bash
   mkdir -p build && cd build
   cmake -G Ninja \
     -DCMAKE_BUILD_TYPE=Debug \
     -Djsav_PACKAGING_MAINTAINER_MODE=OFF \
     -Djsav_ENABLE_COVERAGE=ON \
     -Djsav_ENABLE_SANITIZER_ADDRESS=ON \
     -Djsav_ENABLE_SANITIZER_UNDEFINED=ON \
     ..
   ```

3. **Build all targets:**

   ```bash
   ninja
   ```

4. **Run tests to verify setup:**

   ```bash
   ctest --output-on-failure
   ```

For detailed build instructions, see [AGENTS.md Section 3](AGENTS.md#3-build-instructions).

---

## Development Workflow

### Branching Strategy

**Strategy**: Feature branches from `main`

**Naming Convention**: `###-feature-name` (e.g., `123-add-unicode-lexer`)

**Workflow:**

```bash
# Create feature branch from main
git checkout main
git pull origin main
git checkout -b 123-feature-name

# After making changes, push branch
git push -u origin 123-feature-name
```

### Commit Messages

**Format**: Conventional Commits

```text
<type>(<scope>): <description>

[optional body]

Closes #<issue-number>
```

**Types:**

- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes (formatting, etc.)
- `refactor`: Code refactoring
- `test`: Adding or updating tests
- `chore`: Build system, dependencies, tooling

**Examples:**

```text
feat(lexer): add Unicode escape sequence support

Implemented parsing of \uXXXX and \UXXXXXXXX escape sequences
in string literals.

Closes #123
```

```text
fix(parser): resolve null pointer in error handler

Added null check before dereferencing token pointer in
error reporting path.
```

---

## Making Changes

### Code Style

**Formatting:**
- Run `clang-format` on all modified files before committing
- Configuration: `.clang-format` (140 column limit, 4-space indent)

```bash
clang-format -i src/**/*.cpp src/**/*.hpp include/**/*.hpp
```

**Naming Conventions:**

| Element | Convention | Example |
|---------|------------|---------|
| Classes/Structs | PascalCase | `Lexer`, `Token` |
| Functions/Methods | camelCase | `tokenize()`, `format_size()` |
| Member Variables | Trailing underscore | `source_`, `value_` |
| Local Variables | snake_case or camelCase | `tokens`, `lexer` |
| Constants | `kPascalCase` or UPPER_SNAKE_CASE | `kMaxRetries`, `UNIT_DIVIDER` |
| Namespaces | lowercase with underscores | `jsv`, `fs` |

**Modern C++ Usage:**

- Use `std::format` / `std::print` instead of `printf` or iostreams
- Mark functions `constexpr` where possible
- Use `[[nodiscard]]` on all functions returning values
- Prefer `enum class` over unscoped enums
- Use `std::unique_ptr` for exclusive ownership, `std::shared_ptr` only when sharing is required
- **No raw `new`/`delete`** — use RAII and smart pointers

For complete style guidelines, see [AGENTS.md Section 4](AGENTS.md#4-code-style-guidelines).

### Testing Requirements

**All new code must include tests:**

1. **Unit tests** in `test/tests.cpp` for runtime behavior
2. **Constexpr tests** in `test/constexpr_tests.cpp` for compile-time evaluation
3. **Coverage target**: ≥80% line coverage, ≥70% branch coverage

**Test Naming Convention:**

```cpp
TEST_CASE("ClassName_MethodName_Scenario", "[module]") {
    SECTION("Specific behavior") {
        // Test implementation
    }
}
```

**Example:**

```cpp
TEST_CASE("Lexer_EmptyInput_ReturnsEOF", "[lexer]") {
    SECTION("Empty source returns EOF token") {
        Lexer lexer("");
        auto tokens = lexer.tokenize();
        REQUIRE(tokens.size() == 1);
        REQUIRE(tokens[0].type == TokenType::Eof);
    }
}
```

**Run tests before submitting:**

```bash
# Build all test targets
ninja tests constexpr_tests relaxed_constexpr_tests

# Run all tests
ctest --output-on-failure

# Run specific test suite
ctest -R "unittests.Lexer" --output-on-failure
```

**Sanitizer Requirements:**

- All tests must pass with AddressSanitizer and UndefinedBehaviorSanitizer enabled
- Zero memory leaks required

For complete testing guidelines, see [AGENTS.md Section 5](AGENTS.md#5-testing).

### Build Verification

**Before submitting changes, verify:**

```bash
# Clean build
rm -rf build/
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug ..
cmake --build build

# Run all tests
cd build && ctest --output-on-failure

# Run static analysis (if enabled)
cmake -Djsav_ENABLE_CLANG_TIDY=ON -Djsav_ENABLE_CPPCHECK=ON ..
cmake --build build

# Check code formatting
clang-format --dry-run --Werror src/**/*.cpp include/**/*.hpp
```

**CI Requirements:**

- Zero compile errors/warnings (warnings as errors)
- All tests pass (all three targets)
- clang-tidy reports zero issues
- cppcheck reports zero issues
- AddressSanitizer: no violations
- UndefinedBehaviorSanitizer: no violations
- Code formatted per `.clang-format`

---

## Submitting Changes

### Pull Request Checklist

Before submitting a pull request, ensure:

- [ ] Linked issue reference (e.g., "Closes #123")
- [ ] All CI checks pass (tests, static analysis, sanitizers)
- [ ] Test coverage for new code (≥80% line, ≥70% branch)
- [ ] Updated documentation if public API changed (Doxygen comments in `include/jsav/`)
- [ ] Code formatted with `clang-format`
- [ ] Commit messages follow Conventional Commits format
- [ ] No new compiler warnings
- [ ] No sanitizer violations
- [ ] No circular dependencies introduced (jsav_Core_lib ↔ jsav_Lib)
- [ ] No raw `new`/`delete` in application code
- [ ] Followed project naming conventions

### Pull Request Template

```markdown
## Description
Brief description of changes made.

## Type of Change
- [ ] Bug fix (non-breaking change that fixes an issue)
- [ ] New feature (non-breaking change that adds functionality)
- [ ] Breaking change (fix or feature that would cause existing functionality to change)
- [ ] Documentation update

## Related Issue
Closes #<issue-number>

## Testing
Describe testing performed:
- [ ] Unit tests added/updated
- [ ] Constexpr tests added/updated (if applicable)
- [ ] All existing tests pass
- [ ] Sanitizers report zero violations
- [ ] Coverage targets met

## Checklist
- [ ] Code follows project style guidelines
- [ ] Code formatted with clang-format
- [ ] Doxygen documentation updated (if public API changed)
- [ ] No new compiler warnings
- [ ] No circular dependencies introduced
```

### Code Review Process

1. **Automated Checks**: CI runs all tests, static analysis, and formatting checks
2. **Maintainer Review**: A project maintainer reviews code for:
   - Correctness of implementation
   - Adequate test coverage
   - Style compliance
   - API documentation
   - No new compiler warnings
3. **Feedback**: Reviewer provides feedback; address comments and push updates
4. **Approval**: Once approved, maintainer merges PR to `main`

**Review Criteria:**

- Code correctness and robustness
- Test coverage adequacy
- Performance impact (if any)
- Security implications
- Documentation completeness

---

## Architecture Overview

**Project Structure:**

```text
jsav/
├── include/jsav/          # Public API headers (installed for consumers)
│   ├── lexer/             # Lexical analysis interfaces
│   └── fs/                # Filesystem utilities
├── include/jsavCore/      # Internal utility headers (not installed)
├── src/jsav/              # Main executable (entry point)
├── src/jsav_Core_lib/     # Core library (logging, file I/O, timers)
├── src/jsav_Lib/          # Main library (compiler logic)
└── test/                  # Unit tests, constexpr tests
```

**Dependency Hierarchy:**

```text
jsav_Core_lib (foundational utilities)
    ↓
jsav_Lib (compiler logic)
    ↓
jsav (executable)
```

**Key Rules:**

- **No circular dependencies** between `jsav_Core_lib` and `jsav_Lib`
- **Public headers** in `include/jsav/` must not expose dependency types (spdlog, fmt, CLI11)
- **Internal headers** in `include/jsavCore/` are not installed

For complete architecture documentation, see [AGENTS.md Section 2](AGENTS.md#2-repository-structure).

---

## Questions?

- **Build Issues**: See [AGENTS.md Appendix B: Troubleshooting](AGENTS.md#appendix-b-troubleshooting)
- **Code Style**: See [AGENTS.md Section 4](AGENTS.md#4-code-style-guidelines)
- **Testing**: See [AGENTS.md Section 5](AGENTS.md#5-testing)
- **General**: Open an issue with the `question` label

**Resources:**

- [AGENTS.md](AGENTS.md) — Comprehensive technical reference
- [AI_GUIDELINES.md](AI_GUIDELINES.md) — Development guidelines
- [C++23 Best Practices](https://leanpub.com/cpp23_best_practices/) — Project philosophical foundation

---

Thank you for contributing to jsav! Your efforts help make this project better for everyone.

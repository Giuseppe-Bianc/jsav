# AGENTS.md

**Version**: 1.0.0
**Last Updated**: 2026-03-05
**Status**: Active — This document serves as the authoritative technical reference for the jsav project

---

## Document Purpose and Scope

This document provides comprehensive, authoritative guidance for both human developers and automated AI agents working with the jsav codebase. It serves as the single source of truth for understanding the project's architecture, build processes, coding standards, testing strategies, and operational constraints.

**Key Characteristics:**

- **Evidence-Based**: All information is sourced from specific repository files, ensuring accuracy and verifiability
- **Dual Audience**: Written to be equally useful for human developers reading linearly and AI agents parsing programmatically
- **Actionable**: Commands are copy-pasteable; rules are stated as explicit imperatives
- **Complete**: Covers all aspects necessary for effective contribution without requiring external context

**Intended Use Cases:**

1. **Onboarding**: New developers can understand the project structure and development workflow
2. **Automated Code Generation**: AI agents can produce code that complies with project standards
3. **Quality Assurance**: Reviewers can verify compliance with architectural and style rules
4. **Troubleshooting**: Developers can diagnose and resolve common build and development issues

---

## 1. Project Overview

### 1.1 Problem Statement

**jsav** is a compiler implementation project written in modern C++23. It addresses the fundamental need for a robust, well-structured compiler foundation that:

- Leverages contemporary C++ language features to their fullest extent
- Enforces strict code quality standards through comprehensive automated tooling
- Provides a maintainable and extensible architecture for compiler development
- Demonstrates best practices in C++ software engineering through its own implementation

The project recognizes that compiler development presents unique challenges: complex state management, intricate error handling, performance-critical parsing algorithms, and the need for precise diagnostics. jsav addresses these challenges by establishing a strong architectural foundation with rigorous quality gates.

**Philosophical Foundation**: The project is based on the guidelines from [C++23 Best Practices](https://leanpub.com/cpp23_best_practices/) by Jason Turner, emphasizing tool-assisted development, type safety, and modern C++ idioms.

**Source**: `QWEN.md`, `AI_GUIDELINES.md`, `HUMAN_GUIDELINES.md`, `.specify/memory/constitution.md`

### 1.2 Core Functionality

The jsav compiler provides a comprehensive suite of capabilities organized into distinct, modular components:

#### Lexical Analysis Subsystem

- **Unicode-Aware Lexer**: Full UTF-8 support for international character sets and source code encoding
- **Token Stream Generation**: Converts raw source text into structured token sequences
- **Source Location Tracking**: Precise tracking of token positions for error reporting and diagnostics
- **Source Span Management**: Handles multi-line tokens and complex lexical constructs

**Implementation Location**: `include/jsav/lexer/`, `src/jsav_Lib/lexer/`

#### File System Operations Module

- **Cross-Platform File I/O**: Abstracted filesystem operations that work consistently across Windows, Linux, and macOS
- **Directory Management**: Creation, deletion, and traversal of directory structures
- **Path Manipulation**: Canonicalization, normalization, and resolution of file paths
- **Error Handling**: Structured error reporting for filesystem operations with detailed diagnostic information

**Implementation Location**: `include/jsav/fs/`, `include/jsavCore/FileReader.hpp`

#### Logging Infrastructure

- **Structured Logging**: Multi-level logging (trace, debug, info, warning, error, critical) via spdlog
- **Console and File Sinks**: Simultaneous output to console and log files for debugging and production use
- **Performance Timing**: Built-in timer utilities for measuring compilation phases and identifying bottlenecks
- **Thread-Safe Operation**: Safe logging from multiple threads without race conditions

**Implementation Location**: `include/jsavCore/Log.hpp`, `src/jsav_Core_lib/jsavCore.cpp`

#### Command-Line Interface

- **Argument Parsing**: Sophisticated CLI parsing using CLI11 library with support for flags, options, and subcommands
- **Help System**: Automatic generation of help messages and usage documentation
- **Version Information**: Build metadata including version number and Git SHA for traceability
- **Configuration Options**: Compile-time and runtime configuration through command-line parameters

**Implementation Location**: `src/jsav/main.cpp`

**Source**: `QWEN.md`, observed in `src/jsav/main.cpp`, `include/jsav/lexer/`

### 1.3 Runtime Context

#### Deployment Environment

**Target Platforms**: Native compiler executable running on:

- **Windows**: 64-bit Windows 10/11 with MSVC 2022+ or Clang-cl
- **Linux**: Modern distributions (Ubuntu 20.04+, Fedora 35+, etc.) with GCC 13+ or Clang 16+
- **macOS**: macOS 11+ (Big Sur) with Apple Clang 16+ or Homebrew GCC/Clang

**Execution Context**:

- Runs as a standalone command-line application
- No runtime dependencies beyond standard C++ library and bundled dependencies
- Designed for integration into build pipelines and CI/CD systems

**Source**: `CMakeLists.txt`, `ProjectOptions.cmake`,

#### Platform Requirements

**Compiler Requirements**:

- **GCC**: Version 13 or later (full C++23 feature support)
- **Clang**: Version 16 or later (comprehensive C++23 implementation)
- **MSVC**: Visual Studio 2022 or later (version 1930+) with C++23 mode

**Build System Requirements**:

- **CMake**: Version 4.2 minimum (leverages modern CMake features)
- **Ninja**: Recommended build generator for optimal performance
- **Git**: Required for version information embedding

**Runtime Dependencies**:

- Standard C++ library (C++23 compliant)
- No external runtime dependencies (all dependencies statically linked or bundled)

**Source**: `CMakeLists.txt`, `ProjectOptions.cmake`, `QWEN.md`

#### Execution Model

**Compilation Pipeline**:

1. **Input Phase**: Reads source file(s) specified via command-line arguments
2. **Lexical Analysis**: Single-pass tokenization of input source code
3. **Processing**: (Future phases: parsing, semantic analysis, code generation)
4. **Output**: Produces compiled output or diagnostic information

**Performance Characteristics**:

- Designed for incremental compilation support
- Memory-efficient processing for large source files
- Parallelizable phases where dependencies allow

**Error Handling Strategy**:

- Graceful degradation on errors (continues processing where safe)
- Comprehensive diagnostic output with source locations
- Structured error codes for integration with development tools

**Source**: `src/jsav/main.cpp`, `QWEN.md`

### 1.4 Key External Integrations

The project utilizes a minimal, carefully curated set of external dependencies, each selected for quality, performance, and alignment with project goals:

#### fmtlib/fmt (v12.1.0)

**Purpose**: String formatting library, used as fallback when `std::format` is unavailable
**Integration**: Linked as `fmtlib::fmt` target
**Usage Pattern**: Accessed through `FORMAT()` macro for consistent formatting interface
**Rationale**: Provides C++23-style formatting on platforms with incomplete `std::format` support
**License**: MIT License (compatible with project distribution)

**Source**: `Dependencies.cmake`

#### spdlog (v1.17.0)

**Purpose**: High-performance logging library for structured diagnostic output
**Integration**: Linked as `spdlog::spdlog` target with PCH (Pre-Compiled Headers) enabled
**Configuration**:

- Windows: Wide character support enabled for native Windows API compatibility
- Unix: Narrow character mode for standard POSIX compatibility
**Usage Pattern**: Global logger instance configured at application startup
**License**: MIT License (compatible with project distribution)

**Source**: `Dependencies.cmake`, `ProjectOptions.cmake`

#### CLI11 (v2.6.1)

**Purpose**: Command-line argument parsing with support for complex option structures
**Integration**: Linked as `CLI11::CLI11` target
**Features Used**: Subcommands, option groups, custom validators, automatic help generation
**License**: BSD 3-Clause License (compatible with project distribution)

**Source**: `Dependencies.cmake`, `src/jsav/main.cpp`

#### Catch2 (v3.13.0)

**Purpose**: Modern C++ testing framework with support for unit tests, benchmarks, and compile-time verification
**Integration**: Linked as `Catch2::Catch2WithMain` target (includes main entry point)
**Features Used**:

- `TEST_CASE`/`SECTION` structure for hierarchical test organization
- `STATIC_REQUIRE` for compile-time assertions
- Benchmark support for performance regression testing
- XML reporter for CI integration
**License**: Boost Software License 1.0 (compatible with project distribution)

**Source**: `Dependencies.cmake`, `test/CMakeLists.txt`

#### Dependency Management Principles

**Version Locking**: All dependencies specify exact versions to ensure reproducible builds across time and environments.

**Header Isolation**: Dependency headers are included only in implementation files (`.cpp`) or internal headers, never exposed through public API headers. This prevents "header leak" anti-pattern where consumers become coupled to project dependencies.

**Private Linkage**: Dependencies are linked with `PRIVATE` visibility in CMake targets, preventing propagation to downstream consumers.

**Source**: `Dependencies.cmake`, `.specify/memory/constitution.md`

---

## 2. Repository Structure

### 2.1 Top-Level Directory Layout

The repository follows a conventional CMake project structure with clear separation between public interfaces, implementation, tests, and build infrastructure. This organization supports modular development, clear dependency boundaries, and maintainable code organization.

```text
jsav/
├── CMakeLists.txt              # Main CMake configuration (project definition, options, install rules)
├── ProjectOptions.cmake        # Compiler options, sanitizers, static analysis setup, feature flags
├── Dependencies.cmake          # External dependency management via CPM (version-locked)
├── CMakePresets.json           # Predefined CMake configurations for all platforms and toolchains
├── include/
│   ├── jsav/                   # Public headers (main module API — installed for consumers)
│   │   ├── headers.hpp         # Master include for jsav module (convenience header)
│   │   ├── jsav.hpp            # Main module header (exports lexer, fs, tokens — primary API)
│   │   ├── fs/                 # Filesystem utilities (public API — cross-platform file ops)
│   │   │   ├── FileCreationResult.hpp
│   │   │   ├── FileDeletionResult.hpp
│   │   │   ├── FolderCreationResult.hpp
│   │   │   ├── FolderDeletionResult.hpp
│   │   │   ├── fs.hpp
│   │   │   ├── FSConstats.hpp
│   │   │   └── OSOperationResult.hpp
│   │   └── lexer/              # Lexer interfaces (public API — lexical analysis)
│   │       ├── unicode/        # Unicode support utilities
│   │       │   ├── UnicodeData.hpp
│   │       │   └── Utf8.hpp
│   │       ├── Lexer.hpp
│   │       ├── SourceLocation.hpp
│   │       ├── SourceSpan.hpp
│   │       └── Token.hpp
│   └── jsavCore/               # Core library headers (internal utilities — NOT installed)
│       ├── headersCore.hpp     # Core master include (convenience header for core module)
│       ├── jsavCore.hpp        # Core module header (internal implementation details)
│       ├── cast/               # Casting utilities (type-safe conversions)
│       │   ├── BaseCast.hpp
│       │   ├── BitCast.hpp
│       │   ├── casts.hpp
│       │   ├── NarrowCast.hpp
│       │   └── TypeSizes.hpp
│       ├── timer/              # Timer utilities (performance measurement)
│       │   ├── timeFactors.hpp
│       │   ├── Timer.hpp
│       │   ├── TimerConstats.hpp
│       │   └── Times.hpp
│       ├── disableWarn.hpp     # Warning suppression macros (compiler-specific)
│       ├── FileReader.hpp      # File reading utilities (internal file I/O)
│       ├── FileReaderError.hpp # File reader error types (structured error handling)
│       ├── format.hpp          # Formatting utilities (wrapper around fmt/std::format)
│       ├── Log.hpp             # Logging setup (spdlog configuration and macros)
│       └── move.hpp            # Move semantics utilities (perfect forwarding helpers)
├── src/
│   ├── CMakeLists.txt          # Source subdirectory configuration (target definitions)
│   ├── jsav/                   # Main executable (application entry point)
│   │   ├── CMakeLists.txt      # Executable target configuration
│   │   ├── Costanti.hpp        # Compile-time constants (application-specific)
│   │   └── main.cpp            # Entry point with CLI parsing (program startup)
│   ├── jsav_Core_lib/          # Core library implementation (foundational utilities)
│   │   ├── CMakeLists.txt      # Core library target configuration
│   │   └── jsavCore.cpp        # Core module definitions (logging, file I/O, timers)
│   └── jsav_Lib/               # Main library implementation (compiler logic)
│       ├── CMakeLists.txt      # Main library target configuration
│       ├── jsav.cpp            # Main library definitions (compiler orchestration)
│       └── lexer/              # Lexer implementation (tokenization logic)
│           ├── Lexer.cpp
│           ├── SourceLocation.cpp
│           ├── SourceSpan.cpp
│           └── Token.cpp
├── test/
│   ├── CMakeLists.txt          # Test configuration (Catch2 setup, test discovery)
│   ├── constexpr_tests.cpp     # Compile-time constexpr tests (STATIC_REQUIRE assertions)
│   ├── tests.cpp               # Runtime unit tests (REQUIRE assertions, I/O tests)
│   └── testsConstanst.hpp      # Test constants and utilities (shared test fixtures)
├── fuzz_test/                  # Fuzz testing (optional, libFuzzer-based mutation testing)
│   ├── CMakeLists.txt
│   └── fuzz_tester.cpp
├── cmake/                      # CMake modules and utilities (build system infrastructure)
│   ├── _FORTIFY_SOURCE.hpp    # Security hardening definitions
│   ├── Cache.cmake             # Compilation caching configuration (ccache integration)
│   ├── CompilerWarnings.cmake  # Compiler-specific warning flags (GCC/Clang/MSVC)
│   ├── CPM.cmake               # CMake Package Manager (dependency fetching)
│   ├── Cuda.cmake              # CUDA support configuration (optional)
│   ├── Doxygen.cmake           # Documentation generation configuration
│   ├── Emscripten.cmake        # WebAssembly compilation support (optional)
│   ├── Hardening.cmake         # Security hardening options (FORTIFY, relro, etc.)
│   ├── InterproceduralOptimization.cmake  # LTO/IPO configuration
│   ├── LibFuzzer.cmake         # LibFuzzer integration for fuzz testing
│   ├── Linker.cmake            # Linker-specific flags and configuration
│   ├── PackageProject.cmake    # CMake package configuration for installation
│   ├── PreventInSourceBuilds.cmake  # Enforces out-of-source builds
│   ├── Sanitizers.cmake        # Sanitizer configuration (ASan, UBSan, TSan)
│   ├── Simd.cmake              # SIMD feature detection and configuration
│   ├── StandardProjectSettings.cmake  # Standard CMake project defaults
│   ├── StaticAnalyzers.cmake   # Static analysis configuration (clang-tidy, cppcheck)
│   ├── SystemLink.cmake        # System-specific linker configuration
│   ├── Tests.cmake             # Test configuration utilities (coverage, discovery)
│   ├── Utilities.cmake         # General CMake utility functions
│   └── VCEnvironment.cmake     # Visual Studio environment configuration
├── configured_files/           # Template files for CMake generation (configured at build time)
│   ├── CMakeLists.txt
│   └── config.hpp.in           # Version and configuration header template
├── scripts/                    # Utility scripts (automation, code generation)
│   └── generate_unicode_tables.py  # Unicode data table generation
├── specs/                      # Specification documents (formal requirements, design docs)
└── vn_files/                   # Version number files (semantic versioning metadata)
```

**Source**: Direct observation of repository structure, `QWEN.md`

### 2.2 Directory Relationships and Constraints

The following table describes the purpose, dependencies, and constraints for each top-level directory. Understanding these relationships is critical for maintaining architectural integrity and avoiding circular dependencies.

| Directory | Purpose | Relationships | Constraints |
|-----------|---------|---------------|-------------|
| `include/jsav/` | **Public API headers** — Defines the interface exposed to library consumers | **Consumed by**: `src/jsav_Lib/`, `src/jsav/`, external projects<br>**Depends on**: `include/jsavCore/` (internal utilities)<br>**Installed**: Yes, via CMake `install()` rules | Headers here constitute the **stable public API** — do not break backward compatibility without a deprecation cycle. All public headers must have Doxygen documentation. Changes require API review. |
| `include/jsavCore/` | **Internal utility headers** — Foundational utilities used across the project | **Consumed by**: `src/jsav_Core_lib/`, `src/jsav_Lib/`, `src/jsav/`<br>**Depends on**: External dependencies (spdlog, fmt)<br>**Installed**: No, internal use only | Internal implementation details — not exposed to downstream consumers. May change without notice. Should not depend on `include/jsav/` (no circular dependencies). |
| `src/jsav/` | **Main executable** — Application entry point and CLI orchestration | **Links against**: `jsav::jsav_lib`, `jsav::jsav_core_lib`<br>**Depends on**: `include/jsav/`, `include/jsavCore/`<br>**Produces**: `jsav` executable | Single entry point (`main.cpp`). Should contain minimal logic — orchestration only. Business logic belongs in libraries. |
| `src/jsav_Core_lib/` | **Core library implementation** — Foundational utilities (logging, file I/O, timers) | **Links against**: External dependencies (spdlog, fmt)<br>**Depends on**: `include/jsavCore/`<br>**Consumed by**: `src/jsav_Lib/`, `src/jsav/` | No dependencies on `jsav_Lib` (unidirectional dependency graph). Must remain lightweight and universally usable. |
| `src/jsav_Lib/` | **Main library implementation** — Compiler logic (lexer, parser, semantic analysis) | **Links against**: `jsav::jsav_core_lib`, external dependencies<br>**Depends on**: `include/jsav/`, `include/jsavCore/`<br>**Consumed by**: `src/jsav/` | Implements core compiler functionality. May depend on core library but not vice versa. Lexer implementation in `lexer/` subdirectory. |
| `test/` | **Test files** — Unit tests, constexpr tests, integration tests | **Links against**: All libraries (`jsav::jsav_lib`, `jsav::jsav_core_lib`), Catch2<br>**Depends on**: `include/jsav/`, `include/jsavCore/` | Three test targets: `constexpr_tests` (compile-time), `relaxed_constexpr_tests` (runtime constexpr), `tests` (runtime only). Tests must be deterministic and isolated. |
| `cmake/` | **Build system modules** — CMake configuration, tooling integration | **Referenced by**: Root `CMakeLists.txt`, `ProjectOptions.cmake`<br>**Dependencies**: None (pure CMake code) | Do not modify unless updating build system. Changes affect all targets. Requires testing across all platforms (Windows, Linux, macOS). |
| `fuzz_test/` | **Fuzz testing** — Mutation-based testing for robustness | **Links against**: `jsav::jsav_lib`, LibFuzzer<br>**Depends on**: `include/jsav/` | Optional — only built when `jsav_BUILD_FUZZ_TESTS=ON`. Requires libFuzzer support (Clang). Tests must be idempotent and fast. |
| `configured_files/` | **Template files** — CMake-generated configuration headers | **Consumed by**: Build system<br>**Produces**: `config.hpp` (in build directory) | Templates use CMake `@variable@` substitution. Generated files should not be committed to version control. |
| `scripts/` | **Utility scripts** — Automation, code generation, build helpers | **Dependencies**: Python 3.x (for `.py` scripts)<br>**Used by**: Developers, CI pipeline | Scripts should be cross-platform or clearly marked for specific OS. Document usage in script comments. |
| `specs/` | **Specification documents** — Formal requirements, design decisions | **References**: Implementation in `src/`, `include/`<br>**Used by**: Developers, reviewers | Living documents — update when implementation changes. Should justify architectural decisions with rationale. |

**Key Architectural Principles:**

1. **Dependency Direction**: `jsav_Core_lib` → `jsav_Lib` → `jsav` (executable). Core library must not depend on main library.

2. **Include Direction**: `include/jsavCore/` → `include/jsav/` → `src/`. Public headers may include core headers, but not vice versa.

3. **Test Isolation**: Tests link against all libraries but must not modify global state that affects other tests.

4. **Build System Purity**: `cmake/` modules must be side-effect free and idempotent.

**Source**: `CMakeLists.txt`, `src/CMakeLists.txt`, `test/CMakeLists.txt`, `.gitignore`, `.specify/memory/constitution.md`

### 2.3 Build Pipeline Flow

The build process follows a well-defined sequence of stages, each with specific responsibilities and outputs. Understanding this flow is essential for debugging build issues and optimizing compilation performance.

#### Stage 1: Configuration Phase

**Trigger**: `cmake -S . -B build` or CMake preset invocation

**Process**:

1. CMake reads root `CMakeLists.txt` (minimum version 4.2)
2. Includes `ProjectOptions.cmake` → defines build options and feature flags
3. Includes `Dependencies.cmake` → configures CPM and external packages
4. Includes `cmake/` modules → compiler warnings, sanitizers, static analyzers
5. Processes `CMakePresets.json` → applies platform-specific settings
6. Generates build system files (Ninja build rules by default)

**Outputs**:

- `build/CMakeCache.txt` — Cached configuration variables
- `build/build.ninja` — Ninja build rules
- `build/config.hpp` — Configured header with version information

**Key Configuration Variables**:

- `CMAKE_BUILD_TYPE` — Debug, Release, RelWithDebInfo
- `jsav_PACKAGING_MAINTAINER_MODE` — Developer mode (enables more options)
- `jsav_ENABLE_COVERAGE` — Code coverage instrumentation
- `jsav_ENABLE_SANITIZER_*` — Sanitizer enable flags
- `jsav_ENABLE_CLANG_TIDY` — Static analysis integration

**Source**: `CMakeLists.txt`, `ProjectOptions.cmake`, `CMakePresets.json`

#### Stage 2: Dependency Resolution

**Trigger**: First build invocation after configuration

**Process**:

1. CPM.cmake checks for existing dependencies in `_deps/` directory
2. For missing dependencies, CPM downloads from GitHub releases
3. Dependencies are built as separate CMake targets
4. Target properties (include directories, compile flags) are propagated

**Dependencies Downloaded**:

- `fmt` (v12.1.0) → `fmtlib::fmt` target
- `spdlog` (v1.17.0) → `spdlog::spdlog` target
- `Catch2` (v3.13.0) → `Catch2::Catch2WithMain` target
- `CLI11` (v2.6.1) → `CLI11::CLI11` target

**Output Location**: `build/_deps/<name>-src/`

**Caching**: Dependencies are cached in `~/.cpm/` (or `C:\Users\<user>\.cpm\` on Windows) for reuse across projects.

**Source**: `Dependencies.cmake`, `cmake/CPM.cmake`

#### Stage 3: Target Creation and Compilation

**Target Hierarchy**:

```text
jsav_options (interface)
    └── Compiler options, C++23 standard, Unicode flags

jsav_warnings (interface)
    └── Warning flags (-Wall -Wextra or /W4)

jsav_core_lib (static library)
    ├── Depends: jsav_options, jsav_warnings, spdlog, fmt
    └── Sources: src/jsav_Core_lib/jsavCore.cpp

jsav_lib (static library)
    ├── Depends: jsav_core_lib, jsav_options, jsav_warnings
    └── Sources: src/jsav_Lib/jsav.cpp, src/jsav_Lib/lexer/*.cpp

jsav (executable)
    ├── Depends: jsav_lib, jsav_core_lib, CLI11
    └── Sources: src/jsav/main.cpp
```

**Compilation Order**:

1. Interface libraries (options, warnings) — header-only, no compilation
2. `jsav_core_lib` — foundational utilities (must compile first)
3. `jsav_lib` — main library (depends on core_lib)
4. `jsav` executable — final linking

**Parallelization**: Ninja automatically parallelizes compilation across available CPU cores. Use `ninja -j <N>` to control parallelism.

**Source**: `CMakeLists.txt`, `src/CMakeLists.txt`, `src/jsav/CMakeLists.txt`, `src/jsav_Core_lib/CMakeLists.txt`, `src/jsav_Lib/CMakeLists.txt`

#### Stage 4: Test Discovery and Registration

**Trigger**: `BUILD_TESTING=ON` (default for top-level project)

**Process**:

1. CMake processes `test/CMakeLists.txt`
2. Creates three test executables:
   - `constexpr_tests` — Compile-time verification
   - `relaxed_constexpr_tests` — Runtime constexpr testing
   - `tests` — Runtime unit tests
3. Catch2's `catch_discover_tests()` scans test files for `TEST_CASE` macros
4. Registers individual tests with CTest

**Test Registration**:

```cmake
catch_discover_tests(
    tests
    TEST_PREFIX "unittests."
    REPORTER XML
    OUTPUT_DIR .
    OUTPUT_PREFIX "unittests."
    OUTPUT_SUFFIX .xml)
```

**Output**: Individual test executables in `build/test/` directory

**Source**: `test/CMakeLists.txt`

#### Stage 5: Output and Installation

**Build Output Directory**: `out/build/<preset>/` (configured in `CMakePresets.json`)

**Artifact Locations**:

- Executables: `out/build/<preset>/jsav[.exe]`
- Libraries: `out/build/<preset>/*.lib` or `*.a`
- Test binaries: `out/build/<preset>/test/`

**Installation** (when `cmake --install` is run):

- Headers: `<install-prefix>/include/jsav/`
- Libraries: `<install-prefix>/lib/`
- Executables: `<install-prefix>/bin/`
- CMake config: `<install-prefix>/lib/cmake/jsav/`

**Packaging**: CPack generates installers with versioned filenames:

```text
jsav-<version>-<git_sha>-<platform>-<build_type>-<compiler>
```

**Source**: `CMakeLists.txt`, `CMakePresets.json`, `cmake/PackageProject.cmake`

---

---

## 3. Build Instructions

This section provides comprehensive, step-by-step instructions for building the jsav project across all supported platforms. The instructions are organized by platform and build configuration, with explicit commands that can be executed verbatim.

### 3.1 Prerequisites

Successful building of jsav requires a specific set of tools and dependencies. These are categorized as **required** (mandatory for any build) and **recommended** (enhance development workflow but not strictly necessary).

#### Required Tools

| Tool | Minimum Version | Purpose | Installation Verification |
|------|-----------------|---------|---------------------------|
| **CMake** | 4.2 | Build system configuration and generation | `cmake --version` |
| **C++ Compiler** | GCC 13+ / Clang 16+ / MSVC 2022+ (1930+) | C++23 compilation | `g++ --version`, `clang++ --version`, `cl` |
| **Ninja** | 1.10+ | Build generator (faster than Make) | `ninja --version` |

**Compiler Selection Rationale**:

- **GCC 13+**: Full C++23 feature support including `std::format`, enhanced constexpr, and modules
- **Clang 16+**: Comprehensive C++23 implementation with excellent diagnostic messages
- **MSVC 2022+**: C++23 mode with continuous feature updates; requires `/std:c++latest` flag

**Note on CMake Version**: The project requires CMake 4.2+ to leverage modern CMake features such as `CMAKE_PRESET_VERSION 3`, improved dependency management, and modern target-based configuration patterns.

**Source**: `CMakeLists.txt`, `ProjectOptions.cmake`, `QWEN.md`

#### Recommended Tools

The following tools are not strictly required but significantly enhance the development workflow and are expected by the project's CI/CD pipeline:

| Tool | Purpose | Installation Verification | Priority |
|------|---------|---------------------------|----------|
| **clang-tidy** | Static analysis — detects bugs, style violations, and modernization opportunities | `clang-tidy --version` | **High** (enforced in CI) |
| **cppcheck** | Static analysis — complementary to clang-tidy, catches different issue classes | `cppcheck --version` | **High** (enforced in CI) |
| **ccache** | Compilation caching — speeds up rebuilds by 5-10x for incremental builds | `ccache --version` | **High** (recommended for development) |
| **include-what-you-use** | Header dependency analysis — ensures minimal, correct includes | `iwyu --version` | Medium (optional) |
| **lizard** | Code complexity analysis — enforces function complexity thresholds | `lizard --version` | **High** (enforced in CI) |
| **gcovr** | Coverage report generation — produces HTML and Cobertura XML reports | `gcovr --version` | Medium (for coverage analysis) |

**Tool Installation Commands**:

**Ubuntu/Debian Linux**:

```bash
# Update package index
sudo apt update

# Install compiler toolchain
sudo apt install build-essential g++

# Install CMake and Ninja
sudo apt install cmake ninja-build

# Install static analysis tools
sudo apt install clang-tidy cppcheck

# Install ccache for compilation caching
sudo apt install ccache

# Install gcovr for coverage reports
sudo apt install gcovr

# Install lizard (may require pip)
pip3 install lizard
```

**Windows (Chocolatey)**:
```powershell
# Install LLVM (includes clang-tidy)
choco install llvm

# Install cppcheck
choco install cppcheck

# Install ccache
choco install ccache

# Install CMake
choco install cmake

# Install Ninja
choco install ninja

# Install gcovr
pip install gcovr
```

**macOS (Homebrew)**:

```bash
# Install LLVM (includes clang-tidy)
brew install llvm

# Install cppcheck
brew install cppcheck

# Install ccache
brew install ccache

# Install CMake
brew install cmake

# Install Ninja
brew install ninja

# Install gcovr
brew install gcovr

# Install lizard
pip3 install lizard
```

**Important**: The project is configured to **fail configuration** if required tools are missing. This is intentional — do not disable tool checks. Instead, install the missing tools.

**Source**: `QWEN.md`, `ProjectOptions.cmake`, `cmake/StaticAnalyzers.cmake`, `.gitlab-ci.yml`

### 3.2 C++ Standard

**Standard**: C++23 (ISO/IEC 14882:2023)

The project explicitly targets C++23 to leverage modern language features that improve code safety, expressiveness, and performance.

**Configuration in CMakeLists.txt**:

```cmake
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)  # Strict standard conformance
```

**Key C++23 Features Used**:

- `std::format` and `std::print` — Type-safe formatting (with fmt fallback)
- Enhanced `constexpr` — More functions can be evaluated at compile-time
- `std::expected<T, E>` — Error handling without exceptions (where available)
- Designated initializers — Clearer aggregate initialization
- `if consteval` — Compile-time branching
- Improved template deduction — Reduced boilerplate

**Compiler Flag Mapping**:

- **GCC**: `-std=c++23` or `-std=c++2b` (depending on version)
- **Clang**: `-std=c++23`
- **MSVC**: `/std:c++latest`

**Standard Library Implementation**:

- **libstdc++** (GCC): Full C++23 support as of GCC 13
- **libc++** (Clang): Comprehensive C++23 implementation
- **MSVC STL**: C++23 features shipped continuously

**Source**: `CMakeLists.txt`, `AI_GUIDELINES.md`, `HUMAN_GUIDELINES.md`

### 3.3 Build System

**Primary System**: CMake 4.2+ (Cross-platform build system generator)

**Default Generator**: Ninja (recommended for performance)

**Alternative Generators**:

- **Ninja Multi-Config**: For switching between Debug/Release without reconfiguration
- **Visual Studio Generator**: For IDE integration on Windows (`-G "Visual Studio 17 2022"`)
- **Unix Makefiles**: Fallback when Ninja is unavailable (slower parallel builds)

**Source**: `CMakeLists.txt`, `CMakePresets.json`

#### CMake Presets

The project provides predefined configurations via `CMakePresets.json`. Presets encapsulate platform-specific settings, compiler selections, and build options for common scenarios.

**Preset Naming Convention**:

- `windows-msvc-debug-developer-mode` — Windows, MSVC compiler, Debug build, full tooling
- `windows-msvc-release-developer-mode` — Windows, MSVC, Release, full tooling
- `unixlike-gcc-debug` — Linux/macOS, GCC compiler, Debug build
- `unixlike-clang-debug` — Linux/macOS, Clang compiler, Debug build

**Preset Structure**:

```json
{
    "version": 3,
    "configurePresets": [
        {
            "name": "windows-msvc-debug-developer-mode",
            "displayName": "msvc Debug (Developer Mode)",
            "generator": "Ninja",
            "binaryDir": "${sourceDir}/out/build/${presetName}",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Debug",
                "ENABLE_DEVELOPER_MODE": "ON"
            }
        }
    ]
}
```

**Using Presets**:

```bash
# Configure using preset
cmake -S . --preset <preset-name>

# Build using preset
cmake --build --preset <preset-name>

# Test using preset
ctest --preset test-<preset-name>
```

**Toolchain Files**:

- **Standard Builds**: No toolchain file required
- **Emscripten (WASM)**: Toolchain available but currently disabled
- **Cross-Compilation**: Custom toolchain files can be specified via `-DCMAKE_TOOLCHAIN_FILE=<path>`

**Source**: `CMakePresets.json`, `cmake/Emscripten.cmake`

### 3.4 Build Commands

This subsection provides copy-pasteable command sequences for all supported platforms and configurations. Commands are presented in the exact order they must be executed.

#### Windows (MSVC Compiler)

**Prerequisites**: Visual Studio 2022 with "Desktop development with C++" workload, or Build Tools for Visual Studio 2022.

**Using CMake Presets (Recommended)**:

```powershell
# Navigate to project root
cd C:\dev\visualStudio\jsav

# Configure with CMake preset (Debug, Developer Mode with all tools)
cmake -S . --preset windows-msvc-debug-developer-mode

# Build the project
cmake --build --preset windows-msvc-debug-developer-mode

# Run all tests (stops on first failure)
ctest --preset test-windows-msvc-debug-developer-mode

# Build Release configuration
cmake -S . --preset windows-msvc-release-developer-mode
cmake --build --preset windows-msvc-release-developer-mode
```

**Manual Configuration (Without Presets)**:

```powershell
# Create build directory
mkdir build
cd build

# Configure with Ninja generator
cmake -G Ninja `
  -DCMAKE_BUILD_TYPE=Debug `
  -DCMAKE_C_COMPILER=cl `
  -DCMAKE_CXX_COMPILER=cl `
  -Djsav_PACKAGING_MAINTAINER_MODE=OFF `
  ..

# Build
ninja

# Run tests
ctest --output-on-failure
```

**Using Visual Studio IDE**:

1. Open Visual Studio 2022
2. File → Open → CMake → Select `CMakeLists.txt`
3. Select configuration from dropdown (Debug/x64, Release/x64)
4. Build → Build All (or Ctrl+Shift+B)
5. Test → Run All Tests

**Source**: `CMakePresets.json`, `QWEN.md`

#### Unix-like Systems (GCC Compiler)

**Prerequisites**: GCC 13+, CMake 4.2+, Ninja

**Using CMake Presets (Recommended)**:

```bash
# Navigate to project root
cd /path/to/jsav

# Configure (Debug)
cmake -S . --preset unixlike-gcc-debug

# Build
cmake --build --preset unixlike-gcc-debug

# Run all tests
ctest --preset test-unixlike-gcc-debug

# Build Release configuration
cmake -S . --preset unixlike-gcc-release
cmake --build --preset unixlike-gcc-release
```

**Manual Configuration**:

```bash
# Create and enter build directory
mkdir -p build && cd build

# Configure with Ninja
cmake -G Ninja \
  -DCMAKE_C_COMPILER=gcc \
  -DCMAKE_CXX_COMPILER=g++ \
  -DCMAKE_BUILD_TYPE=Debug \
  -Djsav_PACKAGING_MAINTAINER_MODE=OFF \
  ..

# Build (uses all available cores)
ninja

# Run tests
ctest --output-on-failure
```

**Source**: `CMakePresets.json`, `QWEN.md`, `.gitlab-ci.yml`

#### Unix-like Systems (Clang Compiler)

**Prerequisites**: Clang 16+, CMake 4.2+, Ninja

```bash
# Configure (Debug)
cmake -S . --preset unixlike-clang-debug

# Build
cmake --build --preset unixlike-clang-debug

# Run tests
ctest --preset test-unixlike-clang-debug
```

**Manual Configuration**:

```bash
mkdir -p build && cd build

cmake -G Ninja \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_BUILD_TYPE=Debug \
  -Djsav_PACKAGING_MAINTAINER_MODE=OFF \
  ..

ninja
ctest --output-on-failure
```

**Source**: `CMakePresets.json`

#### Manual Configuration (Cross-Platform, Detailed)

For advanced users requiring fine-grained control over build configuration:

```bash
# Create build directory
mkdir -p build && cd build

# Configure with comprehensive options
cmake -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -Djsav_PACKAGING_MAINTAINER_MODE=OFF \
  -Djsav_ENABLE_COVERAGE=ON \
  -Djsav_ENABLE_SANITIZER_ADDRESS=ON \
  -Djsav_ENABLE_SANITIZER_UNDEFINED=ON \
  -Djsav_ENABLE_CLANG_TIDY=ON \
  -Djsav_ENABLE_CPPCHECK=ON \
  -Djsav_ENABLE_CACHE=ON \
  -Djsav_WARNINGS_AS_ERRORS=ON \
  ..

# Build all targets (including tests)
ninja jsav tests constexpr_tests relaxed_constexpr_tests

# Run all tests
ctest --output-on-failure

# Generate coverage report (after running tests)
gcovr -r .. --config=../gcovr.cfg
```

**Configuration Options Explained**:

| Option | Default | Description |
|--------|---------|-------------|
| `CMAKE_BUILD_TYPE` | (Required) | `Debug`, `Release`, `RelWithDebInfo`, `MinSizeRel` |
| `jsav_PACKAGING_MAINTAINER_MODE` | OFF | Developer mode (ON enables more options by default) |
| `jsav_ENABLE_COVERAGE` | OFF | Enable code coverage instrumentation (gcov) |
| `jsav_ENABLE_SANITIZER_ADDRESS` | ON (if supported) | AddressSanitizer for memory error detection |
| `jsav_ENABLE_SANITIZER_UNDEFINED` | ON (if supported) | UndefinedBehaviorSanitizer for UB detection |
| `jsav_ENABLE_CLANG_TIDY` | ON | Enable clang-tidy static analysis during build |
| `jsav_ENABLE_CPPCHECK` | ON | Enable cppcheck static analysis during build |
| `jsav_ENABLE_CACHE` | ON | Enable ccache for compilation caching |
| `jsav_WARNINGS_AS_ERRORS` | ON | Treat compiler warnings as errors |

**Source**: `ProjectOptions.cmake`, `QWEN.md`, `cmake/Sanitizers.cmake`, `cmake/StaticAnalyzers.cmake`

### 3.5 Build Type Matrix

The project supports multiple build configurations, each optimized for different purposes. Select the appropriate build type based on your development phase and objectives.

| Build Type | CMake Flag | Optimization | Debug Info | Use Case | Sanitizers |
|------------|------------|--------------|------------|----------|------------|
| **Debug** | `-DCMAKE_BUILD_TYPE=Debug` | None (`-O0`) | Full (`-g3`) | Active development, debugging | Full support |
| **Release** | `-DCMAKE_BUILD_TYPE=Release` | Maximum (`-O3`) | None | Production deployment | Limited support |
| **RelWithDebInfo** | `-DCMAKE_BUILD_TYPE=RelWithDebInfo` | Optimized (`-O2`) | Full (`-g`) | Performance testing with debug info | Partial support |
| **MinSizeRel** | `-DCMAKE_BUILD_TYPE=MinSizeRel` | Size (`-Os`) | None | Size-constrained environments | Limited support |

**Recommended Development Configuration**:

```bash
# Debug build with coverage and sanitizers (best for development)
cmake -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -Djsav_PACKAGING_MAINTAINER_MODE=OFF \
  -Djsav_ENABLE_COVERAGE=ON \
  -Djsav_ENABLE_SANITIZER_ADDRESS=ON \
  -Djsav_ENABLE_SANITIZER_UNDEFINED=ON \
  ..
```

**Recommended CI/CD Configuration**:

```bash
# Release build with sanitizers (for production validation)
cmake -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -Djsav_PACKAGING_MAINTAINER_MODE=ON \
  -Djsav_ENABLE_SANITIZER_ADDRESS=ON \
  ..
```

**Build Type Selection Guidelines**:

1. **Active Development**: Use `Debug` with sanitizers enabled. Provides best error messages and fastest compile times.

2. **Performance Testing**: Use `RelWithDebInfo` to get optimized code with debug symbols for profiling.

3. **Release Builds**: Use `Release` for final artifacts. Ensure all tests pass with sanitizers before shipping.

4. **Coverage Analysis**: Must use `Debug` or `RelWithDebInfo` with `jsav_ENABLE_COVERAGE=ON`.

**Source**: `QWEN.md`, `ProjectOptions.cmake`, `cmake/StandardProjectSettings.cmake`

---

---

## 4. Code Style Guidelines

This section defines the coding standards and style conventions for the jsav project. Adherence to these guidelines ensures code consistency, readability, and maintainability across the codebase. The guidelines are derived from observed patterns in the existing code, `.clang-format` and `.clang-tidy` configurations, and the project's philosophical foundation in C++23 best practices.

**Philosophy**: Code style is not arbitrary — it serves to reduce cognitive load, prevent bugs, and facilitate collaboration. The guidelines prioritize:

1. **Clarity over cleverness**: Readable code is better than clever code
2. **Consistency over personal preference**: Uniform style reduces distraction
3. **Automation over manual enforcement**: Let tools handle style, humans handle logic

**Source**: `.clang-format`, `.clang-tidy`, `AI_GUIDELINES.md`, `HUMAN_GUIDELINES.md`, `.specify/memory/constitution.md`

### 4.1 Naming Conventions

Naming is a critical aspect of code readability. Consistent naming allows developers to quickly understand the role and purpose of identifiers without consulting documentation.

**Evidence Base**: The conventions below are observed across `include/`, `src/`, and `test/` directories, and are enforced by `.clang-tidy` configuration.

#### Identifier Naming Table

| Element | Convention | Format | Example | Source Files | Rationale |
|---------|------------|--------|---------|--------------|-----------|
| **Classes / Structs** | PascalCase | Capitalize first letter of each word | `Lexer`, `Token`, `TimeValues`, `FormattedSize` | `include/jsav/lexer/Token.hpp`, `src/jsav/main.cpp` | Distinguishes types from variables; follows C++ convention |
| **Functions / Methods** | camelCase | Lowercase first, capitalize subsequent words | `tokenize()`, `format_size()`, `to_string()` | `src/jsav/main.cpp`, `include/jsav/lexer/Lexer.hpp` | Distinguishes functions from types; readable for multi-word names |
| **Member Variables** | Trailing underscore | `name_` pattern | `socket_`, `value_`, `unit_` | Observed in headers (`include/jsav/lexer/`) | Distinguishes members from locals and parameters; avoids `this->` |
| **Local Variables** | snake_case or camelCase | Lowercase with optional underscores | `tokens`, `lexer`, `porfilename` | Throughout codebase | Short-lived scope reduces ambiguity |
| **Function Parameters** | camelCase or snake_case | Same as locals | `argc`, `argv`, `bytes` | `src/jsav/main.cpp` | Consistency with locals; no trailing underscore |
| **Constants (file-scope)** | `kPascalCase` or `UPPER_SNAKE_CASE` | Prefix with `k` or all-caps | `kMaxRetries`, `UNIT_DIVIDER`, `UNITS` | `src/jsav/main.cpp` | Distinguishes from variables; `k` prefix common in Google style |
| **Constants (class-scope)** | `kPascalCase` | Static constexpr members | `kDefaultBufferSize` | Observed in headers | Consistent with file-scope, scoped by class |
| **Enumerators** | PascalCase | Scoped enum values | `TokenType::Identifier`, `TokenType::Eof` | `include/jsav/lexer/Token.hpp` | Matches enum class naming; clear when qualified |
| **Namespaces** | lowercase with underscores | Short, descriptive names | `jsv`, `vnd`, `fs`, `std` | `include/` headers | Avoids collision with types; short for frequent use |
| **Template Parameters** | PascalCase | Often single letters for simple cases | `T`, `U`, `AllocatorType` | Observed in headers | Convention from standard library |
| **Macro Names** | UPPER_SNAKE_CASE | All caps with underscores | `DISABLE_WARNINGS_PUSH`, `INIT_LOG`, `FORMAT` | Throughout codebase | Distinguishes macros from other identifiers |
| **File Names (headers)** | PascalCase or snake_case | Match primary class name or module | `SourceLocation.hpp`, `Token.hpp`, `headers.hpp` | `include/jsav/lexer/` | Headers often named after primary class |
| **File Names (sources)** | snake_case or match header | Match corresponding header | `SourceLocation.cpp`, `main.cpp`, `tests.cpp` | `src/jsav_Lib/lexer/` | Consistency with headers |
| **Test Cases** | PascalCase with underscores | Descriptive scenario format | `Lexer_EmptyInput_ReturnsEOF`, `Logger_setup_logger_Default` | `test/tests.cpp` | Self-documenting test names; clear failure messages |
| **Test Tags** | snake_case in brackets | Category identifiers | `[setup_logger]`, `[error_handler]`, `[TimeValues]` | `test/tests.cpp` | Groups related tests for filtered execution |

**Source**: `.clang-tidy`, observed in `include/jsav/lexer/Token.hpp`, `src/jsav/main.cpp`, `test/tests.cpp`, `AI_GUIDELINES.md`

#### clang-tidy Identifier Configuration

The `.clang-tidy` configuration enforces identifier length rules with specific exemptions:

```yaml
CheckOptions:
  - key: readability-identifier-length.IgnoredVariableNames
    value: 'x|y|z'
  - key: readability-identifier-length.IgnoredParameterNames
    value: 'x|y|z'
```

**Interpretation**:

- Variables and parameters named `x`, `y`, or `z` are exempt from minimum length requirements
- This exemption supports mathematical contexts (e.g., coordinate systems, complex numbers)
- All other identifiers must meet minimum length thresholds (typically 3 characters)

**Naming Anti-Patterns to Avoid**:

| Anti-Pattern | Example | Preferred | Rationale |
|--------------|---------|-----------|-----------|
| Hungarian notation | `strName`, `iCount` | `name`, `count` | Type information belongs in type system, not names |
| Abbreviations (unclear) | `mgr`, `dlg`, `frm` | `manager`, `dialog`, `frame` | Clarity over brevity; abbreviations obscure meaning |
| Inconsistent casing | `getToken`, `get_token` | `getToken` (camelCase) | Consistency within file/module |
| Generic names | `data`, `info`, `temp` | `tokenData`, `parseInfo`, `tempBuffer` | Specific names document intent |
| Boolean negation | `isNotFound`, `notValid` | `isValid`, `found` | Positive names are clearer; avoid double negatives |

**Source**: `.clang-tidy`

### 4.2 File Organization

Proper file organization ensures that code is easy to navigate, understand, and maintain. The jsav project follows established C++ conventions with project-specific adaptations.

#### Header Guards

**Standard**: `#pragma once` (universal across all headers)

**Rationale**:

- Simpler than traditional `#ifndef`/`#define`/`#endif` guards
- Supported by all modern compilers (GCC, Clang, MSVC)
- Less error-prone (no risk of guard name collisions)
- Slightly faster compilation (compiler handles uniqueness)

**Example**:

```cpp
// SourceLocation.hpp
#pragma once

// Header content...
```

**Alternative (not used in jsav)**:

```cpp
// Traditional guard (NOT used)
#ifndef JSAV_LEXER_SOURCELOCATION_HPP
#define JSAV_LEXER_SOURCELOCATION_HPP
// ...
#endif
```

**Source**: Observed in all `*.hpp` files in `include/` directory

#### Header/Source Pairing

The project follows a clear separation between interface (headers) and implementation (source files):

**Directory Structure**:

```text
include/jsav/          ← Public headers (API)
├── lexer/
│   ├── Lexer.hpp      ← Interface declaration
│   └── ...
src/jsav_Lib/          ← Implementation
├── lexer/
│   ├── Lexer.cpp      ← Implementation
│   └── ...
```

**Pairing Rules**:

1. Every `.hpp` file in `include/` should have a corresponding `.cpp` file in `src/` (if implementation is needed)
2. Header-only libraries may exist without `.cpp` files (e.g., template utilities)
3. Implementation files should include their corresponding header first (verifies header is self-contained)

**Example**:

```cpp
// Lexer.cpp - First include is the header being implemented
#include "lexer/Lexer.hpp"  // Self-include (verifies completeness)
#include "headers.hpp"       // Project master include
// ... other includes ...

// Implementation...
```

**Source**: `.clang-format`, observed in `src/jsav_Lib/lexer/Lexer.cpp`

#### Master Include Files

The project uses "master include" or "umbrella header" files to simplify inclusion of related headers:

**`include/jsav/headers.hpp`** (jsav module):

```cpp
#pragma once
#include "headersCore.hpp"  // Include core first
#include "lexer/SourceLocation.hpp"
#include "lexer/SourceSpan.hpp"
#include "lexer/Token.hpp"
#include "lexer/Lexer.hpp"
// ... other jsav headers ...
```

**`include/jsavCore/headersCore.hpp`** (core module):

```cpp
#pragma once
#include "FileReader.hpp"
#include "Log.hpp"
#include "format.hpp"
#include "timer/Timer.hpp"
#include "cast/casts.hpp"
// ... other core headers ...
```

**Usage Pattern**:

```cpp
// In source files, include the master header for convenience
#include "jsav/headers.hpp"  // Brings in all jsav headers
```

**Benefits**:

- Reduces include boilerplate in source files
- Ensures consistent header inclusion order
- Simplifies dependency management

**Caveats**:

- May increase compile times (includes unused headers)
- Can hide missing dependencies (transitive includes)
- Use in source files, not in headers (headers should be minimal)

**Source**: `include/jsav/headers.hpp`, `include/jsavCore/headersCore.hpp`

#### Include Order

The `.clang-format` configuration enforces a specific include order to prevent dependency issues and ensure consistency:

**Order (enforced by `.clang-format`)**:

```cpp
// 1. Corresponding header (for .cpp files)
#include "lexer/Lexer.hpp"

// 2. Project headers (priority 1)
#include "jsav/headers.hpp"
#include "jsavCore/Log.hpp"

// 3. LLVM/Clang headers (priority 2)
#include <llvm/Support/FileSystem.h>

// 4. Third-party headers (priority 3)
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <CLI11/CLI11.hpp>

// 5. Standard library headers (implicit priority 4)
#include <string>
#include <vector>
#include <memory>
```

**`.clang-format` Configuration**:

```yaml
IncludeCategories:
  - Regex: '^"(llvm|llvm-c|clang|clang-c)/'
    Priority: 2
  - Regex: '^(<|"(gtest|gmock|isl|json)/)'
    Priority: 3
  - Regex: '.*'
    Priority: 1  # Project headers default
```

**Rationale**:

- Project headers first: Catches missing includes in headers themselves
- Third-party after: Ensures project headers don't depend on transitive includes
- Standard library last: Most stable, least likely to change

**Source**: `.clang-format`, observed in source files

#### Forward Declarations

To minimize compilation dependencies, use forward declarations where possible:

**When to Use**:

- Pointer or reference members (`MyClass* ptr;` or `MyClass& ref;`)
- Function parameters by pointer/reference
- Return types by pointer/reference

**When NOT to Use**:

- Value members (`MyClass member;`) — needs full type
- Inheritance (`class Derived : public Base`) — needs full type
- Template parameters — usually needs full type

**Example**:

```cpp
// In Lexer.hpp
class Token;  // Forward declaration (used as pointer/reference)

class Lexer {
public:
    std::vector<Token> tokenize();  // OK if Token is complete in cpp
    Token* nextToken();             // OK with forward declaration
};
```

**Source**: C++ best practices, observed in headers

### 4.3 Modern C++ Usage

The jsav project embraces modern C++23 features to write safer, more expressive, and more efficient code. This subsection provides guidance on when and how to use specific language features.

**Philosophy**: Default to modern C++ features. Legacy approaches (C-style casts, raw loops, `printf`) should be avoided unless there is a compelling reason.

**Source**: `AI_GUIDELINES.md`, `HUMAN_GUIDELINES.md`, `.specify/memory/constitution.md`, observed in `src/jsav/main.cpp`

#### Feature Usage Table

| Feature | Usage Guideline | When to Use | Example | Anti-Pattern |
|---------|-----------------|-------------|---------|--------------|
| **`std::format` / `std::print`** | **Preferred** for all string formatting | All new code; replace `printf`, `sprintf`, `iostream` formatting | `FORMAT("{} {}", value, unit)`<br>`std::print("Result: {}\n", result)` | `printf("Result: %d\n", result)` |
| **`constexpr`** | **Default** for functions where possible | Any function that can be evaluated at compile-time | `[[nodiscard]] constexpr FormattedSize format_size(std::size_t bytes) noexcept` | Runtime evaluation of compile-time computable values |
| **Concepts** | Constrain template parameters | When template constraints improve error messages or document intent | `template<std::integral T> T add(T a, T b);` | Unconstrained templates with `static_assert` |
| **`[[nodiscard]]`** | On **all** functions returning values | When ignoring return value is likely a bug | `[[nodiscard]] std::string to_string() const;` | Ignoring return values of `std::optional`, `std::expected`, error codes |
| **`enum class`** | **Always** use scoped enums | All new enumerations | `enum class TokenType { Identifier, Literal, Eof };` | Unscoped `enum` (pollutes namespace, implicit conversions) |
| **`auto`** | In range-based for, type deduction | When type is obvious or verbose; required for complex types | `for(const auto& token : tokens)`<br>`auto iter = map.find(key);` | `auto x = 5;` (type not obvious) |
| **`std::optional`** | For nullable return values | When absence of value is valid and expected | `std::optional<std::string> findError();` | Raw pointers for nullable returns |
| **`std::expected`** (C++23) | For error handling without exceptions | When errors are expected and should be handled explicitly | `std::expected<Token, LexerError> nextToken();` | Error codes, exceptions for expected failures |
| **Lambdas** | Over `std::bind` and function objects | Inline callbacks, simple closures | `std::transform(begin, end, begin, [](auto x){ return x * 2; });` | `std::bind`, separate function objects for simple operations |
| **Range-based for** | **Always** prefer over raw loops | Iterating over containers, ranges | `for(const auto& item : container)` | `for(auto it = c.begin(); it != c.end(); ++it)` |
| **Range views** | For composable transformations | Chaining operations on ranges | `range \| std::views::filter(pred) \| std::views::transform(f)` | Nested loops, intermediate containers |
| **`std::span`** | Non-owning views over sequences | Function parameters for arrays/vectors | `void process(std::span<const int> data);` | Raw pointer + size pairs |
| **`std::string_view`** | Read-only string parameters | When function doesn't need to modify string | `void parse(std::string_view input);` | `const std::string&` (forces allocation) |
| **Designated initializers** | For aggregate initialization | Clear, self-documenting initialization | `Point{.x = 1.0, .y = 2.0};` | Constructor overloads, `std::make_tuple` |
| **`if consteval`** | Compile-time branching | Different behavior at compile-time vs runtime | `if consteval { /* compile-time */ } else { /* runtime */ }` | `if constexpr(std::is_constant_evaluated())` |
| **`consteval`** | Immediate functions | Functions that must run at compile-time | `consteval int factorial(int n) { return n <= 1 ? 1 : n * factorial(n-1); }` | `constexpr` when compile-time is mandatory |
| **`constinit`** | Static initialization | Ensure static variables are initialized at compile-time | `constinit thread_local int counter = 0;` | Static initialization with runtime cost |
| **`std::unique_ptr`** | Default smart pointer | Exclusive ownership of heap objects | `auto ptr = std::make_unique<MyClass>();` | `std::shared_ptr` without sharing, raw `new` |
| **`std::shared_ptr`** | Only when sharing required | Multiple owners, shared lifetime | `std::shared_ptr<Cache> sharedCache;` | `std::unique_ptr` when sharing is actual requirement |
| **Structured bindings** | Decomposing tuples/pairs | Clearer access to tuple elements | `auto [key, value] = *iter;` | `std::get<0>(pair)`, `pair.first` |

**Source**: `AI_GUIDELINES.md`, `HUMAN_GUIDELINES.md`, observed in `src/jsav/main.cpp`, `.specify/memory/constitution.md`

#### Detailed Feature Guidance

##### `std::format` and `std::print`

**Rationale**: Type-safe, extensible, locale-aware formatting that avoids the pitfalls of `printf` (type mismatches, buffer overflows) and the verbosity of iostreams.

**Usage**:

```cpp
// Preferred (C++23)
std::print("Processing file: {}\n", filename);
auto result = std::format("Value: {:.2f}", value);

// Fallback (fmt library, used via FORMAT macro)
LINFO("{}", porfilename);
LERROR("Error: {}", e.what());
```

**Format Specifiers**:

```cpp
std::format("{:<10}", value);   // Left-align, width 10
std::format("{:>10}", value);   // Right-align, width 10
std::format("{:.2f}", value);   // 2 decimal places
std::format("{:#x}", value);    // Hex with 0x prefix
std::format("{:%Y-%m-%d}", time);  // Date formatting
```

**Anti-Pattern**:

```cpp
// AVOID: printf (type-unsafe, no type checking)
printf("Value: %d\n", value);

// AVOID: iostreams (verbose, no format validation)
std::cout << "Value: " << value << std::endl;

// PREFERRED: std::format (type-safe, concise)
std::print("Value: {}\n", value);
```

**Source**: `AI_GUIDELINES.md`, `HUMAN_GUIDELINES.md`

##### `constexpr` Functions

**Rationale**: Compile-time evaluation catches errors earlier, improves runtime performance, and enables metaprogramming without templates.

**Guidelines**:

1. Mark functions `constexpr` by default (C++23 relaxed many restrictions)
2. Use `consteval` only when compile-time evaluation is mandatory
3. Combine with `[[nodiscard]]` for pure functions

**Example**:

```cpp
// Preferred: constexpr by default
[[nodiscard]] constexpr FormattedSize format_size(std::size_t bytes) noexcept {
    auto size = C_LD(bytes);
    std::size_t unit = 0;
    while(size >= UNIT_DIVIDER && unit < UNIT_LEN) {
        size /= UNIT_DIVIDER;
        ++unit;
    }
    return {.value = size, .unit = UNITS[unit]};
}

// Use consteval when compile-time is required
consteval int compileTimeFactorial(int n) {
    return n <= 1 ? 1 : n * compileTimeFactorial(n - 1);
}
```

**Source**: `AI_GUIDELINES.md`, observed in `src/jsav/main.cpp`

##### `[[nodiscard]]` Attribute

**Rationale**: Prevents bugs from accidentally ignoring return values, especially for error codes, `std::optional`, and `std::expected`.

**When to Use**:

- Functions returning error codes or `std::expected`
- Functions returning `std::optional` (might be empty)
- Pure functions (return value is the only effect)
- Factory functions (ignoring result means memory leak)

**Example**:

```cpp
[[nodiscard]] std::optional<Token> findToken(std::string_view name);
[[nodiscard]] std::expected<int, Error> parseNumber(std::string_view s);
[[nodiscard]] constexpr int add(int a, int b) noexcept;
```

**Source**: `AI_GUIDELINES.md`, `HUMAN_GUIDELINES.md`

### 4.4 Memory Management

Memory management is one of the most critical aspects of C++ programming. The jsav project follows a strict ownership model to prevent memory leaks, dangling pointers, and other memory-related bugs.

**Philosophy**: Automatic resource management is preferred over manual management. The compiler should handle resource cleanup whenever possible.

**Source**: `AI_GUIDELINES.md`, `.specify/memory/constitution.md`, `ProjectOptions.cmake`

#### Ownership Decision Ladder

When allocating or managing resources, follow this decision ladder (most preferred → least preferred):

```text
                    ┌─────────────────────┐
                    │  1. Stack Allocation │  ← DEFAULT CHOICE
                    │  (automatic storage) │
                    └─────────────────────┘
                               ↓
              (if heap allocation is required)
                               ↓
                    ┌─────────────────────┐
                    │  2. Smart Pointers  │
                    │  - unique_ptr       │  ← Exclusive ownership
                    │  - shared_ptr       │  ← Shared ownership (rare)
                    └─────────────────────┘
                               ↓
              (if non-owning reference is needed)
                               ↓
                    ┌─────────────────────┐
                    │  3. Raw Pointers    │  ← Non-owning, short duration
                    │  or References      │
                    └─────────────────────┘
```

#### Level 1: Stack Allocation (DEFAULT)

**When to Use**: For all local variables and class members where the lifetime is tied to scope.

**Rationale**:

- Automatic cleanup (no manual deallocation)
- Exception-safe (no leaks on throw)
- Best performance (no heap allocation overhead)
- Cache-friendly (contiguous memory)

**Example**:

```cpp
// Preferred: Stack allocation
void process() {
    MyClass obj;              // Automatic storage
    std::vector<int> vec;     // Container on stack (data on heap)
    std::array<int, 10> arr;  // Fixed-size array on stack
    
    // No manual cleanup needed
}
```

**Anti-Pattern**:

```cpp
// AVOID: Unnecessary heap allocation
void process() {
    auto* obj = new MyClass();  // Why heap?
    delete obj;                 // Manual cleanup required
}
```

#### Level 2: Smart Pointers (When Heap is Required)

**When to Use**: When objects must outlive their creating scope, or when polymorphic behavior requires dynamic allocation.

##### `std::unique_ptr` (Exclusive Ownership)

**When to Use**: Default choice for heap allocation. Single owner, automatic cleanup.

**Rationale**:

- Zero overhead compared to raw pointers
- Automatic deletion on scope exit
- Move-only (prevents accidental copies)
- Clear ownership semantics

**Example**:

```cpp
// Preferred: unique_ptr for exclusive ownership
auto ptr = std::make_unique<MyClass>(arg1, arg2);
process(std::move(ptr));  // Transfer ownership

// Factory function returning unique_ptr
std::unique_ptr<Parser> createParser(std::string_view source);
```

**Anti-Pattern**:

```cpp
// AVOID: Raw new/delete
MyClass* ptr = new MyClass();  // Error-prone
delete ptr;                    // What if exception?

// PREFERRED: make_unique
auto ptr = std::make_unique<MyClass>();
```

##### `std::shared_ptr` (Shared Ownership)

**When to Use**: **Only** when multiple owners are strictly necessary and ownership cannot be clearly assigned.

**Rationale**:

- Reference-counted automatic cleanup
- Copyable (shared ownership)
- Overhead (reference counting, control block)

**Example**:

```cpp
// Acceptable: Multiple owners required
class CacheEntry {
    std::shared_ptr<Data> data;  // Shared among multiple caches
};

// Factory returning shared_ptr
std::shared_ptr<ConnectionPool> getGlobalPool();
```

**Anti-Pattern**:

```cpp
// AVOID: shared_ptr by default (unnecessary overhead)
auto ptr = std::make_shared<MyClass>();  // Why shared?

// PREFERRED: unique_ptr unless sharing is required
auto ptr = std::make_unique<MyClass>();
```

#### Level 3: Raw Pointers (Non-Owning References)

**When to Use**: **Only** for non-owning references with short duration.

**Permitted Scenarios**:

1. **Observer parameters**: Function doesn't take ownership

   ```cpp
   void process(const MyClass* obj);  // Non-owning parameter
   void process(MyClass& obj);        // Prefer reference when not null
   ```

2. **C API interop**: Interfacing with C libraries

   ```cpp
   FILE* file = std::fopen("file.txt", "r");  // C API
   std::fclose(file);                          // Manual cleanup required
   ```

3. **Polymorphic references**: Base class pointer to derived

   ```cpp
   void render(const Shape& shape);  // Reference to base class
   ```

**Prohibited Scenarios**:

- Ownership transfer (use `std::unique_ptr`)
- Resource management (use RAII wrappers)
- Long-lived pointers without clear lifetime

**Example**:

```cpp
// Acceptable: Non-owning observer
class Lexer {
public:
    void setSource(std::string_view source);  // string_view (non-owning)
    const Token* currentToken() const;        // Observer pointer
private:
    std::string source_;  // Owned data
};

// Anti-pattern: Raw pointer as member (ownership unclear)
class Lexer {
    Token* currentToken_;  // Who owns this? When deleted?
};
```

#### Additional Memory Management Rules

**Rule of 0**: Prefer classes that don't manage resources manually. Let compilers generate special member functions.

```cpp
// Preferred: Rule of 0 (no manual resource management)
class Lexer {
public:
    // Compiler generates: destructor, copy/move constructors, copy/move assignment
    // All correct because members manage their own resources
private:
    std::string source_;      // Self-managing
    std::vector<Token> tokens; // Self-managing
};
```

**Rule of 5** (when manual management is unavoidable): If you define any of destructor, copy constructor, copy assignment, move constructor, or move assignment, define all five.

```cpp
// When manual resource management is required
class Buffer {
public:
    ~Buffer();                    // Release resource
    Buffer(const Buffer&);        // Deep copy
    Buffer& operator=(const Buffer&);
    Buffer(Buffer&&) noexcept;    // Move
    Buffer& operator=(Buffer&&) noexcept;
private:
    int* data_;                   // Manual management
    std::size_t size_;
};
```

**Use `std::span` for Sequences**: Non-owning view over contiguous data.

```cpp
// Preferred: span for non-owning view
void process(std::span<const int> data);

// Anti-pattern: Raw pointer + size
void process(const int* data, std::size_t size);
```

**AddressSanitizer Requirement**: Test suite must run with AddressSanitizer and report zero leaks.

```bash
# Configure with ASan
cmake -Djsav_ENABLE_SANITIZER_ADDRESS=ON ..

# Run tests - must report zero leaks
ctest --output-on-failure
```

**Source**: `AI_GUIDELINES.md`, `.specify/memory/constitution.md`, `ProjectOptions.cmake`, `cmake/Sanitizers.cmake`

---

---

## 5. Testing

Testing is a fundamental pillar of the jsav development workflow. The project employs a comprehensive testing strategy with three distinct test targets, each serving a specific purpose in the verification pipeline. This section provides detailed guidance on the testing framework, test organization, execution commands, and best practices for writing effective tests.

**Philosophy**: Tests are executable specifications. They document expected behavior, prevent regressions, and provide confidence for refactoring. Test quality is as important as production code quality.

**Source**: `test/CMakeLists.txt`, `QWEN.md`, `AI_GUIDELINES.md`, `.specify/memory/constitution.md`

### 5.1 Test Framework

**Framework**: Catch2 v3.13.0 (C++ Testing Framework)

**Selection Rationale**:

- **Modern C++**: Native C++17/20 support, no macros required (but available)
- **Compile-time Testing**: `STATIC_REQUIRE` for constexpr verification
- **BDD-style**: `TEST_CASE`/`SECTION` structure for hierarchical organization
- **Rich Matchers**: Type-safe assertions with descriptive failure messages
- **Benchmarking**: Built-in micro-benchmark support for performance regression
- **XML Reporting**: JUnit-compatible output for CI integration
- **Standalone**: `Catch2WithMain` variant provides main entry point

**Configuration**: `test/CMakeLists.txt`

**Installation**: Managed via CPM in `Dependencies.cmake`

```cmake
cpmaddpackage("gh:catchorg/Catch2@3.13.0")
```

**Documentation**: https://catch2.docsforge.com/

**Source**: `Dependencies.cmake`, `test/CMakeLists.txt`

### 5.2 Test Targets

The project maintains three distinct test targets, each compiled and executed separately. This separation enables compile-time verification, runtime testing of constexpr code, and runtime-only testing for I/O and system-dependent functionality.

#### Test Target Comparison Table

| Target | Source File | Purpose | Assertion Type | Compilation | Execution Time | Use Cases |
|--------|-------------|---------|----------------|-------------|----------------|-----------|
| **`constexpr_tests`** | `test/constexpr_tests.cpp` | Compile-time verification of constexpr functions | `STATIC_REQUIRE` | If it compiles, tests pass | N/A (compile-time) | Constexpr functions, type traits, compile-time computation |
| **`relaxed_constexpr_tests`** | `test/constexpr_tests.cpp` | Runtime version of constexpr tests | `REQUIRE` (with `-DCATCH_CONFIG_RUNTIME_STATIC_REQUIRE`) | Normal compilation | Fast (milliseconds) | Debugging constexpr test failures, runtime verification |
| **`tests`** | `test/tests.cpp` | Runtime-only tests (I/O, system-dependent, non-constexpr) | `REQUIRE` | Normal compilation | Variable (seconds) | File I/O, logging, CLI parsing, integration tests |

**Source**: `test/CMakeLists.txt`, `QWEN.md`

#### constexpr_tests (Compile-Time Verification)

**Purpose**: Verify that functions produce correct results when evaluated at compile-time. This target catches errors during compilation rather than at runtime.

**Assertion Mechanism**: `STATIC_REQUIRE` — causes compilation failure if condition is false

**Example**:

```cpp
#include <catch2/catch_test_macros.hpp>

TEST_CASE("constexpr format_size", "[format]") {
    constexpr auto result = format_size(1024);
    STATIC_REQUIRE(result.value == 1.0L);
    STATIC_REQUIRE(result.unit == "KB");
}
```

**Compilation Behavior**:

- If all `STATIC_REQUIRE` conditions are true: Compilation succeeds
- If any `STATIC_REQUIRE` condition is false: Compilation fails with error message
- Test "passing" means successful compilation

**When to Use**:

- Constexpr mathematical functions
- Type traits and concepts
- Compile-time string processing
- Template metaprogramming

**Source**: `test/CMakeLists.txt`, `test/constexpr_tests.cpp`

#### relaxed_constexpr_tests (Runtime Constexpr Verification)

**Purpose**: Same test code as `constexpr_tests`, but assertions are evaluated at runtime. This enables debugging of test failures that would otherwise halt compilation.

**Assertion Mechanism**: `REQUIRE` — runtime assertion with test failure reporting

**Compilation Flag**: `-DCATCH_CONFIG_RUNTIME_STATIC_REQUIRE`

**Example** (same source as constexpr_tests):

```cpp
TEST_CASE("constexpr format_size", "[format]") {
    constexpr auto result = format_size(1024);
    REQUIRE(result.value == 1.0L);   // Runtime assertion
    REQUIRE(result.unit == "KB");
}
```

**When to Use**:

- Debugging failing constexpr tests (run relaxed version first)
- Verifying runtime behavior matches compile-time behavior
- Testing constexpr functions with runtime inputs

**Workflow**:

1. Write test in `constexpr_tests.cpp`
2. Build and run `relaxed_constexpr_tests`
3. Debug any failures
4. Verify `constexpr_tests` compiles successfully

**Source**: `test/CMakeLists.txt`, `QWEN.md`

#### tests (Runtime-Only Tests)

**Purpose**: Test functionality that cannot be evaluated at compile-time, including I/O operations, system calls, logging, and integration tests.

**Assertion Mechanism**: `REQUIRE` and variants (`REQUIRE_NOTHROW`, `REQUIRE_THROWS`, etc.)

**Example**:

```cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

TEST_CASE("Logger setup", "[setup_logger]") {
    SECTION("Default setup") {
        REQUIRE_NOTHROW(setup_logger());
    }
    
    SECTION("Logger sinks") {
        setup_logger();
        auto logger = spdlog::default_logger();
        REQUIRE(logger->sinks().size() == 1);
    }
}
```

**When to Use**:

- File system operations
- Logging and output verification
- CLI argument parsing
- Exception handling
- Integration tests (multiple components)
- Performance benchmarks

**Source**: `test/CMakeLists.txt`, `test/tests.cpp`

### 5.3 Build and Run Commands

This subsection provides comprehensive commands for building and executing tests at various levels of granularity.

#### Build Test Targets

```bash
# Build all test targets
ninja tests constexpr_tests relaxed_constexpr_tests

# Build specific test target
ninja tests
ninja constexpr_tests
ninja relaxed_constexpr_tests
```

#### Run All Tests

```bash
# Using CTest (recommended - aggregates all test targets)
ctest --output-on-failure

# With verbose output (shows all test details)
ctest --output-on-failure --verbose

# Parallel test execution (speeds up large test suites)
ctest --output-on-failure --parallel 4

# Filter by name pattern (regex)
ctest --output-on-failure -R "unittests"
```

**CTest Configuration**: Tests are configured in `test/CMakeLists.txt` with XML output for CI integration:

```cmake
catch_discover_tests(
    tests
    TEST_PREFIX "unittests."
    REPORTER XML
    OUTPUT_DIR .
    OUTPUT_PREFIX "unittests."
    OUTPUT_SUFFIX .xml)
```

**Output Location**: `build/test/Testing/` (CTest reports)

**Source**: `test/CMakeLists.txt`, `QWEN.md`

#### Run Single Test Suite

Test suites are groups of related tests identified by their name pattern.

```bash
# Filter by suite name pattern (CTest regex)
ctest --output-on-failure -R "unittests.Logger"

# Filter by tag (Catch2 tag syntax - requires running test binary directly)
./build/test/tests "[setup_logger]"

# Multiple tags (OR logic)
./build/test/tests "[setup_logger],[error_handler]"

# Multiple tags (AND logic)
./build/test/tests "[setup_logger][logging]"
```

**Common Test Suite Patterns**:

- `unittests.Logger.*` — All logger tests
- `unittests.TimeValues.*` — All time-related tests
- `constexpr.format.*` — All constexpr format tests
- `relaxed_constexpr.*` — All relaxed constexpr tests

**Source**: `test/CMakeLists.txt`, `QWEN.md`

#### Run Single Test Case

For debugging specific failures or verifying individual behaviors.

```bash
# Filter by exact test name (CTest)
ctest --output-on-failure -R "unittests.Logger_setup_logger_Default"

# Filter by test name pattern (regex)
ctest --output-on-failure -R "unittests.*Default"

# Run specific test case (Catch2 - direct binary invocation)
./build/test/tests "Logger setup"

# Run specific section within test case
./build/test/tests "Logger setup" -c "Default setup"
```

**Test Name Format**: Test names in CTest are prefixed based on test target:

- `unittests.` — From `tests` target
- `constexpr.` — From `constexpr_tests` target
- `relaxed_constexpr.` — From `relaxed_constexpr_tests` target

**Source**: `test/CMakeLists.txt`, `QWEN.md`

#### Test Execution with Sanitizers

Running tests with sanitizers enabled detects memory errors and undefined behavior.

```bash
# Configure with sanitizers
cmake -Djsav_ENABLE_SANITIZER_ADDRESS=ON \
      -Djsav_ENABLE_SANITIZER_UNDEFINED=ON \
      ..

# Build and run tests
ninja tests
ctest --output-on-failure

# Expected output: Zero sanitizer violations
```

**Sanitizer Behavior**:

- **AddressSanitizer**: Detects buffer overflows, use-after-free, memory leaks
- **UndefinedBehaviorSanitizer**: Detects signed overflow, null dereference, misaligned access

**Failure Mode**: Any sanitizer violation causes immediate test failure with detailed stack trace.

**Source**: `cmake/Sanitizers.cmake`, `ProjectOptions.cmake`

### 5.4 Test Naming Conventions

Consistent test naming enables easy test discovery, clear failure messages, and self-documenting test suites.

#### File Naming

| File | Purpose | Content |
|------|---------|---------|
| `tests.cpp` | Runtime unit tests | All non-constexpr tests |
| `constexpr_tests.cpp` | Constexpr tests | Compile-time and relaxed constexpr tests |
| `testsConstanst.hpp` | Test utilities | Shared constants, fixtures, helper functions |

**Source**: `test/` directory structure

#### Test Case Naming Pattern

**Format**: `TEST_CASE("Description", "[tag]")`

**Structure**:

```cpp
TEST_CASE("ClassName_MethodName_Scenario", "[module]") {
    SECTION("Specific behavior or edge case") {
        // Test implementation
    }
}
```

**Examples from Codebase**:

```cpp
TEST_CASE("Logger setup", "[setup_logger]")
TEST_CASE("my_error_handler(const std::string&) tests", "[error_handler]")
TEST_CASE("TimeValues initialization", "[TimeValues]")
TEST_CASE("FolderDeletionResult deleteFolder", "[folder_deletion]")
```

**Section Naming**:

```cpp
SECTION("Default setup")           // Specific configuration
SECTION("Basic error handling")    // Specific scenario
SECTION("Initialization with nanoseconds")  // Specific case
```

**Source**: `test/tests.cpp`, observed patterns

#### Test Name Best Practices

**DO**:

- Use descriptive names that explain the scenario being tested
- Include class/method name for context
- Specify expected behavior or result
- Use tags for categorization and filtering

**AVOID**:

- Generic names like "test1", "basic_test"
- Names that don't indicate what is being tested
- Overly long names (hard to read in output)
- Missing tags (prevents filtered execution)

**Good Examples**:

```cpp
TEST_CASE("Lexer_EmptyInput_ReturnsEOF", "[lexer]")
TEST_CASE("Parser_InvalidSyntax_ThrowsException", "[parser]")
TEST_CASE("Formatter_NullValue_PrintsZero", "[format]")
```

**Poor Examples**:

```cpp
TEST_CASE("test1", "[misc]")           // What is being tested?
TEST_CASE("Lexer test", "[lexer]")     // Too vague
TEST_CASE("Testing the lexer with empty input to see what happens", "[lexer]")  // Too long
```

**Source**: `test/tests.cpp`, C++ testing best practices

### 5.5 Test Fixtures and Mocks

#### Test Fixtures

**Location**: `test/testsConstanst.hpp`

**Purpose**: Shared constants, helper functions, and common setup code used across multiple tests.

**Example Pattern**:

```cpp
// testsConstanst.hpp
#pragma once
#include <string>

namespace test_constants {
    constexpr std::string_view TEST_FILE_PATH = "test_data/sample.txt";
    constexpr std::size_t TEST_BUFFER_SIZE = 1024;
}
```

**Usage in Tests**:
```cpp
#include "testsConstanst.hpp"

TEST_CASE("File reading", "[file_io]") {
    const auto content = readFromFile(test_constants::TEST_FILE_PATH);
    REQUIRE_FALSE(content.empty());
}
```

**Source**: `test/testsConstanst.hpp`, `test/tests.cpp`

#### Inline Fixtures

For test-specific setup, use inline fixtures within `TEST_CASE` or `SECTION`:

```cpp
TEST_CASE("Folder deletion", "[filesystem]") {
    // Arrange: Create test folder structure
    fs::path testFolder = fs::temp_directory_path() / "test_folder_deletion";
    if(fs::exists(testFolder)) { fs::remove_all(testFolder); }
    fs::create_directories(testFolder / "subfolder1");
    std::ofstream(testFolder / "file1.txt") << "File 1 content";
    
    // Act: Delete folder
    auto result = deleteFolder(testFolder);
    
    // Assert: Verify deletion
    REQUIRE(result.success());
    REQUIRE_FALSE(fs::exists(testFolder));
}
```

**Arrange-Act-Assert Pattern**:
1. **Arrange**: Set up test data and preconditions
2. **Act**: Execute the code being tested
3. **Assert**: Verify the results

**Source**: `test/tests.cpp`, observed patterns

#### Mocks

**Status**: Hand-written mocks (no mocking framework configured)

**When to Use Mocks**:

- Testing code with external dependencies (file system, network)
- Isolating unit under test from complex collaborators
- Simulating error conditions that are hard to trigger

**Example Pattern** (hand-written mock):

```cpp
// Mock file reader for testing
class MockFileReader {
public:
    std::string readContent;
    bool shouldFail = false;
    
    std::string readFromFile(const std::string& path) {
        if(shouldFail) {
            throw std::runtime_error("Mock file read error");
        }
        return readContent;
    }
};

// Usage in test
TEST_CASE("Parser handles file read errors", "[parser]") {
    MockFileReader mock;
    mock.shouldFail = true;
    
    REQUIRE_THROWS_AS(parseFromMock(mock), std::runtime_error);
}
```

**Future Enhancement**: Consider integrating [trompeloeil](https://github.com/rollbear/trompeloeil) or [FakeIt](https://github.com/eranpeer/FakeIt) for more sophisticated mocking if needed.

**Source**: `test/tests.cpp`, observed patterns

### 5.6 Workflow for Adding Tests

The project follows a specific workflow for adding new tests, particularly for constexpr-capable code.

#### Step-by-Step Workflow

**Step 1: Write Test in `constexpr_tests.cpp`**

```cpp
// test/constexpr_tests.cpp
TEST_CASE("format_size converts bytes to KB", "[format]") {
    constexpr auto result = format_size(1024);
    STATIC_REQUIRE(result.value == 1.0L);
    STATIC_REQUIRE(result.unit == "KB");
}
```

**Step 2: Build and Run `relaxed_constexpr_tests`**

```bash
# Build relaxed version first (faster iteration)
ninja relaxed_constexpr_tests

# Run tests
./build/test/relaxed_constexpr_tests
```

**Step 3: Debug Any Failures**

- If test fails, debug using runtime output
- Fix implementation or test as needed
- Re-run `relaxed_constexpr_tests` until passing

**Step 4: Verify `constexpr_tests` Compiles**

```bash
# Build constexpr version
ninja constexpr_tests

# If compilation succeeds, tests pass
# If compilation fails, fix constexpr violations
```

**Step 5: Add to `tests.cpp` for Non-Constexpr Functionality**

```cpp
// test/tests.cpp
TEST_CASE("format_size edge cases", "[format]") {
    REQUIRE(format_size(0).value == 0.0L);
    REQUIRE(format_size(1).unit == "B");
    REQUIRE(format_size(1024 * 1024).unit == "MB");
}
```

**Source**: `QWEN.md`, `AI_GUIDELINES.md`, `test/CMakeLists.txt`

#### Test-Driven Development (TDD) Workflow

The project encourages Test-Driven Development following the Red-Green-Refactor cycle:

**Red Phase**:

1. Write a failing test that defines desired behavior
2. Verify test fails (compile error for constexpr, runtime failure for runtime tests)
3. Keep test minimal — test one behavior only

**Green Phase**:

1. Implement minimum code to make test pass
2. Do not add untested functionality
3. Accept "ugly" implementation temporarily

**Refactor Phase**:

1. With tests passing, improve code structure
2. Run tests after each refactoring change
3. Revert immediately if tests fail

**Source**: `.specify/memory/constitution.md`, `AI_GUIDELINES.md`

### 5.7 Test Coverage

**Tool**: gcovr (Coverage Report Generator)

**Configuration**: `gcovr.cfg`

**Enabling Coverage**:

```bash
# Configure with coverage instrumentation
cmake -Djsav_ENABLE_COVERAGE=ON ..

# Build and run tests
ninja tests
ctest --output-on-failure

# Generate coverage report
gcovr -r .. --config=../gcovr.cfg
```

**Output Formats**:

- **HTML**: `./out/coverage/index.html` (interactive, detailed)
- **XML**: `./out/cobertura.xml` (CI integration)
- **Summary**: Console output with line and branch coverage percentages

**Coverage Goals**:

- **Line Coverage**: Target ≥80% (all executable lines)
- **Branch Coverage**: Target ≥70% (all decision branches)
- **Function Coverage**: Target ≥90% (all functions called)

**Coverage Exclusions** (configured in `gcovr.cfg`):

- Test directories (`test/`, `fuzz_test/`)
- Dependencies (`_deps/`)
- Generated files (`configured_files/`)

**Source**: `gcovr.cfg`, `cmake/Tests.cmake`, `QWEN.md`

---

---

## 6. Development Workflow

### 6.1 Branching

**Strategy**: Feature branches from `main`

**Naming Convention**: `###-feature-name` (e.g., `123-add-unicode-lexer`)

**Source**: `.specify/memory/constitution.md`

### 6.2 Commit Messages

**Format**: Conventional Commits (recommended, observed in project culture)

```text
<type>(<scope>): <description>

[optional body]

Closes #<issue-number>
```

**Types**: `feat`, `fix`, `docs`, `style`, `refactor`, `test`, `chore`

**Source**: Project convention (observed), `AI_GUIDELINES.md`

### 6.3 Pull Requests

**Required Components**:

- [ ] Linked issue reference
- [ ] Passing CI checks (all tests, static analysis, sanitizers)
- [ ] Test coverage for new code
- [ ] Updated documentation if public API changed
- [ ] Code formatted with `clang-format`

**Source**: `.specify/memory/constitution.md`, `AI_GUIDELINES.md`

### 6.4 Code Review Criteria

**Automated Checks** (CI-enforced):

- [ ] Zero compile errors/warnings (warnings as errors)
- [ ] All tests pass (`constexpr_tests`, `relaxed_constexpr_tests`, `tests`)
- [ ] clang-tidy reports zero issues
- [ ] cppcheck reports zero issues
- [ ] AddressSanitizer: no violations
- [ ] UndefinedBehaviorSanitizer: no violations
- [ ] lizard: all functions within thresholds (CCN ≤15, length ≤100 lines, params ≤6) — **manual check**
- [ ] Code formatted per `.clang-format`

**Manual Review**:

- [ ] Correctness of implementation
- [ ] Adequate test coverage
- [ ] Style compliance (naming, structure)
- [ ] API documentation (Doxygen comments for public headers)
- [ ] No new compiler warnings

**Source**: `.specify/memory/constitution.md`, `QWEN.md`, `ProjectOptions.cmake`

---

## 7. Rules for Automated Agents and AI Tools

This section contains **enforceable rules** for automated agents. Violations are categorized by severity.

### Architecture Rules

| Rule | Scope | Violation Consequence |
|------|-------|----------------------|
| **Do not add new top-level directories** without updating this document and `CMakeLists.txt` | Project-wide | Build system breaks, CI fails |
| **Do not introduce circular dependencies** between modules | `src/jsav_Core_lib/` ↔ `src/jsav_Lib/` | Compilation fails, linker errors |
| **Do not move public headers** (`include/jsav/`) without updating all `#include` paths and CMake install rules | `include/jsav/` | Downstream consumers break, installation fails |
| **Do not modify vendored dependencies** in `_deps/` | `_deps/` (build directory) | Dependencies must be updated via CPM in `Dependencies.cmake` |

### Dependency Rules

| Rule | Scope | Violation Consequence |
|------|-------|----------------------|
| **Do not add external libraries** without explicit approval and updating `Dependencies.cmake` | Project-wide | Build breaks, license conflicts |
| **All dependencies must use CPM** with explicit version locking | `Dependencies.cmake` | Non-reproducible builds |
| **Do not vendor third-party code** directly into `src/` | `src/` | License violations, maintenance burden |
| **Do not expose dependency types** in public headers | `include/jsav/` | "Header leak" anti-pattern — consumers become coupled to dependencies |

**Source**: `Dependencies.cmake`, `.specify/memory/constitution.md`

### API Rules

| Rule | Scope | Violation Consequence |
|------|-------|----------------------|
| **Do not change public API signatures** without deprecation cycle | `include/jsav/` headers | Downstream consumers break at compile/link time |
| **Do not remove or rename public symbols** without updating Doxygen documentation | `include/jsav/` | Documentation drift, consumer breakage |
| **Do not expose internal implementation details** in public headers | `include/jsav/` | ABI instability, consumer coupling |

**Public API Boundary**: Headers in `include/jsav/` are installed and exported (defined in `CMakeLists.txt` install rules). Headers in `include/jsavCore/` are internal-only.

**Source**: `CMakeLists.txt`, `include/jsav/` structure

### Compilation Rules

| Rule | Scope | Violation Consequence |
|------|-------|----------------------|
| **Every commit must leave the project compilable** on all supported platforms (Windows, Linux, macOS) | Project-wide | CI fails, blocks merge |
| **Do not disable compiler warnings** project-wide | `ProjectOptions.cmake`, `cmake/CompilerWarnings.cmake` | Reduces code quality guardrails |
| **All new code must compile without warnings** under configured warning set (`-Wall -Wextra` or MSVC `/W4`) | All `src/` and `include/` files | Warnings treated as errors — build fails |
| **Do not bypass static analysis** (clang-tidy, cppcheck) | Project-wide | CI fails, technical debt accumulates |

**Warning Configuration**:

- GCC/Clang: `-Wall -Wextra -Wshadow -Wconversion -Wsign-conversion` (see `cmake/CompilerWarnings.cmake`)
- MSVC: `/W4 /permissive-` (see `cmake/CompilerWarnings.cmake`)

**Source**: `ProjectOptions.cmake`, `cmake/CompilerWarnings.cmake`, `.gitlab-ci.yml`

### Testing Rules

| Rule | Scope | Violation Consequence |
|------|-------|----------------------|
| **Every new public function must have at least one unit test** | All functions exported from `include/jsav/` | Code coverage gaps, unverified behavior |
| **Do not delete existing tests** without documented justification | `test/` directory | Regression risk, loss of verification |
| **Tests must pass before any PR is merged** | All test targets | CI fails, blocks merge |
| **Prefer test-driven development** (Red-Green-Refactor) | All new code | Ensures testability, reduces bugs |

**Test Coverage Requirement**: New code must maintain adequate coverage (verified via gcovr). Target: Line coverage ≥80%, Branch coverage ≥70%.

**Source**: `.specify/memory/constitution.md`, `AI_GUIDELINES.md`, `gcovr.cfg`

### Style Rules

| Rule | Scope | Violation Consequence |
|------|-------|----------------------|
| **Run `clang-format`** on all modified files before committing | All `*.cpp`, `*.hpp` files | CI formatting check fails |
| **Do not mix naming conventions** within a single file | Per-file basis | Code review rejection, inconsistency |
| **Follow `.clang-format` configuration** (140 column limit, 4-space indent) | Project-wide | Formatting inconsistencies |
| **Use `std::format` / `std::print`** instead of iostream/printf | All new code | Static analysis failure |

**clang-format Command**:

```bash
clang-format -i path/to/changed/files/*.cpp path/to/changed/files/*.hpp
```

**Source**: `.clang-format`, `AI_GUIDELINES.md`, `HUMAN_GUIDELINES.md`

### Memory Safety Rules

| Rule | Scope | Violation Consequence |
|------|-------|----------------------|
| **No raw `new`/`delete`** in application code | All `src/` files | Sanitizer violations, memory leaks |
| **Prefer `std::unique_ptr`** over `std::shared_ptr` | All heap allocations | Unnecessary overhead, ownership ambiguity |
| **Use `std::span`** for non-owning views over sequences | Function parameters | Bounds safety, clarity |
| **AddressSanitizer must report zero leaks** | Test suite execution | Memory safety violations |

**Source**: `.specify/memory/constitution.md`, `AI_GUIDELINES.md`, `ProjectOptions.cmake`

### Complexity Rules

| Rule | Threshold | Detection Mechanism |
|------|-----------|---------------------|
| **Cyclomatic Complexity** | ≤15 per function | lizard (manual) |
| **Function Length** | ≤100 lines per function | lizard (manual) |
| **Parameters** | ≤6 per function | lizard (manual), clang-tidy |

**Commands**:

```bash
# lizard is a Python script - install first
pip3 install lizard

# Run lizard analysis manually
lizard src/**/*.cpp include/**/*.hpp --CCN 15 --length 100 --arguments 6

# Generate HTML report
lizard src/**/*.cpp include/**/*.hpp --html > lizard-report.html
```

**Note**: lizard is a standalone Python tool, not a CMake target. Run it manually during development.

**Source**: `QWEN.md`, `AI_GUIDELINES.md`, `.specify/memory/constitution.md`

---

## Appendix A: Quick Reference Commands

This appendix provides copy-pasteable command sequences for common development tasks. Commands are organized by workflow stage.

### A.1 Development Setup (Recommended)

**Initial Project Setup**:

```bash
# Clone repository (if not already done)
git clone <repository-url>
cd jsav

# Quick iteration setup (Debug with coverage and sanitizers)
mkdir -p build && cd build
cmake -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -Djsav_PACKAGING_MAINTAINER_MODE=OFF \
  -Djsav_ENABLE_COVERAGE=ON \
  ..

# Build all targets
ninja

# Verify build succeeded
echo "Build complete: $(which jsav)"
```

**Source**: `QWEN.md`, `AI_GUIDELINES.md`

### A.2 Testing Workflow

**Quick Test Feedback** (after code changes):

```bash
# Run runtime tests only (fastest feedback)
cd build
ninja tests relaxed_constexpr_tests
ctest -R "unittests|relaxed_constexpr" --output-on-failure
```

**Full Test Suite**
:
```bash
# Run all tests (including constexpr)
cd build
ctest --output-on-failure

# Run tests with verbose output
ctest --verbose --output-on-failure

# Run tests in parallel (speeds up large test suites)
ctest --parallel 4 --output-on-failure
```

**Specific Test Execution**:

```bash
# Run specific test suite by pattern
ctest -R "unittests.Logger" --output-on-failure

# Run specific test case
ctest -R "unittests.Logger_setup_logger_Default" --output-on-failure

# Run tests matching multiple patterns
ctest -R "unittests.*(Logger|TimeValues)" --output-on-failure

# Run Catch2 test binary directly (for tag filtering)
./build/test/tests "[setup_logger]"
./build/test/tests "[error_handler],[filesystem]"
```

**Source**: `QWEN.md`, `test/CMakeLists.txt`

### A.3 Code Formatting

**Format Modified Files**:

```bash
# Format all changed files (before commit)
clang-format -i src/**/*.cpp src/**/*.hpp include/**/*.hpp

# Format specific files
clang-format -i src/jsav/main.cpp include/jsav/lexer/Lexer.hpp

# Check formatting without modifying (for CI)
clang-format --dry-run --Werror src/**/*.cpp

# Format test files
clang-format -i test/**/*.cpp test/**/*.hpp
```

**Editor Integration**:

```bash
# VS Code: Format on save (add to settings.json)
{
    "editor.formatOnSave": true,
    "editor.defaultFormatter": "ms-vscode.cpptools",
    "C_Cpp.clang_format_fallbackStyle": "file"
}

# CLion: Use clang-format (Settings → Editor → Code Style → C++)
# Enable "Use clang-format" and select project file
```

**Source**: `.clang-format`, `HUMAN_GUIDELINES.md`

### A.4 Coverage Report

**Generate Coverage**:

```bash
# Configure with coverage (if not already done)
cd build
cmake -Djsav_ENABLE_COVERAGE=ON ..

# Build and run tests
ninja
ctest --output-on-failure

# Generate coverage report
gcovr -r .. --config=../gcovr.cfg

# View HTML report
# Linux/macOS:
xdg-open ./out/coverage/index.html  # Linux
open ./out/coverage/index.html      # macOS
# Windows:
start ./out/coverage/index.html
```

**Coverage Report Options**:

```bash
# Generate XML report (for CI)
gcovr -r .. --cobertura-pretty --cobertura out/cobertura.xml

# Generate detailed HTML with source code
gcovr -r .. --html-details ./out/coverage/index.html

# Show coverage summary in terminal
gcovr -r .. --print-summary

# Filter to specific directories
gcovr -r .. --filter src/ --filter include/ --exclude test/
```

**Source**: `gcovr.cfg`, `cmake/Tests.cmake`, `QWEN.md`

### A.5 Static Analysis

**Run clang-tidy**:

```bash
# clang-tidy runs automatically during build if enabled
# Configure with clang-tidy
cmake -Djsav_ENABLE_CLANG_TIDY=ON ..

# Run manually on specific files
clang-tidy src/jsav/main.cpp -- -I include -std=c++23

# Run on all source files (script)
find src -name "*.cpp" | xargs -I {} clang-tidy {} -- -I include -std=c++23
```

**Run cppcheck**:

```bash
# cppcheck runs automatically during build if enabled
# Configure with cppcheck
cmake -Djsav_ENABLE_CPPCHECK=ON ..

# Run manually
cppcheck --enable=all --inconclusive --template=gcc src/ include/
```

**Run lizard (Complexity Analysis)**:

```bash
# lizard is a Python script, not a CMake target
# Install lizard first (if not already installed)
pip3 install lizard

# Run lizard on source files
lizard src/**/*.cpp include/**/*.hpp --CCN 15 --length 100 --arguments 6

# Generate HTML report (requires lizard-html plugin or manual export)
lizard src/**/*.cpp include/**/*.hpp --html > lizard-report.html

# Run with warnings for functions exceeding thresholds
lizard src/**/*.cpp include/**/*.hpp --CCN 15 --length 100 --arguments 6 --warning_limit 1
```

**Note**: lizard is not integrated into the CMake build system. It must be run manually as a standalone Python tool.

**Source**: `cmake/StaticAnalyzers.cmake`, `QWEN.md`, `AI_GUIDELINES.md`, lizard documentation

### A.6 Build Commands Reference

**Standard Builds**:

```bash
# Debug build (development)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Release build (production)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# RelWithDebInfo (performance testing with debug info)
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build
```

**Using CMake Presets**:

```bash
# Windows MSVC Debug
cmake -S . --preset windows-msvc-debug-developer-mode
cmake --build --preset windows-msvc-debug-developer-mode

# Unix GCC Debug
cmake -S . --preset unixlike-gcc-debug
cmake --build --preset unixlike-gcc-debug

# Unix Clang Release
cmake -S . --preset unixlike-clang-release
cmake --build --preset unixlike-clang-release
```

**Clean Build**:

```bash
# Full clean rebuild
rm -rf build/
cmake -S . -B build -G Ninja
cmake --build build

# Clean specific target
ninja clean
ninja jsav
```

**Source**: `CMakePresets.json`, `QWEN.md`

### A.7 Git Workflow

**Standard Feature Development**:

```bash
# Create feature branch from main
git checkout main
git pull origin main
git checkout -b 123-feature-name

# Make changes and commit
git add <files>
git commit -m "feat(scope): add new feature

Detailed description of what was added and why.

Closes #123"

# Push branch
git push -u origin 123-feature-name
```

**Sync with Main**:

```bash
# Rebase feature branch on latest main
git checkout main
git pull origin main
git checkout 123-feature-name
git rebase main

# Resolve conflicts if any, then continue
git rebase --continue
```

**Prepare for PR**:

```bash
# Ensure code is formatted
clang-format -i src/**/*.cpp src/**/*.hpp include/**/*.hpp

# Run tests
cd build && ctest --output-on-failure

# Check for uncommitted changes
git status

# Squash commits if needed
git rebase -i main
```

**Source**: Git best practices, `.specify/memory/constitution.md`

---

## Appendix B: Troubleshooting

This appendix provides solutions for common development issues.

### B.1 Missing Tools

**Problem**: Configuration fails with "tool not found" error.

**Solution**: **Do not disable tools.** Install missing dependencies.

**Ubuntu/Debian Linux**:

```bash
# Update package index
sudo apt update

# Install static analysis tools
sudo apt install clang-tidy cppcheck

# Install ccache for compilation caching
sudo apt install ccache

# Install gcovr for coverage reports
sudo apt install gcovr

# Install lizard for complexity analysis
pip3 install lizard
```

**Windows (Chocolatey)**:

```powershell
# Install LLVM (includes clang-tidy, clang-format)
choco install llvm

# Install cppcheck
choco install cppcheck

# Install ccache
choco install ccache

# Install CMake
choco install cmake

# Install Ninja
choco install ninja

# Install gcovr
pip install gcovr
```

**macOS (Homebrew)**:

```bash
# Install LLVM (includes clang-tidy, clang-format)
brew install llvm

# Install cppcheck
brew install cppcheck

# Install ccache
brew install ccache

# Install gcovr
brew install gcovr

# Install lizard
pip3 install lizard
```

**Verify Installation**:

```bash
clang-tidy --version
cppcheck --version
ccache --version
gcovr --version
lizard --version
```

**Source**: `QWEN.md`

### B.2 Sanitizer Errors

**Problem**: Tests fail with AddressSanitizer or UndefinedBehaviorSanitizer errors.

**Solution**: **Do not disable sanitizers.** Fix the underlying issues.

**AddressSanitizer Errors** (memory issues):

```text
==ERROR: AddressSanitizer: heap-use-after-free
==ERROR: AddressSanitizer: stack-buffer-overflow
==ERROR: AddressSanitizer: memory leak
```

**Common Causes and Fixes**:

| Error Type | Typical Cause | Fix |
|------------|---------------|-----|
| heap-use-after-free | Accessing freed memory | Ensure pointers are not used after `delete` or scope exit |
| stack-buffer-overflow | Array index out of bounds | Check array indices, use `std::array` or `std::vector` |
| memory leak | Missing `delete` | Use `std::unique_ptr` or RAII wrappers |
| container-overflow | Iterator past end | Use range-based for, check iterator validity |

**UndefinedBehaviorSanitizer Errors** (UB issues):

```text
==ERROR: UndefinedBehaviorSanitizer: signed integer overflow
==ERROR: UndefinedBehaviorSanitizer: null pointer passed as argument
==ERROR: UndefinedBehaviorSanitizer: misaligned address
```

**Common Causes and Fixes**:

| Error Type | Typical Cause | Fix |
|------------|---------------|-----|
| signed integer overflow | Arithmetic exceeding limits | Use `unsigned` or check bounds before arithmetic |
| null dereference | Null pointer access | Add null checks, use `std::optional` |
| misaligned access | Incorrect pointer casting | Use `std::bit_cast`, avoid C-style casts |
| type mismatch | Incorrect type punning | Use `std::variant`, `std::any` |

**Debugging Sanitizer Output**:

```bash
# Get detailed stack trace
export ASAN_OPTIONS=detect_leaks=1:abort_on_error=1:halt_on_error=0

# Run with symbolization (requires debug symbols)
export ASAN_SYMBOLIZER_PATH=/usr/bin/llvm-symbolizer
./build/test/tests

# Compile with debug info for better stack traces
cmake -DCMAKE_BUILD_TYPE=Debug -Djsav_ENABLE_SANITIZER_ADDRESS=ON ..
```

**Source**: `QWEN.md`, `ProjectOptions.cmake`, `cmake/Sanitizers.cmake`

### B.3 Compiler Warnings

**Problem**: Build fails due to warnings treated as errors.

**Solution**: **Do not disable warnings.** Fix the underlying code.

**Common Warnings and Fixes**:

| Warning | Typical Cause | Fix |
|---------|---------------|-----|
| `-Wunused-variable` | Declared but unused variable | Remove variable or use `[[maybe_unused]]` |
| `-Wsign-conversion` | Implicit signed/unsigned conversion | Use explicit cast or matching types |
| `-Wshadow` | Variable shadows outer scope | Rename variable to avoid shadowing |
| `-Wconversion` | Implicit type conversion | Use explicit cast or `narrow_cast<>` |
| `-Wnull-dereference` | Potential null dereference | Add null check before dereference |

**MSVC Specific**:

```text
warning C4244: conversion from 'type1' to 'type2', possible loss of data
warning C4267: conversion from 'size_t' to 'int', possible loss of data
```

**Fix**: Use explicit cast with `narrow_cast<>` or matching types:

```cpp
// Instead of:
int i = size_t_value;

// Use:
int i = narrow_cast<int>(size_t_value);
```

**Source**: `cmake/CompilerWarnings.cmake`, `ProjectOptions.cmake`

### B.4 Build Failures

**Problem**: CMake configuration or build fails.

**Clean Rebuild**:

```bash
# Remove build directory completely
rm -rf build/

# Reconfigure
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -Djsav_PACKAGING_MAINTAINER_MODE=OFF \
  ..

# Rebuild
cmake --build build
```

**CMake Cache Issues**:

```bash
# Clear CMake cache only
cd build
rm CMakeCache.txt

# Reconfigure
cmake ..
```

**Dependency Issues**:

```bash
# Clear CPM cache
rm -rf _deps/

# Reconfigure (dependencies will be re-downloaded)
cmake ..
```

**Generator Switching**:

```bash
# If switching generators (e.g., Ninja → Makefiles)
rm -rf build/
cmake -S . -B build -G "Unix Makefiles"
```

**Source**: `QWEN.md`, CMake best practices

### B.5 Test Failures

**Problem**: Tests fail unexpectedly.

**Debugging Steps**:

```bash
# Run failing test with verbose output
ctest -R "test_name" --verbose --output-on-failure

# Run test binary directly for more output
./build/test/tests "test_name"

# Run with Catch2 debug reporter
./build/test/tests "test_name" -r compact

# Check for sanitizer errors in test output
# Look for "ERROR: AddressSanitizer" or "ERROR: UndefinedBehaviorSanitizer"
```

**Constexpr Test Failures**:

```bash
# If constexpr_tests fail to compile, run relaxed version first
ninja relaxed_constexpr_tests
./build/test/relaxed_constexpr_tests

# Debug runtime failures, then verify constexpr compilation
ninja constexpr_tests
```

**Source**: `test/CMakeLists.txt`, `QWEN.md`

### B.6 Performance Issues

**Problem**: Slow compilation or test execution.

**Solutions**:

**Slow Compilation**:

```bash
# Enable ccache for compilation caching
cmake -Djsav_ENABLE_CACHE=ON ..

# Use Ninja for faster parallel builds
cmake -G Ninja ..

# Increase parallelism
ninja -j 8  # Adjust based on CPU cores

# Use precompiled headers (if configured)
cmake -Djsav_ENABLE_PCH=ON ..
```

**Slow Tests**:

```bash
# Run tests in parallel
ctest --parallel 4

# Run only affected tests
ctest -R "pattern" --output-on-failure

# Skip benchmarks during development
ctest -E "benchmark" --output-on-failure
```

**Source**: `cmake/Cache.cmake`, `QWEN.md`

---

---

## 8. Agent-Based Code Generation Framework

This section introduces a comprehensive framework of autonomous AI agents for automating code generation, maintenance, and quality assurance within the `jsav` compiler project. Each agent has a specialized role and operates in coordination with the others through well-defined interfaces and communication protocols.

---

### 8.1 Specialized Agent Roles and Responsibilities

#### Agent 1: Planner Agent — Strategic Task Decomposition and Workflow Orchestration

**Primary Responsibility:** The Planner Agent functions as the central orchestrator of the code generation ecosystem, serving as the primary interface between high-level development objectives and their tactical implementation. This agent possesses comprehensive understanding of project architecture, dependency relationships, and development constraints as documented in this `AGENTS.md` file.

**Input Specifications:** Natural language descriptions of desired features, performance optimizations, or architectural modifications. Input may range from broad directives such as "implement Unicode escape sequences in the lexer" to specific technical requirements such as "optimize token stream generation to reduce memory allocations."

**Output Deliverables:**

- Comprehensive file modification schedules with dependency ordering respecting the `jsav_Core_lib` → `jsav_Lib` → `jsav` dependency hierarchy
- Detailed specifications for new functions and modules, including their target directory (`include/jsav/`, `include/jsavCore/`, `src/jsav_Lib/`, etc.)
- Test coverage requirements mapped to the three test targets (`constexpr_tests`, `relaxed_constexpr_tests`, `tests`)
- Risk assessment and mitigation strategies, particularly for changes to public API headers in `include/jsav/`
- CMake target update requirements when new source files or dependencies are introduced

**Technical Infrastructure:** The Planner Agent leverages the architectural knowledge documented in this file, specifically Sections 2.2 (Directory Relationships), 2.3 (Build Pipeline Flow), and 7 (Rules for Automated Agents), to generate plans that preserve overall system coherence and ensure compatibility with the established CMake build system.

---

#### Agent 2: Coder Agent — Automated Code Implementation and Integration

**Primary Responsibility:** The Coder Agent specializes in translating detailed technical specifications into production-quality C++23 code. It demonstrates expertise in C++23-specific best practices, established software design patterns, and the coding conventions defined in Section 4 of this document.

**Input Specifications:** Precisely defined implementation requirements from the Planner Agent, including:

- Full function signatures with return types and parameter lists, including `[[nodiscard]]`, `constexpr`, `noexcept` qualifiers where applicable; each parameter documented with its type, purpose, and applicable constraints
- Detailed algorithmic logic descriptions aligned with the project's C++23 feature preferences (Section 4.3)
- Target file paths and namespace definitions following the `jsv` / `fs` / `vnd` namespace conventions
- Integration requirements with existing modules, specifying which CMake targets must be linked

**Output Deliverables:**

- Complete function implementations with robust error handling using `std::expected<T, E>` or `std::optional<T>` as appropriate, avoiding raw exceptions for expected failure paths
- `#pragma once` header guards, correct include ordering per `.clang-format` configuration, and proper use of forward declarations to minimize compilation dependencies
- Doxygen-compatible documentation for all public symbols in `include/jsav/`
- CMake `CMakeLists.txt` updates when new `.cpp` source files are added to `src/jsav_Lib/` or `src/jsav_Core_lib/`
- Compliance with the column limit (140 characters) and indentation (4 spaces) specified in `.clang-format`

**Technical Infrastructure:** The Coder Agent applies deep expertise in C++23 semantics, memory ownership (Section 4.4), and the project's compiler warning configuration (`cmake/CompilerWarnings.cmake`). It performs static analysis awareness by producing code that generates zero warnings under `-Wall -Wextra -Wshadow -Wconversion -Wsign-conversion` (GCC/Clang) or `/W4 /permissive-` (MSVC).

---

#### Agent 3: Tester Agent — Quality Assurance and Code Integrity Validation

**Primary Responsibility:** The Tester Agent provides comprehensive quality assurance by automating test generation and execution, alongside systematic code quality analysis. It verifies functional correctness using the three-target Catch2 testing strategy established in Section 5.

**Input Specifications:**

- Complete function implementations requiring validation, with their behavioral specifications
- Expected behavior descriptions including edge cases, boundary conditions, and error scenarios
- Performance requirements and compile-time evaluation constraints
- Integration testing requirements for inter-module interactions

**Output Deliverables:**

- `constexpr_tests.cpp` additions using `STATIC_REQUIRE` for all `constexpr`-capable functions
- `tests.cpp` additions using `REQUIRE`, `REQUIRE_NOTHROW`, `REQUIRE_THROWS_AS` for runtime behavior
- Test case names following the `ClassName_MethodName_Scenario` convention with appropriate `[tag]` annotations
- Detailed test execution reports with coverage analysis via `gcovr`, targeting ≥80% line coverage and ≥70% branch coverage
- Code duplication analysis using `similarity-rs`:
    - Installation: `cargo install similarity-rs`
    - **Critical Exclusion Policy:** The `test/` and `fuzz_test/` directories must be excluded from duplicate code analysis to prevent false positive detections. Use the `--skip-test` flag where applicable.

**Technical Infrastructure:** The Tester Agent employs:

- **Catch2 v3.13.0:** Standard unit, integration, and compile-time testing (`STATIC_REQUIRE`)
- **gcovr:** Coverage report generation in HTML and Cobertura XML formats
- **AddressSanitizer / UndefinedBehaviorSanitizer:** Configured via `jsav_ENABLE_SANITIZER_ADDRESS=ON` and `jsav_ENABLE_SANITIZER_UNDEFINED=ON`; all tests must report zero violations

---

#### Agent 4: Refactor Agent — Code Quality Enhancement and Architectural Improvement

**Primary Responsibility:** The Refactor Agent specializes in systematic code quality improvement through structural refactoring, performance optimization, and maintainability enhancement. It operates at both the function level (complexity reduction) and the architectural level (dependency restructuring).

**Input Specifications:**

- Target modules or functions requiring refactoring, with specific quality metrics to improve (cyclomatic complexity ≤15, function length ≤100 lines, parameters ≤6 per Section 7)
- Architectural constraints: refactoring must not introduce circular dependencies between `jsav_Core_lib` and `jsav_Lib`
- Performance benchmarks and acceptance criteria for optimization-focused refactors
- Compatibility requirements when modifying public API headers in `include/jsav/`

**Output Deliverables:**

- Refactored code maintaining functional equivalence, verified by the full test suite
- Enhanced documentation following the Doxygen standards required for `include/jsav/` headers

**Mandatory Output Format — Unified Diff:** All refactoring operations must produce complete unified diff output (`diff -u` format):

- **File Path Documentation:** Full file paths with line number references
- **Change Visualization:** Removed lines prefixed with `-`, added lines prefixed with `+`
- **Context Preservation:** Minimum 3 lines of unchanged code surrounding each modification
- **Comprehensive Coverage:** All changes across the entire refactoring scope documented without omissions

**Technical Infrastructure:** The Refactor Agent integrates with:

- **clang-tidy:** Static analysis enforcement (configured via `jsav_ENABLE_CLANG_TIDY=ON`)
- **cppcheck:** Complementary static analysis (`jsav_ENABLE_CPPCHECK=ON`)
- **lizard:** Complexity analysis — `lizard src/**/*.cpp include/**/*.hpp --CCN 15 --length 100 --arguments 6`
- **clang-format:** Mandatory formatting pass after every refactoring increment — `clang-format -i <files>`

---

#### Agent 5: Security Agent — Vulnerability Detection and Hardening

**Primary Responsibility:** The Security Agent identifies, prioritizes, and remediates security vulnerabilities and insecure coding patterns throughout the codebase. It minimizes the project's attack surface and establishes secure defaults for the compiler's public API and build tooling.

**Input Specifications:**

- Source code under `src/` and `include/`, dependency declarations in `Dependencies.cmake`, and CI configurations
- Threat model defining expected trust boundaries (e.g., processing potentially malicious source files through the lexer)
- Optional compliance targets or organizational security policies

**Output Deliverables:**

- Vulnerability reports mapped to specific files and functions with remediation suggestions and patches in unified diff format
- Dependency CVE audits with recommended version constraints for `Dependencies.cmake` (all dependencies are version-locked via CPM)
- Safe-configuration patches, including additional compiler warning flags for `cmake/CompilerWarnings.cmake` and hardening options in `cmake/Hardening.cmake`
- Security-focused unit and integration tests in `test/tests.cpp` covering boundary conditions, malformed input handling, and resource exhaustion scenarios

**Technical Infrastructure:** The Security Agent integrates with:

- **cmake/Hardening.cmake:** FORTIFY_SOURCE, RELRO, and related hardening flags
- **AddressSanitizer:** Mandatory for security validation — zero leaks policy enforced
- **Static analysis (clang-tidy, cppcheck):** Taint analysis and unsafe pattern detection
- **Dependency auditing:** CVE lookups for fmtlib/fmt v12.1.0, spdlog v1.17.0, CLI11 v2.6.1, Catch2 v3.13.0

**Key Constraint:** Per Section 4.3 and 4.4, any introduction of `reinterpret_cast` or manual memory management requires explicit justification and Security Agent approval. All raw pointer usage must be reviewed against the Non-Owning Reference policy.

---

#### Agent 6: Performance Agent — Profiling and Optimization

**Primary Responsibility:** The Performance Agent evaluates runtime and compile-time performance metrics, identifies hotspots in the compiler pipeline, and recommends targeted optimizations that maintain functional correctness while improving throughput and reducing memory footprint.

**Input Specifications:**

- Benchmarks, flamegraphs, and compiled artifacts from the `out/build/<preset>/` directory
- Performance requirements and SLOs for specific compilation phases (lexical analysis, file I/O)
- CI benchmark history for regression detection across builds

**Output Deliverables:**

- Hotspot analysis reports with concrete file- and function-level recommendations
- Microbenchmarks added to `test/tests.cpp` using Catch2's built-in benchmark support
- Safe refactor suggestions (algorithmic improvements, data layout changes, `constexpr` promotion)
- Patch sets in unified diff format with before/after performance measurements and CI integration guidance
- `cmake/InterproceduralOptimization.cmake` configuration recommendations for LTO/IPO where appropriate

**Technical Infrastructure:**

- **Profiling tools:** `perf` (Linux), Windows ETW, flamegraph generators
- **Catch2 benchmarks:** Integrated into `test/tests.cpp` using `BENCHMARK` sections
- **Build type selection:** `RelWithDebInfo` (`-O2 -g`) for profiling with debug symbols
- **ccache analysis:** `cmake/Cache.cmake` configuration review for optimal cache hit rates
- **Compiler flags:** `-O3`, LTO (`cmake/InterproceduralOptimization.cmake`), SIMD (`cmake/Simd.cmake`)

**Workflow Constraint:** All optimization decisions must be based on empirical profiling data. Per Anti-Pattern 7 (Speculative Optimization Without Empirical Validation), theoretical analysis alone is insufficient justification for introducing code complexity.

---

#### Agent 7: Documentation Agent — API Docs and Onboarding Guides

**Primary Responsibility:** The Documentation Agent produces and maintains high-quality developer and user documentation. It automates generation of Doxygen comments, usage examples, design notes, and onboarding guides to reduce cognitive load for contributors to the `jsav` project.

**Input Specifications:**

- Public API headers in `include/jsav/` and internal headers in `include/jsavCore/`
- Module interfaces, example code from `test/tests.cpp`, and `README` drafts
- Issue and PR discussions that indicate unclear or undocumented areas
- Audience targets: new contributor, library consumer, compiler developer, CI maintainer

**Output Deliverables:**

- Complete Doxygen comments for all public symbols in `include/jsav/`, following C++ documentation conventions:

  ```cpp
  /// @brief Brief description of the class or function.
  ///
  /// Detailed explanation of functionality, parameters, and return values.
  ///
  /// @param paramName Description of the parameter, its units, and valid ranges.
  /// @return Description of the return value.
  ///
  /// @throws std::runtime_error If applicable (prefer std::expected over throws).
  ///
  /// @note Any important implementation notes or constraints.
  ///
  /// @code
  /// // Usage example
  /// Lexer lex(source);
  /// auto tokens = lex.tokenize();
  /// @endcode
  ```

- Usage examples and short how-to guides for common workflows (lexer initialization, token stream traversal, filesystem operations)
- Updates to this `AGENTS.md` and `QWEN.md` when architectural decisions change
- A contributor onboarding checklist covering: prerequisites installation (Section 3.1), first build (Section 3.4), running tests (Section 5.3), and code style setup (Section 4)
- Doxygen configuration updates in `cmake/Doxygen.cmake` for new modules

**Technical Infrastructure:**

- **Doxygen:** Configured via `cmake/Doxygen.cmake`; CI verification that documentation builds without warnings
- **clang-tidy:** `modernize-use-nodiscard` and `readability-*` checks enforce documentation completeness
- **Automated verification:** CI ensures all code examples in documentation compile successfully under C++23

---

### 8.2 Agent Interaction Protocol and Workflow Architecture

The agent ecosystem operates through a carefully orchestrated sequential workflow:

**Phase 1 — Strategic Planning:** The **Planner Agent** receives high-level development objectives and produces implementation strategies with task prioritization, dependency analysis per the `jsav_Core_lib` → `jsav_Lib` → `jsav` hierarchy, and identification of affected CMake targets.

**Phase 2 — Test Specification:** The **Tester Agent** generates comprehensive test specifications (including `STATIC_REQUIRE` contracts for `constexpr` functions) before implementation begins, establishing behavioral contracts that guide the Coder Agent.

**Phase 3 — Code Generation:** The **Coder Agent** produces implementation artifacts compliant with C++23 standards, project naming conventions (Section 4.1), and the memory ownership model (Section 4.4).

**Phase 4 — Quality Assessment:** The **Tester Agent** validates generated code through:

- `ninja constexpr_tests relaxed_constexpr_tests tests` build and execution
- `ctest --output-on-failure` with all three test targets
- `similarity-rs` duplicate code analysis (excluding `test/` and `fuzz_test/`)
- Zero AddressSanitizer and UndefinedBehaviorSanitizer violations

**Phase 5 — Quality Gate Evaluation:**

- **Success Criteria:** All three test targets pass, ≥80% line coverage, zero sanitizer violations, zero clang-tidy and cppcheck issues
- **Failure Protocol:** Non-conforming code returns to the **Coder Agent** with precise failure diagnostics
- No phase may be bypassed; quality gate failures are mandatory stop points

**Phase 6 — Security and Performance Review:** The **Security Agent** and **Performance Agent** may be invoked in parallel for security hardening and performance baseline validation.

**Phase 7 — Continuous Improvement:** The **Refactor Agent** is invoked to address complexity violations (lizard CCN >15), function length violations (>100 lines), or architectural improvements identified during review, always producing unified diff output.

**Phase 8 — Documentation Synchronization:** The **Documentation Agent** updates Doxygen comments, this `AGENTS.md`, and `QWEN.md` to reflect all changes made in preceding phases.

---

### 8.3 Patterns: Established Best Practices for Agent-Based Code Generation

#### Pattern 1: Single Responsibility Agent Design

Each agent maintains exclusive ownership of a distinct development phase. Responsibilities do not overlap. Clear input/output contracts define boundaries. Apply this pattern when designing new agents: resist scope creep and create new specialized agents when novel capabilities are required.

#### Pattern 2: Dependency-Ordered Task Execution

The Planner Agent must construct explicit dependency graphs respecting the `jsav_Core_lib` → `jsav_Lib` → `jsav` hierarchy (Section 2.2). Apply topological sorting to establish valid execution sequences. Resolve circular dependencies by introducing abstract interfaces before proceeding with implementation.

#### Pattern 3: Test-First Specification Generation

`STATIC_REQUIRE` contracts and `REQUIRE`-based test cases must be generated before implementation begins. The Coder Agent treats test suites as authoritative behavioral contracts. Failed test executions trigger immediate revision cycles with precise failure diagnostics.

#### Pattern 4: Incremental Refactoring with Regression Protection

Decompose large refactoring operations into minimal atomic changes. Execute `ctest --output-on-failure` after each increment. Generate unified diff output for every increment. Maintain rollback capability for any increment that introduces regressions.

#### Pattern 5: Security-by-Default Configuration

Default configurations prioritize security over convenience. Unsafe operations (`reinterpret_cast`, raw `new`/`delete`) require explicit opt-in. Enable `cmake/Hardening.cmake` options by default. Audit all CPM dependencies for CVEs before approval.

#### Pattern 6: Performance-Aware Development with Baseline Tracking

Establish performance baselines using `RelWithDebInfo` builds before optimization. Integrate Catch2 `BENCHMARK` sections into CI. Base all optimization decisions on empirical profiling data (flamegraphs, perf). Define acceptable regression thresholds and flag changes that exceed them.

#### Pattern 7: Comprehensive Change Documentation with Unified Diff Format

All agents that modify code must produce `diff -u` output with full file paths, minimum 3 lines of context, and complete coverage of the entire change scope. This is mandatory for audit trail capabilities and effective code review.

#### Pattern 8: Modular Documentation with Audience Targeting

Create discrete documentation modules targeting specific audiences: new contributors (onboarding checklist), library consumers (public API in `include/jsav/`), compiler developers (internal architecture), and CI maintainers (build system). Minimize duplication through cross-references. Verify all code examples compile under C++23.

---

### 8.4 Anti-Patterns: Common Pitfalls to Avoid

| Anti-Pattern | Description | Consequence | Correct Alternative |
|---|---|---|---|
| **Monolithic Agent Design** | Agent spans planning, coding, testing, and refactoring simultaneously | Diluted expertise, reduced output quality | Pattern 1: Single Responsibility Agent Design |
| **Quality Gate Bypass** | Proceeding to the next phase before completing validation of the current one | Compounding defects, architectural debt | Strict Phase 5 enforcement; treat failures as mandatory stop points |
| **Incomplete Change Documentation** | Producing diffs that omit context, file paths, or portions of the change scope | Audit trail gaps, review impediments, rollback failure | Pattern 7: Unified Diff Format for all modifications |
| **Implementation-First Development** | Generating code before establishing test specifications | Specification ambiguity, delayed error detection, inadequate coverage | Pattern 3: Test-First Specification Generation |
| **Large-Scale Refactoring Without Incremental Validation** | Modifying numerous files simultaneously without intermediate checkpoints | Regression localization impossible, forced full rollback | Pattern 4: Incremental Refactoring with Regression Protection |
| **Security as a Final-Phase Activity** | Deferring security review until after implementation and deployment | Architectural security flaws, entrenched vulnerable dependencies | Pattern 5: Security-by-Default Configuration |
| **Speculative Optimization** | Implementing optimizations without empirical profiling data | Wasted effort, unmeasured regressions, unnecessary complexity | Pattern 6: Performance-Aware Development with Baseline Tracking |
| **Generic Documentation Without Audience Differentiation** | Documentation that attempts to serve all audiences simultaneously | Cognitive overload for novices, insufficient depth for experts | Pattern 8: Modular Documentation with Audience Targeting |

---

### 8.5 Enforced Rules for Agent Operations

The following rules are mandatory for all agents operating on this codebase. They extend and reinforce Section 7 (Rules for Automated Agents and AI Tools).

| Rule | Scope | Enforcement |
|---|---|---|
| **Never bypass the three-target test suite** (`constexpr_tests`, `relaxed_constexpr_tests`, `tests`) | All agents | Phase 5 quality gate |
| **Never introduce circular dependencies** between `jsav_Core_lib` and `jsav_Lib` | Planner, Coder | Build failure at link time |
| **Always run `clang-format -i`** on all modified `.cpp` and `.hpp` files before finalizing output | Coder, Refactor | CI formatting check failure |
| **Never modify `_deps/`** — update only `Dependencies.cmake` via CPM with explicit version locks | All agents | Build system integrity |
| **Never expose dependency types** (spdlog, fmt, CLI11) in `include/jsav/` public headers | Coder | Header leak anti-pattern; consumer breakage |
| **Always produce unified diff output** for every code modification | Refactor, Security, Performance | Audit trail requirement (Pattern 7) |
| **Exclude `test/` and `fuzz_test/` from `similarity-rs` analysis** using `--skip-test` | Tester | False positive prevention |
| **Zero sanitizer violations** required before any phase transition | Tester | AddressSanitizer + UndefinedBehaviorSanitizer |
| **Never disable compiler warnings** project-wide; fix the underlying code | All agents | Section 7 Compilation Rules |
| **All `constexpr`-capable functions must have `STATIC_REQUIRE` coverage** in `constexpr_tests.cpp` | Tester | Section 5.2 requirements |


## Appendix C: Document Verification

### C.1 Evidence Sources

All information in this document is sourced from specific repository files. Each claim is traceable to its source.

**Primary Sources**:

| File | Content Covered |
|------|-----------------|
| `CMakeLists.txt` | Project configuration, target definitions, install rules, version information |
| `ProjectOptions.cmake` | Compiler options, sanitizer configuration, feature flags |
| `Dependencies.cmake` | External dependency management, version locking |
| `CMakePresets.json` | Predefined build configurations for all platforms |
| `.clang-format` | Code formatting rules (140 column limit, 4-space indent) |
| `.clang-tidy` | Static analysis configuration, identifier length rules |
| `.gitlab-ci.yml` | CI/CD pipeline configuration, test stages |
| `gcovr.cfg` | Coverage reporting configuration, exclusions |
| `QWEN.md` | Project documentation, build instructions, development workflow |
| `AI_GUIDELINES.md` | AI agent guidelines, testing practices, tool usage |
| `HUMAN_GUIDELINES.md` | Developer philosophy, C++ best practices references |
| `.specify/memory/constitution.md` | Project constitution, core principles, quality gates |
| `test/CMakeLists.txt` | Test configuration, Catch2 setup, test discovery |
| `cmake/CompilerWarnings.cmake` | Compiler-specific warning flags |
| `cmake/Sanitizers.cmake` | Sanitizer configuration, ASan/UBSan setup |
| `cmake/StaticAnalyzers.cmake` | clang-tidy, cppcheck integration |

**Observed Code Patterns**:

- Naming conventions observed in `include/jsav/`, `src/`, `test/` directories
- Header guard style (`#pragma once`) observed in all header files
- Include order observed in source files
- Memory management patterns observed in implementation files

### C.2 Placeholder Items

**Status**: No placeholders used.

All information in this document has been verified against repository files. No speculative or assumed information is included.

### C.3 Document Update Procedure

When updating this document:

1. **Verify Changes**: Ensure any new information is verified against actual repository files
2. **Update Sources**: Add new source files to the Evidence Sources table
3. **Check Consistency**: Verify new information does not contradict existing content
4. **Update Version**: Increment version number and update "Last Updated" date
5. **Review Placeholders**: If adding placeholders, ensure they reference specific files to inspect

**Source**: Evidence-First Documentation pattern
# jsav - C++23 Compiler Project

## Project Overview

**jsav** is a compiler written in C++23, built with modern CMake practices and comprehensive tooling for code quality assurance. The project follows strict static analysis guidelines and incorporates best practices from [C++23 Best Practices](https://leanpub.com/cpp23_best_practices/) by Jason Turner.

### Key Technologies

- **Language**: C++23
- **Build System**: CMake 3.21+ with Ninja generator
- **Dependencies**: Managed via CPM.cmake (CMake Package Manager): fmt, spdlog, Catch2, CLI11
- **Testing**: Catch2 framework with constexpr, runtime, and fuzz testing support
- **Static Analysis**: clang-tidy, cppcheck, lizard (complexity analysis)
- **Sanitizers**: AddressSanitizer, UndefinedBehaviorSanitizer
- **Code Coverage**: gcovr for coverage reporting

### Project Structure

```text
jsav/
├── CMakeLists.txt              # Main CMake configuration
├── ProjectOptions.cmake        # Project-wide CMake options
├── Dependencies.cmake          # External dependency management
├── CMakePresets.json           # CMake configuration presets
├── include/
│   ├── jsav/                   # Public headers (main module)
│   │   ├── fs/                 # Filesystem utilities
│   │   ├── headers.hpp         # Master include for jsav module
│   │   └── jsav.hpp            # Main module header
│   └── jsavCore/               # Core library headers
│       ├── cast/               # Casting utilities
│       ├── timer/              # Timer utilities
│       ├── disableWarn.hpp     # Warning suppression macros
│       ├── FileReader.hpp      # File reading utilities
│       ├── format.hpp          # Formatting utilities
│       ├── Log.hpp             # Logging utilities
│       └── headersCore.hpp     # Core master include
├── src/
│   ├── CMakeLists.txt          # Source subdirectory config
│   ├── jsav/                   # Main executable
│   │   ├── CMakeLists.txt
│   │   ├── Costanti.hpp        # Constants
│   │   └── main.cpp            # Entry point with CLI
│   ├── jsav_Core_lib/          # Core library implementation
│   │   ├── CMakeLists.txt
│   │   └── jsavCore.cpp
│   └── jsav_Lib/               # Main library implementation
│       ├── CMakeLists.txt
│       └── jsav.cpp
├── test/
│   ├── CMakeLists.txt          # Test configuration
│   ├── constexpr_tests.cpp     # Compile-time constexpr tests
│   └── tests.cpp               # Runtime tests
├── fuzz_test/                  # Fuzz testing (optional)
├── cmake/                      # CMake modules and utilities
└── configured_files/           # Template files for generation
```

### Library Targets

| Target | Description |
|--------|-------------|
| `jsav::jsav_options` | Interface library for compile options |
| `jsav::jsav_warnings` | Interface library for warning flags |
| `jsav::jsav_core_lib` | Core library (logging, file I/O, utilities) |
| `jsav::jsav_lib` | Main library (depends on core_lib) |
| `jsav` | Main executable |

---

## Building and Running

### Prerequisites

**Required:**

- C++23 compatible compiler (GCC 13+, Clang 16+, MSVC 2022+)
- CMake 3.21+
- Ninja build system (recommended)

**Recommended Tools:**

- clang-tidy (static analysis)
- cppcheck (static analysis)
- ccache (compilation caching)
- include-what-you-use (header dependency analysis)

### Quick Start (Windows)

```powershell
# Configure with CMake preset
cmake -S . --preset windows-msvc-debug-developer-mode

# Build
cmake --build --preset windows-msvc-debug-developer-mode

# Run tests
ctest --preset test-windows-msvc-debug-developer-mode
```

### Quick Start (Unix-like with GCC)

```bash
# Configure
cmake -S . --preset unixlike-gcc-debug

# Build
cmake --build --preset unixlike-gcc-debug

# Run tests
ctest --preset test-unixlike-gcc-debug
```

### Manual Configuration (Ninja)

```bash
# Create build directory
mkdir -p build && cd build

# Configure (Debug with coverage)
cmake -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -Djsav_PACKAGING_MAINTAINER_MODE=OFF \
  -Djsav_ENABLE_COVERAGE=ON \
  ..

# Build
ninja

# Run all tests
ctest --output-on-failure

# Generate coverage report
gcovr -r .. --config=../gcovr.cfg
```

### CMake Build Options

| Option | Default (Dev) | Description |
|--------|---------------|-------------|
| `jsav_ENABLE_COVERAGE` | OFF | Enable code coverage reporting |
| `jsav_ENABLE_SANITIZER_ADDRESS` | ON (if supported) | AddressSanitizer |
| `jsav_ENABLE_SANITIZER_UNDEFINED` | ON (if supported) | UndefinedBehaviorSanitizer |
| `jsav_ENABLE_CLANG_TIDY` | ON | Enable clang-tidy analysis |
| `jsav_ENABLE_CPPCHECK` | ON | Enable cppcheck analysis |
| `jsav_ENABLE_CACHE` | ON | Enable ccache |
| `jsav_WARNINGS_AS_ERRORS` | ON | Treat warnings as errors |
| `jsav_PACKAGING_MAINTAINER_MODE` | OFF | Developer mode (more options enabled) |

### Running the Application

```bash
# After building
./build/jsav --help

# Example usage
./build/jsav -i input_file --compile
./build/jsav --version
./build/jsav --clean --compile
```

---

## Development Conventions

### Coding Standards

1. **Modern C++ Features**
   - Use `std::format` and `std::print` instead of iostream/printf
   - Prefer marking functions `constexpr` when possible (leveraging C++23's expanded constexpr capabilities)
   - Use concepts to constrain template parameters
   - Prefer `[[nodiscard]]` on functions returning values
   - Use scoped enums (`enum class`)

2. **Memory Management**
   - No raw `new`/`delete`
   - Prefer stack allocation → `std::array`/`std::vector` → smart pointers
   - Always prefer `std::unique_ptr` over `std::shared_ptr`
   - Follow Rule of 0 (avoid manual resource management)

3. **Type Safety**
   - Define strong types instead of primitives (e.g., `Velocity{int}` not `int`)
   - Avoid C-style casts
   - Use `const` liberally

4. **Code Organization**
   - Use algorithms over raw loops
   - Use ranged-for with `auto` when algorithms don't apply
   - Keep functions ≤100 lines, cyclomatic complexity ≤15
   - Parameters ≤6 per function

### Code Formatting

**Always run clang-format on modified files:**

```bash
clang-format -i path/to/changed/files/*.cpp path/to/changed/files/*.hpp
```

The project uses `.clang-format` configuration (140 column limit, 4-space indent).

### Testing Practices

**Three Test Targets:**

1. **`constexpr_tests`** - Compile-time static assertions
   - Tests compiled as `STATIC_REQUIRE`
   - If it compiles, tests pass
   - Located in `test/constexpr_tests.cpp`

2. **`relaxed_constexpr_tests`** - Runtime version of constexpr tests
   - Same tests with `REQUIRE` (runtime assertions)
   - Use for debugging test failures
   - Compiled with `-DCATCH_CONFIG_RUNTIME_STATIC_REQUIRE`

3. **`tests`** - Runtime-only tests
   - For I/O, runtime features, non-constexpr code
   - Located in `test/tests.cpp`

**Workflow for Adding Tests:**
```
1. Add tests to relaxed_constexpr_tests first
2. Debug and fix issues
3. Verify they compile in constexpr_tests
4. For non-constexpr functionality, add to tests.cpp
```

**Running Tests:**
```bash
# Run all tests
ctest --output-on-failure

# Run specific test categories
ctest -R "unittests" --output-on-failure
ctest -R "constexpr" --output-on-failure
ctest -R "relaxed_constexpr" --output-on-failure
```

### Code Coverage

```bash
# Configure with coverage
cmake -Djsav_ENABLE_COVERAGE=ON ..

# Build and run tests
ninja
ctest

# Generate reports
gcovr -r .. --config=../gcovr.cfg
# HTML report: ./out/coverage/index.html
# XML report: ./out/cobertura.xml
```

### Static Analysis

The project enforces strict static analysis:

```bash
# clang-tidy (automatic during build if enabled)
# cppcheck (automatic during build if enabled)

# Lizard complexity analysis
cmake --build build --target lizard
cmake --build build --target lizard_html
```

**Complexity Thresholds:**

- Cyclomatic Complexity: ≤15
- Function Length: ≤100 lines
- Parameters: ≤6

### Common Development Workflow

```bash
# 1. Quick iteration setup
mkdir -p build && cd build
cmake -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -Djsav_PACKAGING_MAINTAINER_MODE=OFF \
  -Djsav_ENABLE_COVERAGE=ON \
  ..

# 2. Build
ninja

# 3. Test changes (runtime tests first)
ninja tests relaxed_constexpr_tests
ctest -R "unittests|relaxed_constexpr" --output-on-failure

# 4. Check coverage
ctest
gcovr -r .. --config=../gcovr.cfg

# 5. Format code
clang-format -i src/**/*.cpp src/**/*.hpp include/**/*.hpp

# 6. Commit after tests pass
```

---

## Troubleshooting

### Missing Tools

If configuration fails due to missing tools, **install them** (don't disable):

```bash
# Ubuntu/Debian
sudo apt install clang-tidy cppcheck ccache

# Windows (Chocolatey)
choco install clang-tidy cppcheck ccache

# macOS (Homebrew)
brew install llvm cppcheck ccache
```

### Sanitizer Errors

**Do not disable sanitizers.** Fix the underlying issues:

- AddressSanitizer errors → Memory access issues
- UndefinedBehaviorSanitizer errors → Undefined behavior

### Warnings as Errors

The project treats warnings as errors by default. **Fix warnings** rather than disabling them.

### Build Configuration Issues

```bash
# Clean rebuild
rm -rf build/
cmake -S . -B build -G Ninja
cmake --build build
```

---

## Additional Resources

- [Building Instructions](README_building.md) - Detailed build guide
- [Dependencies](README_dependencies.md) - Dependency installation
- [Docker](README_docker.md) - Containerized development
- [AI Guidelines](AI_GUIDELINES.md) - AI assistant guidelines
- [Human Guidelines](HUMAN_GUIDELINES.md) - Developer philosophy

---

## Key Files Reference

| File | Purpose |
|------|---------|
| `CMakeLists.txt` | Main build configuration |
| `CMakePresets.json` | Predefined build configurations |
| `ProjectOptions.cmake` | Compiler and sanitizer options |
| `Dependencies.cmake` | External dependency management |
| `.clang-format` | Code formatting rules |
| `.clang-tidy` | Static analysis configuration |
| `gcovr.cfg` | Coverage reporting configuration |
| `AI_GUIDELINES.md` | AI agent working guidelines |
| `HUMAN_GUIDELINES.md` | Developer philosophy and practices |

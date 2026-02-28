# CMake API Contract: UTF-8 Support

**Generated**: 2026-02-27  
**Feature**: 001-cmake-utf8-support  
**Type**: Build System Public API

---

## Overview

This document defines the public CMake API contract for UTF-8 support. Projects using the jsav build system can rely on these functions and options for UTF-8 configuration.

---

## 1. CMake Option Contract

### `jsav_ENABLE_UTF8`

**Signature**:

```cmake
option(jsav_ENABLE_UTF8 "Enable UTF-8 compiler flags and console support" ON)
```

**Contract**:

- **Type**: Boolean option
- **Default**: `ON`
- **Scope**: Global (affects all targets in project)
- **Behavior when ON**:

    - Compiler flags for UTF-8 are applied to all targets
    - Runtime header `jsav/utf8_console.hpp` is generated
    - BOM checking is performed on source files
- **Behavior when OFF**:

    - No UTF-8 configuration is applied
    - No runtime header is generated
    - BOM checking is skipped

**Usage Example**:

```cmake
# In CMakeLists.txt or preset
-Djsav_ENABLE_UTF8=ON  # Explicit enable
-Djsav_ENABLE_UTF8=OFF # Explicit disable
```

**Stability Guarantee**: This option name and behavior will not change without a major version bump.

---

## 2. CMake Function Contracts

### 2.1 `jsav_enable_utf8_compiler_flags()`

**Purpose**: Apply UTF-8 compiler flags to a target

**Signature**:

```cmake
function(jsav_enable_utf8_compiler_flags TARGET)
```

**Parameters**:

- `TARGET` (required): CMake target name to apply flags to

**Contract**:

- **MSVC**: Adds `/utf-8` to target compile options
- **GCC**: Adds `-finput-charset=UTF-8 -fexec-charset=UTF-8` to target compile options
- **Clang**: Adds `-finput-charset=UTF-8 -fexec-charset=UTF-8` to target compile options
- **Other compilers**: No-op (function returns without error)

**Side Effects**:

- Sets `JSAV_UTF8_COMPILER_FLAGS` cache variable with applied flags

**Example**:

```cmake
include(cmake/Utf8Compiler.cmake)
jsav_enable_utf8_compiler_flags(jsav)
jsav_enable_utf8_compiler_flags(jsav_lib)
```

**Error Conditions**:

- None - function silently succeeds even if compiler doesn't support flags

---

### 2.2 `jsav_enable_utf8_console()`

**Purpose**: Configure and install UTF-8 console runtime header

**Signature**:

```cmake
function(jsav_enable_utf8_console)
```

**Parameters**: None

**Contract**:

- Configures `utf8_console.hpp` from `utf8_console.hpp.in` template
- Adds `configured_files/include` to include directories for all targets
- Generated header is available as `<jsav/utf8_console.hpp>`

**Side Effects**:

- Sets `JSAV_UTF8_CONSOLE_HEADER` cache variable with generated header path
- Creates `build/configured_files/include/jsav/utf8_console.hpp`

**Example**:

```cmake
include(cmake/Utf8Console.cmake)
jsav_enable_utf8_console()
# Now can #include <jsav/utf8_console.hpp> in source
```

**Error Conditions**:

- FATAL_ERROR if template file `utf8_console.hpp.in` not found

---

### 2.3 `jsav_check_utf8_bom()`

**Purpose**: Check source files for UTF-8 BOM and strip if present

**Signature**:

```cmake
function(jsav_check_utf8_bom FILES)
```

**Parameters**:

- `FILES` (required): List of source file paths to check

**Contract**:

- Reads first 3 bytes of each file
- If bytes are `EF BB BF` (UTF-8 BOM), logs warning
- Does NOT modify source files (detection only)
- Returns list of files with BOM detected

**Side Effects**:

- Logs WARNING for each file with BOM
- Sets `JSAV_UTF8_BOM_DETECTED` local variable to TRUE if any BOM found

**Example**:

```cmake
include(cmake/Utf8BomStrip.cmake)
file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)
jsav_check_utf8_bom("${SOURCE_FILES}")
```

**Error Conditions**:

- None - inaccessible files are skipped with warning

---

## 3. Generated Header Contract

### `<jsav/utf8_console.hpp>`

**Purpose**: Runtime UTF-8 console initialization

**Include Path**: `#include <jsav/utf8_console.hpp>`

**Contract**:

- **Namespace**: `jsav::utf8`
- **Function**: `init_console()` - no parameters, no return value
- **Behavior on Windows**:
    - Calls `SetConsoleOutputCP(CP_UTF8)`
    - On failure: prints error to stderr and calls `std::exit(EXIT_FAILURE)`
- **Behavior on Linux/macOS**:
    - No-op (function body is empty)
    - Returns immediately
- **Dependencies**:
    - `<windows.h>` (Windows only)
    - `<fmt/core.h>` (for error messages)
    - `<cstdlib>` (for `std::exit`)

**Usage Contract**:

```cpp
#include <jsav/utf8_console.hpp>

int main() {
    jsav::utf8::init_console();  // MUST call unconditionally
    // ... rest of application
}
```

**Stability Guarantee**:

- Function signature will not change
- Namespace will not change
- Fail-fast behavior on Windows will not change

---

## 4. Test Tag Contract

### `[utf8]` Test Tag

**Purpose**: Enable selective execution of UTF-8 related tests

**Contract**:

- All UTF-8 tests in `tests.cpp` are tagged with `[utf8]`
- Tests can be run selectively: `ctest -R "\[utf8\]"`
- Tests can be excluded: `ctest -E "\[utf8\]"`

**Example**:

```bash
# Run only UTF-8 tests
ctest -R "\[utf8\]" --output-on-failure

# Run all tests except UTF-8
ctest -E "\[utf8\]"
```

**Test Coverage**:

- String literal byte preservation
- BOM absence verification
- Console initializer behavior
- Locale independence
- Diagnostic logging

---

## 5. Version Compatibility

| jsav Version | CMake Minimum | Compiler Minimum               | Contract Version |
|--------------|---------------|--------------------------------|------------------|
| 0.0.2+       | 3.29          | GCC 13+, Clang 18+, MSVC 2022+ | 1.0              |

**Backward Compatibility**:

- Contracts are stable within major versions (0.x.x)
- Breaking changes require major version bump (1.0.0)
- Deprecation warnings issued one minor version before removal

---

## 6. Error Handling Contract

### CMake-Side Errors

| Error             | Condition                           | Severity    | Message                                                                                         |
|-------------------|-------------------------------------|-------------|-------------------------------------------------------------------------------------------------|
| Template not found | `utf8_console.hpp.in` missing      | FATAL_ERROR | "UTF-8 header template not found at configured_files/include/jsav/utf8_console.hpp.in"          |
| Unsupported compiler | Compiler doesn't support UTF-8 flags | WARNING   | "Compiler does not support UTF-8 flags. UTF-8 support disabled."                                |
| File inaccessible | BOM check can't read file           | WARNING     | "Cannot read file {path} for BOM check, skipping"                                               |

### Runtime Errors

| Error                      | Condition                         | Behavior                | Message                                                                        |
|----------------------------|-----------------------------------|-------------------------|--------------------------------------------------------------------------------|
| Console init failure (Windows) | `SetConsoleOutputCP` returns 0 | `std::exit(EXIT_FAILURE)` | "ERROR: UTF-8 console initialization failed. Terminal does not support UTF-8 output." |

---

## 7. Integration Contract

### Integration with `ProjectOptions.cmake`

**Contract**: UTF-8 support is integrated into existing option macros:

```cmake
# In ProjectOptions.cmake
macro(jsav_local_options)
    # ... other options ...
    
    if(jsav_ENABLE_UTF8)
        include(cmake/Utf8Compiler.cmake)
        include(cmake/Utf8Console.cmake)
        include(cmake/Utf8BomStrip.cmake)
        
        jsav_enable_utf8_compiler_flags(jsav)
        jsav_enable_utf8_console()
        # BOM check happens at configure time
    endif()
endmacro()
```

**Contract Guarantee**:

- UTF-8 setup occurs AFTER sanitizer setup
- UTF-8 setup occurs BEFORE target compilation
- All targets linked to `jsav_options` automatically get UTF-8 flags

---

## 8. Preset Compatibility

**Contract**: All 8 existing CMakePresets.json presets automatically support UTF-8:

| Preset                              | UTF-8 Support    |
|-------------------------------------|------------------|
| `windows-msvc-debug-developer-mode` | ✅ Automatic     |
| `windows-msvc-release`              | ✅ Automatic     |
| `unixlike-gcc-debug`                | ✅ Automatic     |
| `unixlike-gcc-release`              | ✅ Automatic     |
| `unixlike-clang-debug`              | ✅ Automatic     |
| `unixlike-clang-release`            | ✅ Automatic     |
| All other presets                   | ✅ Automatic     |

**No Changes Required**: Existing presets work without modification.

---

## 9. Compliance Requirements

**For jsav Project Maintainers**:

- All CMake modules must implement the function signatures above
- Generated header must match the API contract
- Tests must cover all contract behaviors
- Changes to contracts require major version bump

**For Application Developers**:

- Call `jsav::utf8::init_console()` unconditionally in `main()`
- Ensure all source files are UTF-8 encoded (with or without BOM)
- Use modern terminal emulators that support UTF-8

---

## 10. Deprecation Policy

**Contract Changes**:

1. **Minor changes** (new functions, expanded behavior): Added in minor releases
2. **Deprecation**: Old API marked deprecated with warning, new API provided
3. **Removal**: Deprecated API removed after one minor version cycle
4. **Breaking changes**: Require major version bump

**Example**:

```cmake
# Version 0.1.0 - New function added
function(jsav_enable_utf8_compiler_flags_v2 TARGET)
    message(DEPRECATION "Use jsav_enable_utf8_compiler_flags() instead")
    jsav_enable_utf8_compiler_flags(${TARGET})
endfunction()
```

# Data Model: CMake UTF-8 Support

**Generated**: 2026-02-27  
**Feature**: 001-cmake-utf8-support  
**Type**: Build System Configuration + Runtime Header

---

## Overview

This feature introduces UTF-8 support through CMake configuration objects and a generated runtime header. There are no traditional data entities - instead, the "data model" consists of:

1. **CMake Configuration State** - Build-time options and detected capabilities
2. **Runtime Header Interface** - API exposed to applications
3. **Test Data** - UTF-8 test strings and expected behaviors

---

## 1. CMake Configuration State

### 1.1 Primary Option: `jsav_ENABLE_UTF8`

| Property         | Value                                         |
|------------------|-----------------------------------------------|
| **Type**         | CMake `option()`                              |
| **Default**      | `ON`                                          |
| **Scope**        | Global (affects all targets)                  |
| **Purpose**      | Enable/disable UTF-8 support throughout build |
| **Dependencies** | None (standalone feature)                     |

**Usage**:

```cmake
option(jsav_ENABLE_UTF8 "Enable UTF-8 compiler flags and console support" ON)
```

### 1.2 Internal State Variables

These variables are set by UTF-8 CMake modules and used internally:

| Variable                 | Type     | Scope  | Description                                                                                   |
|--------------------------|----------|--------|-----------------------------------------------------------------------------------------------|
| `JSAV_UTF8_COMPILER_FLAGS` | `string` | Cache  | Compiler flags for UTF-8 support (e.g., `/utf-8` or `-finput-charset=UTF-8 -fexec-charset=UTF-8`) |
| `JSAV_UTF8_CONSOLE_HEADER` | `string` | Cache  | Path to generated `utf8_console.hpp`                                                          |
| `JSAV_UTF8_BOM_DETECTED`   | `bool`   | Local  | Temporary flag during BOM check                                                               |

---

## 2. Runtime Header Interface

### 2.1 Namespace: `jsav::utf8`

**Purpose**: Provide initialization function for UTF-8 console output

**API**:

```cpp
namespace jsav::utf8 {
    /// @brief Initialize console for UTF-8 output
    /// @note On Windows, calls SetConsoleOutputCP(CP_UTF8)
    /// @note On Linux/macOS, this is a no-op (UTF-8 is default)
    /// @throws std::runtime_error on Windows if SetConsoleOutputCP fails
    /// @exit Calls std::exit(EXIT_FAILURE) on initialization failure
    void init_console();
}
```

**Generated Header Structure** (`configured_files/include/jsav/utf8_console.hpp.in`):

```cpp
#ifndef JSAV_UTF8_CONSOLE_HPP
#define JSAV_UTF8_CONSOLE_HPP

#include <cstdlib>      // std::exit, EXIT_FAILURE
#ifdef _WIN32
    #include <windows.h>    // SetConsoleOutputCP, CP_UTF8
#endif
#include <fmt/core.h>   // fmt::print (already pinned at 12.1.0)

namespace jsav::utf8 {
    inline void init_console() {
#ifdef _WIN32
        if (!SetConsoleOutputCP(CP_UTF8)) {
            fmt::print(stderr, 
                "ERROR: UTF-8 console initialization failed.\n"
                "       Terminal does not support UTF-8 output.\n"
                "       Ensure you are using Windows Terminal or a compatible terminal emulator.\n");
            std::exit(EXIT_FAILURE);
        }
#endif
        // Linux/macOS: No-op, UTF-8 is default locale
        // Per spec: Non-UTF-8 locales on Linux/macOS are unsupported
    }
}

#endif // JSAV_UTF8_CONSOLE_HPP
```

**Usage Pattern** (in application `main.cpp`):

```cpp
#include <jsav/utf8_console.hpp>

int main() {
    jsav::utf8::init_console();  // Must call unconditionally
    
    // Application code can now safely output UTF-8
    fmt::print("Hello, 世界！🌍\n");
    
    return 0;
}
```

---

## 3. Test Data: UTF-8 Test Strings

### 3.1 Test String Categories

Tests verify correct handling of these UTF-8 character ranges:

| Category           | Example String                       | Unicode Range      | Purpose                      |
|--------------------|--------------------------------------|--------------------|------------------------------|
| **ASCII**          | `"Hello, World!"`                    | U+0000 to U+007F   | Baseline - should always work |
| **Latin-1 Supplement** | `"Café, naïve, résumé"`            | U+0080 to U+00FF   | European languages           |
| **CJK Characters** | `"你好世界，こんにちは"`               | U+4E00 to U+9FFF   | Chinese, Japanese            |
| **Emoji**          | `"🌍🚀✨✅❌"`                         | U+1F300 to U+1F9FF | Modern emoji support         |
| **Combining Diacritics** | `"é" (e + combining acute)`     | U+0300 to U+036F   | Normalization testing        |
| **Mixed Script**   | `"Hello, 世界！🌍 café"`              | Multiple           | Real-world usage             |

### 3.2 Test Validation Rules

| Requirement                | Validation Method                        | Expected Result                                      |
|----------------------------|------------------------------------------|------------------------------------------------------|
| **FR-001**: Compiler flags | Build succeeds without encoding warnings | Exit code 0                                          |
| **FR-005**: Byte preservation | Hex dump of string literals in binary | Exact UTF-8 byte sequences                           |
| **FR-007**: BOM handling   | Build file with BOM, check preprocessed output | BOM stripped                                        |
| **FR-003**: Console init   | Call `init_console()` on Windows         | `SetConsoleOutputCP` returns non-zero                |
| **FR-009**: Diagnostic logging | Force console init failure (mock)    | Error message contains "UTF-8 console initialization failed" |

---

## 4. CMake Module Interfaces

### 4.1 `Utf8Compiler.cmake`

**Functions**:

```cmake
# Sets compiler flags for UTF-8 support
# Usage: jsav_enable_utf8_compiler_flags(<target>)
function(jsav_enable_utf8_compiler_flags target)
    # MSVC: target_compile_options(${target} PRIVATE /utf-8)
    # GCC/Clang: target_compile_options(${target} PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
endfunction()
```

**Variables Set**:

- `JSAV_UTF8_COMPILER_FLAGS` (cache, string)

### 4.2 `Utf8Console.cmake`

**Functions**:

```cmake
# Configures console output for UTF-8
# Usage: jsav_enable_utf8_console()
function(jsav_enable_utf8_console)
    # Configures utf8_console.hpp from template
    # Sets include directories
endfunction()
```

**Variables Set**:

- `JSAV_UTF8_CONSOLE_HEADER` (cache, string)

### 4.3 `Utf8BomStrip.cmake`

**Functions**:

```cmake
# Checks for and strips UTF-8 BOM
# Usage: jsav_check_utf8_bom(<file_list>)
function(jsav_check_utf8_bom files)
    # Reads first 3 bytes of each file
    # If EF BB BF detected, logs warning and strips
endfunction()
```

**Variables Set**:

- `JSAV_UTF8_BOM_DETECTED` (local, bool)

---

## 5. State Transitions

### 5.1 Build Configuration Flow

```text
[CMake Configure]
        |
        v
[Check jsav_ENABLE_UTF8] --OFF--> [Skip UTF-8 setup]
        |
       ON
        v
[Set compiler flags] --> [Configure header] --> [Check BOM] --> [Done]
```

### 5.2 Runtime Initialization Flow

```text
[Application Start]
        |
        v
[Call jsav::utf8::init_console()]
        |
        +-- Windows --> [Call SetConsoleOutputCP(CP_UTF8)]
        |                    |
        |                   Success --> [Return normally]
        |                    |
        |                   Failure --> [Print error, exit(EXIT_FAILURE)]
        |
        +-- Linux/macOS --> [No-op, return immediately]
```

---

## 6. Validation Rules

### 6.1 Compile-Time Validation

| Rule            | Check                          | Error Message                                                                                |
|-----------------|--------------------------------|----------------------------------------------------------------------------------------------|
| CMake version   | `cmake_minimum_required(VERSION 3.29)` | "CMake 3.29 or higher required for UTF-8 support"                                     |
| Compiler support | Detect compiler ID and version | "Compiler does not support UTF-8 flags. Minimum: GCC 13+, Clang 18+, MSVC 2022+"             |
| Header generation | Verify template exists        | "utf8_console.hpp.in template not found"                                                     |

### 6.2 Runtime Validation

| Rule               | Check                           | Error Message                                                                     |
|--------------------|---------------------------------|-----------------------------------------------------------------------------------|
| Console init (Windows) | `SetConsoleOutputCP` return value | "UTF-8 console initialization failed. Terminal does not support UTF-8 output."   |
| Locale (Linux/macOS) | Assumed UTF-8 (no check)      | N/A - developer responsibility                                                    |

---

## 7. Relationships

### 7.1 Dependencies

```text
jsav_ENABLE_UTF8 (option)
    ├── jsav_enable_utf8_compiler_flags() --> Uses CMAKE_CXX_COMPILER_ID
    ├── jsav_enable_utf8_console() --> Uses configured_files/
    └── jsav_check_utf8_bom() --> Uses file() command
```

### 7.2 Integration Points

```text
ProjectOptions.cmake
    └── jsav_local_options()
        └── if(jsav_ENABLE_UTF8)
            ├── include(cmake/Utf8Compiler.cmake)
            ├── include(cmake/Utf8Console.cmake)
            └── include(cmake/Utf8BomStrip.cmake)
```

---

## 8. File Locations

| File            | Path                                          | Type         | Purpose                        |
|-----------------|-----------------------------------------------|--------------|--------------------------------|
| Compiler Module | `cmake/Utf8Compiler.cmake`                    | CMake        | Set compiler flags             |
| Console Module  | `cmake/Utf8Console.cmake`                     | CMake        | Configure runtime header       |
| BOM Module      | `cmake/Utf8BomStrip.cmake`                    | CMake        | Detect/strip BOM               |
| Header Template | `configured_files/include/jsav/utf8_console.hpp.in` | CMake template | Generated header source     |
| Generated Header | `build/configured_files/include/jsav/utf8_console.hpp` | C++ header | Runtime API            |
| Test File       | `test/tests.cpp` (extended)                   | C++          | UTF-8 test cases               |

---

## 9. Constraints

| Constraint                          | Type | Enforcement                                              |
|-------------------------------------|------|----------------------------------------------------------|
| No new third-party dependencies     | Hard | CMake modules use only built-in commands                 |
| Zero runtime overhead (non-Windows) | Hard | Linux/macOS `init_console()` is empty inline function    |
| Fail-fast on console init failure   | Hard | `std::exit(EXIT_FAILURE)` on Windows failure             |
| UTF-8 identifiers out of scope      | Soft | Documented limitation, not enforced                      |
| Legacy encodings unsupported        | Hard | Compiler errors only (no CMake detection)                |

---

## 10. Performance Characteristics

| Operation                  | Cost                                      | Frequency                         |
|----------------------------|-------------------------------------------|-----------------------------------|
| Compiler flag setting      | O(1)                                      | Once per target at configure time |
| Header generation          | O(1)                                      | Once per configure                |
| BOM check                  | O(n) where n = number of source files     | Once per configure                |
| `init_console()` call (Windows) | O(1)                                 | Once per application start        |
| `init_console()` call (Linux/macOS) | O(1)                             | Once per application start (no-op) |

**Memory Overhead**: Zero - generated header contains only inline functions

**Build Time Overhead**: <1% (BOM check adds ~100ms to configure for 1000-file project)

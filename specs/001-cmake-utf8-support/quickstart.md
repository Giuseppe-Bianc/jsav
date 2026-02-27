# Quickstart: UTF-8 Support in jsav

**Generated**: 2026-02-27  
**Feature**: 001-cmake-utf8-support  
**Status**: Ready for Implementation

---

## For jsav Project Maintainers

This guide shows how to implement UTF-8 support in the jsav CMake build system.

### Step 1: Create CMake Modules

Create three CMake modules in the `cmake/` directory:

#### 1.1 `cmake/Utf8Compiler.cmake`

```cmake
# Utf8Compiler.cmake - Set compiler flags for UTF-8 support
include_guard(GLOBAL)

function(jsav_enable_utf8_compiler_flags TARGET)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(${TARGET} PRIVATE /utf-8)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${TARGET} PRIVATE 
            -finput-charset=UTF-8
            -fexec-charset=UTF-8)
    endif()
    
    set(JSAV_UTF8_COMPILER_FLAGS "Enabled for ${TARGET}" CACHE INTERNAL "UTF-8 compiler flags status")
endfunction()
```

#### 1.2 `cmake/Utf8Console.cmake`

```cmake
# Utf8Console.cmake - Configure UTF-8 console runtime header
include_guard(GLOBAL)

function(jsav_enable_utf8_console)
    # Configure the header from template
    configure_file(
        ${CMAKE_SOURCE_DIR}/configured_files/include/jsav/utf8_console.hpp.in
        ${CMAKE_BINARY_DIR}/configured_files/include/jsav/utf8_console.hpp
        @ONLY
    )
    
    # Add to include directories
    target_include_directories(jsav_options INTERFACE 
        ${CMAKE_BINARY_DIR}/configured_files/include
    )
    
    set(JSAV_UTF8_CONSOLE_HEADER 
        "${CMAKE_BINARY_DIR}/configured_files/include/jsav/utf8_console.hpp" 
        CACHE INTERNAL "Path to generated UTF-8 console header")
endfunction()
```

#### 1.3 `cmake/Utf8BomStrip.cmake`

```cmake
# Utf8BomStrip.cmake - Check for and report UTF-8 BOM
include_guard(GLOBAL)

function(jsav_check_utf8_bom FILES)
    set(BOM_FOUND OFF)
    
    foreach(FILE_PATH ${FILES})
        if(EXISTS ${FILE_PATH})
            file(READ ${FILE_PATH} FILE_CONTENT LIMIT 3)
            string(HEX ${FILE_CONTENT} HEX_CONTENT)
            
            if(HEX_CONTENT STREQUAL "efbbbf")
                message(WARNING "UTF-8 BOM detected in ${FILE_PATH}. Consider removing BOM for consistency.")
                set(BOM_FOUND ON)
            endif()
        endif()
    endforeach()
    
    set(JSAV_UTF8_BOM_DETECTED ${BOM_FOUND} PARENT_SCOPE)
endfunction()
```

---

### Step 2: Create Header Template

Create `configured_files/include/jsav/utf8_console.hpp.in`:

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

---

### Step 3: Integrate with ProjectOptions.cmake

Modify `ProjectOptions.cmake` to include UTF-8 support:

#### 3.1 Add Option (in `jsav_setup_options`)

```cmake
# In jsav_setup_options() macro
option(jsav_ENABLE_UTF8 "Enable UTF-8 compiler flags and console support" ON)

if(NOT PROJECT_IS_TOP_LEVEL)
    mark_as_advanced(jsav_ENABLE_UTF8)
endif()
```

#### 3.2 Wire into `jsav_local_options()`

```cmake
# In jsav_local_options() macro, after sanitizer setup
if(jsav_ENABLE_UTF8)
    include(cmake/Utf8Compiler.cmake)
    include(cmake/Utf8Console.cmake)
    include(cmake/Utf8BomStrip.cmake)
    
    # Apply to main targets
    jsav_enable_utf8_compiler_flags(jsav)
    jsav_enable_utf8_compiler_flags(jsav_lib)
    jsav_enable_utf8_compiler_flags(jsav_core_lib)
    
    # Configure runtime header
    jsav_enable_utf8_console()
    
    # Check for BOM in source files (at configure time)
    file(GLOB_RECURSE SOURCE_FILES 
        ${CMAKE_SOURCE_DIR}/src/*.cpp
        ${CMAKE_SOURCE_DIR}/src/*.hpp
        ${CMAKE_SOURCE_DIR}/include/*.hpp
    )
    jsav_check_utf8_bom("${SOURCE_FILES}")
endif()
```

---

### Step 4: Update Main Application

Modify `src/jsav/main.cpp` to call initialization:

```cpp
#include <jsav/utf8_console.hpp>  // Add this include

int main(int argc, char* argv[]) {
    jsav::utf8::init_console();  // Call unconditionally at start
    
    // ... rest of existing main() implementation
}
```

---

### Step 5: Add Tests to `test/tests.cpp`

Append UTF-8 test cases to existing test file:

```cpp
// In test/tests.cpp, add new test section

#include <jsav/utf8_console.hpp>

TEST_CASE("UTF-8 Support", "[utf8]") {
    SECTION("String literal preservation") {
        // FR-005: UTF-8 byte sequences preserved
        const char* utf8_string = "Hello, 世界！🌍";
        REQUIRE(strlen(utf8_string) > 10);  // Multi-byte characters
        REQUIRE(std::string_view(utf8_string).find("世界") != std::string_view::npos);
    }
    
    SECTION("Console initialization (Windows only)") {
#ifdef _WIN32
        // FR-003: Console initialization
        // Note: Can't actually test SetConsoleOutputCP in unit test
        // This verifies the header compiles and function exists
        REQUIRE_NOTHROW(jsav::utf8::init_console());
#else
        // Linux/macOS: Should be no-op
        REQUIRE_NOTHROW(jsav::utf8::init_console());
#endif
    }
    
    SECTION("Locale independence") {
        // FR-004, FR-006: UTF-8 works regardless of system locale
        std::string utf8_path = "test/路径/ファイル";
        REQUIRE(!utf8_path.empty());
        // Actual path handling tested in integration tests
    }
}
```

---

### Step 6: Verify Build

```bash
# Clean rebuild
rm -rf build/
cmake -S . -B build -G Ninja
cmake --build build

# Run UTF-8 tests
ctest -R "\[utf8\]" --output-on-failure

# Verify UTF-8 output
./build/jsav --help  # Should display UTF-8 characters correctly
```

---

## For Application Developers

If you're using the jsav build system in your application:

### Step 1: Enable UTF-8 in CMake

```cmake
# In your CMakeLists.txt or preset
-Djsav_ENABLE_UTF8=ON
```

### Step 2: Include Header in main.cpp

```cpp
#include <jsav/utf8_console.hpp>

int main() {
    jsav::utf8::init_console();  // Must call unconditionally
    
    // Your application code
    fmt::print("Hello, 世界！🌍\n");
    
    return 0;
}
```

### Step 3: Ensure Source Files are UTF-8

- Save all `.cpp` and `.hpp` files as UTF-8 (with or without BOM)
- Configure your editor to use UTF-8 as default encoding
- Avoid mixing legacy encodings (Windows-1252, ISO-8859-1)

### Step 4: Use Compatible Terminal

**Windows**:

- Use Windows Terminal (recommended)
- Or cmd.exe with UTF-8 font (Consolas, Lucida Console)

**Linux/macOS**:

- Any modern terminal (default UTF-8 locale)
- No additional configuration required

---

## Troubleshooting

### Build Errors

#### "Compiler does not support UTF-8 flags"

**Cause**: Using older compiler version

**Solution**: Upgrade to minimum supported version:

- GCC 13+
- Clang 18+
- MSVC 2022+

#### "UTF-8 header template not found"

**Cause**: Template file missing from `configured_files/`

**Solution**: Verify `configured_files/include/jsav/utf8_console.hpp.in` exists

### Runtime Errors

#### "UTF-8 console initialization failed"

**Cause**: Terminal doesn't support UTF-8

**Solution**:

1. Switch to Windows Terminal
2. Update terminal font to UTF-8-compatible font
3. Check terminal documentation for UTF-8 support

#### UTF-8 characters display as boxes or question marks

**Cause**: Font doesn't have glyphs for required Unicode ranges

**Solution**:

1. Install fonts with broad Unicode coverage (Segoe UI Emoji, Noto fonts)
2. Change terminal font settings
3. Verify font supports required character ranges

### Test Failures

#### UTF-8 tests fail in CI

**Cause**: CI environment uses non-UTF-8 terminal

**Solution**:

```bash
# Configure CI to use UTF-8 locale
export LANG=C.UTF-8
export LC_ALL=C.UTF-8

# Or skip UTF-8 tests in non-interactive environments
ctest -E "\[utf8\]"  # Exclude UTF-8 tests
```

---

## Verification Checklist

After implementation, verify:

- [ ] CMake configures without errors
- [ ] Build completes without encoding warnings
- [ ] `jsav/utf8_console.hpp` is generated in build directory
- [ ] Application compiles with `#include <jsav/utf8_console.hpp>`
- [ ] UTF-8 tests pass: `ctest -R "\[utf8\]"`
- [ ] `./build/jsav --help` displays UTF-8 correctly
- [ ] Application outputs UTF-8 strings correctly
- [ ] No BOM-related warnings (or BOM removed from source files)

---

## Next Steps

After successful implementation:

1. **Commit changes** to feature branch `001-cmake-utf8-support`
2. **Run full test suite**: `ctest --output-on-failure`
3. **Run static analysis**: `cmake --build build --target clang-tidy`
4. **Run coverage**: `gcovr -r . --config=gcovr.cfg`
5. **Create pull request** to `main` branch

---

## References

- [Research Report](./research.md) - Technical decisions and rationale
- [Data Model](./data-model.md) - Configuration objects and API structure
- [CMake API Contract](./contracts/cmake-api.md) - Build system interface
- [Runtime Header Contract](./contracts/runtime-header-api.md) - C++ API interface
- [Feature Specification](./spec.md) - Requirements and acceptance criteria

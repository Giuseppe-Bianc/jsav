# Research: CMake UTF-8 Support

**Generated**: 2026-02-27  
**Feature**: 001-cmake-utf8-support  
**Status**: Complete - All decisions documented

---

## Phase 0: Research Findings

This section documents all technical decisions for UTF-8 support implementation. Since the user specification provided complete technical details, there were no "NEEDS CLARIFICATION" items requiring external research.

---

## Decision 1: Compiler UTF-8 Flags

**Decision**: Use `/utf-8` for MSVC, `-finput-charset=UTF-8 -fexec-charset=UTF-8` for GCC and Clang

**Rationale**:

- MSVC's `/utf-8` flag sets both source and execution character sets to UTF-8 in a single option (Visual Studio 2022+ fully supports this)
- GCC and Clang require separate flags for input charset (`-finput-charset=UTF-8`) and execution charset (`-fexec-charset=UTF-8`)
- These flags ensure source files containing UTF-8 characters (comments, string literals) are interpreted correctly regardless of system locale
- Both approaches are compiler-standard, well-documented, and have zero runtime overhead

**Alternatives Considered**:

- Using `#pragma execution_charset("UTF-8")` in source files: Rejected because it requires modifying every source file and is compiler-specific
- Setting system locale via CMake `set(ENV{LC_ALL} "C.UTF-8")`: Rejected because it affects the entire build environment and doesn't guarantee compiler behavior
- Using BOM in all source files: Rejected because BOM is controversial in C++ community, can cause issues with some tools, and FR-007 requires accepting both forms

---

## Decision 2: UTF-8 Console Output on Windows

**Decision**: Use `SetConsoleOutputCP(CP_UTF8)` called unconditionally in application's `main()` function

**Rationale**:

- Single Win32 API call affects all console output for the process lifetime
- `CP_UTF8` (65001) is the standard UTF-8 code page identifier on Windows
- Unconditional call ensures consistent behavior regardless of system locale
- Fail-fast strategy (exit on failure) provides clear diagnostic feedback
- No runtime overhead on Linux/macOS where UTF-8 is the default

**Alternatives Considered**:

- Detecting terminal compatibility before calling: Rejected per FR-003 clarification - fail-fast is preferred over complex detection logic
- Using `_setmode(_O_U8TEXT, _O_U16TEXT)`: Rejected because it requires wide character I/O throughout the application
- Relying on Windows Terminal automatic UTF-8 detection: Rejected because it doesn't work with legacy cmd.exe and requires user configuration
- Using `chcp 65001` in batch files: Rejected because it affects the entire shell session and doesn't work for embedded applications

**Implementation Pattern**:

```cpp
// In generated utf8_console.hpp
namespace jsav::utf8 {
    inline void init_console() {
#ifdef _WIN32
        if (!SetConsoleOutputCP(CP_UTF8)) {
            fmt::print(stderr, "ERROR: UTF-8 console initialization failed. Terminal does not support UTF-8 output.\n");
            std::exit(EXIT_FAILURE);
        }
#endif
        // Linux/macOS: No-op, UTF-8 is default
    }
}
```

---

## Decision 3: UTF-8 BOM Handling

**Decision**: Accept UTF-8 files with or without BOM; strip BOM during preprocessing if present

**Rationale**:

- UTF-8 BOM (`EF BB BF`) is optional per RFC 3629
- Some editors (Notepad, older Visual Studio) add BOM by default
- GCC/Clang handle BOM correctly with charset flags, but stripping ensures consistent output
- CMake's `file(READ ...)` and `string(SUBSTRING ...)` can detect and strip BOM at configure time
- No runtime overhead - BOM handling occurs only during build configuration

**Alternatives Considered**:

- Rejecting files with BOM: Rejected because it would break builds for developers using editors that add BOM
- Requiring BOM on all files: Rejected because most Unix editors don't add BOM by default
- Runtime BOM detection: Rejected because BOM should be handled at build time, not runtime

**Implementation Pattern**:

```cmake
# In Utf8BomStrip.cmake
function(jsav_check_utf8_bom)
    # Check first 3 bytes of source files for BOM (EF BB BF)
    # Log warning if BOM detected, recommend conversion
    # Strip BOM during configure if needed
endfunction()
```

---

## Decision 4: CMake Module Structure

**Decision**: Three focused CMake modules (`Utf8Compiler.cmake`, `Utf8Console.cmake`, `Utf8BomStrip.cmake`) plus one configured header

**Rationale**:

- Separation of concerns: Each module handles one aspect of UTF-8 support
- Consistent with existing project structure (e.g., `Sanitizers.cmake`, `Hardening.cmake`)
- Configured header pattern matches existing `configured_files/` usage in project
- Enables selective inclusion - projects can use compiler flags without console initialization
- Maintains existing `ProjectOptions.cmake` integration pattern via `jsav_ENABLE_UTF8` option

**Alternatives Considered**:

- Single monolithic `Utf8Support.cmake`: Rejected because it would be harder to maintain and test
- Inline CMake code in `ProjectOptions.cmake`: Rejected because it would bloat an already large file
- External CMake package: Rejected because UTF-8 support is core functionality, not optional

---

## Decision 5: Integration with ProjectOptions.cmake

**Decision**: Wire UTF-8 support into existing `jsav_local_options()` and `jsav_global_options()` macros via `jsav_ENABLE_UTF8` option (default ON)

**Rationale**:

- Consistent with how other features (sanitizers, clang-tidy, coverage) are integrated
- Default ON ensures UTF-8 support is enabled for all new developers automatically
- Option can be disabled for legacy builds or special configurations
- Functions `jsav_enable_utf8_compiler_flags()`, `jsav_enable_utf8_console()`, `jsav_check_utf8_bom()` follow existing naming conventions
- All 8 existing CMakePresets.json presets automatically pick up the new flags

**Alternatives Considered**:

- Separate CMake option per module: Rejected because it would create unnecessary complexity
- Always-on without option: Rejected because some legacy builds may need to disable it
- Environment variable control: Rejected because CMake options are more discoverable and documented

---

## Decision 6: Testing Strategy

**Decision**: Append UTF-8 tests to existing `test/tests.cpp` using Catch2 with `[utf8]` tag

**Rationale**:

- Existing test infrastructure already configured for Catch2 3.13.0
- `[utf8]` tag enables selective test execution: `ctest -R "\[utf8\]"`
- Tests cover all functional requirements (FR-001 through FR-009)
- No new test files or CMake configuration required
- Tests run as part of standard `ctest` invocation

**Test Coverage**:

1. String literal byte preservation (FR-005)
2. BOM absence verification (FR-007)
3. Console initializer behavior (FR-003)
4. Locale independence (FR-004, FR-006)
5. FR-009 diagnostic logging verification

**Alternatives Considered**:

- Separate test file `utf8_tests.cpp`: Rejected because it would require CMakeLists.txt changes
- Integration tests in CI only: Rejected because developers need local test feedback
- Runtime test with actual UTF-8 output: Rejected because byte-level verification is more reliable

---

## Decision 7: Runtime Header API Design

**Decision**: Expose `jsav::utf8::init_console()` as unconditional initialization function

**Rationale**:

- Clear, self-documenting API name
- Namespace `jsav::utf8` prevents name collisions
- Unconditional call requirement documented in quickstart.md
- Fail-fast behavior (exit on failure) prevents silent UTF-8 corruption
- No return value - either succeeds or terminates

**Alternatives Considered**:

- Return boolean for manual error handling: Rejected because it encourages ignoring errors
- Throw exception on failure: Rejected because exceptions may not be enabled in all builds
- Silent failure with fallback: Rejected because it hides configuration problems
- Optional initialization with detection: Rejected per FR-003 - detection adds complexity without benefit

---

## Decision 8: Cross-Platform Behavior

**Decision**: Windows requires explicit initialization; Linux/macOS are no-op

**Rationale**:

- Windows: Console defaults to system code page (e.g., CP1252 for English US)
- Linux/macOS: UTF-8 is default locale on modern distributions (assumed per spec clarifications)
- Non-UTF-8 locales on Linux/macOS explicitly unsupported (developer must configure at OS level)
- Consistent with "fail-fast" philosophy - detect issues early

**Alternatives Considered**:

- Automatic locale detection on Linux/macOS: Rejected because it adds runtime overhead
- Fallback to legacy encodings: Rejected per FR-008 - legacy encodings out of scope
- Runtime locale configuration: Rejected because locale should be configured at OS level

---

## Summary of Technology Choices

| Component              | Technology                        | Version/Method   | Rationale                       |
|------------------------|-----------------------------------|------------------|---------------------------------|
| Compiler Flags (MSVC)  | `/utf-8`                          | Visual Studio 2022+ | Standard, zero overhead        |
| Compiler Flags (GCC)   | `-finput-charset=UTF-8 -fexec-charset=UTF-8` | GCC 13+    | Explicit charset control        |
| Compiler Flags (Clang) | `-finput-charset=UTF-8 -fexec-charset=UTF-8` | Clang 18+  | Explicit charset control        |
| Console Output (Windows) | `SetConsoleOutputCP(CP_UTF8)`   | Win32 API        | Single call, process-wide       |
| BOM Handling           | CMake `file()`/`string()`         | Native commands  | Build-time only                 |
| Diagnostics            | `fmt::print`                      | fmt 12.1.0       | Already pinned dependency       |
| Testing                | Catch2 `REQUIRE`                  | Catch2 3.13.0    | Existing test framework         |
| Build Integration      | CMake option                      | `jsav_ENABLE_UTF8` | Consistent with existing patterns |

---

## References

- [MSVC /utf-8 Documentation](https://learn.microsoft.com/en-us/cpp/build/reference/utf-8-set-source-and-executable-character-sets-to-utf-8)
- [GCC Character Set Options](https://gcc.gnu.org/onlinedocs/gcc/Code-Gen-Options.html)
- [Win32 SetConsoleOutputCP](https://learn.microsoft.com/en-us/windows/console/setconsoleoutputcp)
- [UTF-8 BOM RFC 3629](https://www.rfc-editor.org/rfc/rfc3629)
- [CMake file() Command](https://cmake.org/cmake/help/latest/command/file.html)
